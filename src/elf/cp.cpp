/*
 * Copyright (C) 2011-2013 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/cp.h>
#include <elf/db/db.h>
#include <elf/log.h>
#include <libxml/parser.h>
#include <fstream>
#include <string>

using namespace google::protobuf;

namespace elf {
static const char CONFIG_COMMENT_CHAR = '#';
static const char CSV_DELIM_CHAR = ',';

static int is_period(char ch)
{
    return (ch == '.');
}

static int is_space(char ch)
{
    return std::isspace(static_cast<unsigned char>(ch));
}

static void trim_leading_ws(std::string &str)
{
    std::string::iterator it = str.begin();
    for (; it != str.end(); ++it) {
        if (!is_space(*it))
            break;
    }
    str.erase(str.begin(), it);
}

static void trim_trailing_ws(std::string &str)
{
    std::string::reverse_iterator rit = str.rbegin();
    for (; rit != str.rend(); ++rit) {
        if (!is_space(*rit))
            break;
    }
    str.erase(rit.base(), str.end());
}

static void trim_ws(std::string &str)
{
    trim_trailing_ws(str);
    trim_leading_ws(str);
}

static void get_ext(std::string &str)
{
    trim_ws(str);
    std::string::reverse_iterator rit = str.rbegin();
    for (; rit != str.rend(); ++rit) {
        if (is_period(*rit))
            break;
    }
    str.erase(str.begin(), rit.base());
}

static bool load_csv(const std::string &path, pb_t *cfg)
{
    std::fstream fs(path.c_str(), std::ios::in | std::ios::binary);

    if (!fs) {
        LOG_ERROR("cfg",
                "Can NOT open file %s.", path.c_str());
        return false;
    }

    const Reflection *ref = cfg->GetReflection();
    const Descriptor *des = cfg->GetDescriptor();
    const FieldDescriptor *lfd = des->field(0);
    int num = des->field_count();
    int item_num = 0;
    std::istringstream iss;
    std::string buff;

    // ignore intro line
    std::getline(fs, buff);
    while (std::getline(fs, buff)) {
        iss.str(buff);

        std::string val;
        pb_t *item = ref->AddMessage(cfg, lfd);

        assert(item);
        des = item->GetDescriptor();
        num = des->field_count();
        for (int i = 0; i < num; ++i) {
            std::getline(iss, val, CSV_DELIM_CHAR);
            assert(!iss.eof());

            const FieldDescriptor *ifd = des->field(i);

            pb_set_field(item, ifd, val.c_str());
        }
        ++item_num;
    }
    LOG_TRACE("cfg",
            "Load config file %s DONE, %d items loaded.",
            path.c_str(),
            item_num);
    return true;
}

bool load_cfg(const std::string &path, pb_t *cfg)
{
    std::fstream fs(path.c_str(), std::ios::in | std::ios::binary);

    if (!fs) {
        LOG_ERROR("cfg",
                "Can NOT open file %s.", path.c_str());
        return false;
    }

    const Reflection *ref_cfg = cfg->GetReflection();
    const Descriptor *des_cfg = cfg->GetDescriptor();
    pb_t *sec = NULL;
    const Descriptor *des_sec = NULL;
    const FieldDescriptor *fd = NULL;
    std::string buff;

    while (std::getline(fs, buff)) {
        trim_ws(buff);

        std::string::size_type const len = buff.size();

        if (len == 0 || buff[0] == CONFIG_COMMENT_CHAR)
            continue;

        // Check if we have a trailing \r because we are
        // reading a properties file produced on Windows.
        if (buff[len - 1] == '\r') {
            // Remove trailing 'Windows' \r.
            buff.resize(len - 1);
        }

        // section
        if (buff[0] == '[' && buff[len - 1] == ']') {
            buff.erase(buff.begin());
            buff.resize(len - 2);
            fd = des_cfg->FindFieldByName(buff);

            FieldDescriptor::CppType type = fd->cpp_type();

            if (type != FieldDescriptor::CPPTYPE_MESSAGE) {
                LOG_ERROR("cfg",
                        "Invalid config file: %s.",
                        path.c_str());
                assert(0);
                return false;
            }
            sec = ref_cfg->MutableMessage(cfg, fd);
            des_sec = sec->GetDescriptor();
        }

        // key-value pairs
        std::string::size_type const idx = buff.find('=');

        if (idx != std::string::npos) {
            std::string key = buff.substr(0, idx);
            std::string val = buff.substr(idx + 1);

            trim_ws(key);
            trim_ws(val);
            fd = des_sec->FindFieldByName(key);
            pb_set_field(sec, fd, val.c_str());
        }
    }
    LOG_TRACE("cfg",
            "Load config file %s DONE.",
            path.c_str());
    return true;
}

static void xmlattr_parse(pb_t *pb, xmlAttrPtr attr)
{
    assert(pb && attr);

    const Descriptor *des = pb->GetDescriptor();

    do {
        const FieldDescriptor *fd = des->FindFieldByName((const char *)(attr->name));

        pb_set_field(pb, fd, (const char *)(attr->children->content));
        attr = attr->next;
    } while (attr);
}

static void xmlnode_parse(pb_t *pb, xmlNodePtr node)
{
    assert(pb && node);

    const Reflection *ref = pb->GetReflection();
    const Descriptor *des = pb->GetDescriptor();
    xmlNodePtr cur = xmlFirstElementChild(node);

    while (cur) {
        const FieldDescriptor *fd = des->FindFieldByName((const char *)(cur->name));

        if (fd->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
            pb_t *item = NULL;

            switch (fd->label()) {
                case FieldDescriptor::LABEL_OPTIONAL:
                    item = ref->MutableMessage(pb, fd);
                    break;
                case FieldDescriptor::LABEL_REQUIRED:
                    item = ref->MutableMessage(pb, fd);
                    break;
                case FieldDescriptor::LABEL_REPEATED:
                    item = ref->AddMessage(pb, fd);
                    break;
            }

            if (cur->properties) {
                xmlattr_parse(item, cur->properties);
            } else if (cur->content) {
                pb_set_field(pb, fd, (const char *)(cur->content));
            }
            xmlnode_parse(item, cur);
        } else {
            if (cur->properties) {
                xmlattr_parse(pb, cur->properties);
            } else if (cur->content) {
                pb_set_field(pb, fd, (const char *)(cur->content));
            }
        }
        cur = xmlNextElementSibling(cur);
    }
}

static bool load_xml(const std::string &path, pb_t *cfg)
{
    xmlDocPtr doc = xmlReadFile(path.c_str(), "utf8", XML_PARSE_RECOVER);
    if (!doc) {
        LOG_ERROR("cfg",
                "Can NOT open file %s.", path.c_str());
        return false;
    }

    xmlNodePtr node = xmlDocGetRootElement(doc);
    if (!node) {
        LOG_ERROR("cfg",
                "Can NOT parse file %s.", path.c_str());
        xmlFreeDoc(doc);
        return false;
    }
    xmlnode_parse(cfg, node);
    LOG_TRACE("cfg",
            "Load config file %s DONE.",
            path.c_str());
    xmlFreeDoc(doc);
    return true;
}

static bool load_tbl(int idx, const std::string &tbl_name, pb_t *cfg,
        int type, db_callback proc)
{
    char sql[1024];

    sprintf(sql, "SELECT * FROM `%s`;",
            tbl_name.c_str());
    LOG_TRACE("db",
            "SQL: `%s'", sql);
    db_req(idx, sql, false, proc, type, cfg, "item");
    return true;
}

pb_t *config_load(const std::string &name, const std::string &path,
       int type, db_callback proc, int idx)
{
    pb_t *cfg = pb_create(name);
    std::string ext = path;

    get_ext(ext);
    cfg->Clear();

    bool res = false;
    if (ext == "csv") {
        res = load_csv(path, cfg);
    } else if (ext == "conf") {
        res = load_cfg(path, cfg);
    } else if (ext == "xml") {
        res = load_xml(path, cfg);
    } else if (ext == path) {
        res = load_tbl(idx, path, cfg, type, proc);
        cfg = NULL;
    }

    if (!res) {
        LOG_ERROR("cfg",
                "Invalid config file format: %s.",
                path.c_str());
        pb_destroy(cfg);
        return NULL;
    }
    return cfg;
}
} // namespace elf

