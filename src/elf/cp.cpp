/*
 * Copyright (C) 2011-2013 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/cp.h>
#include <elf/db.h>
#include <elf/log.h>
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
        LOG_ERROR("cp",
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
    LOG_TRACE("cp",
            "Load config file %s DONE, %d items loaded.",
            path.c_str(),
            item_num);
    return true;
}

bool load_cfg(const std::string &path, pb_t *cfg)
{
    std::fstream fs(path.c_str(), std::ios::in | std::ios::binary);

    if (!fs) {
        LOG_ERROR("cp",
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
                LOG_ERROR("cp",
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
    LOG_TRACE("cp",
            "Load config file %s DONE.",
            path.c_str());
    return true;
}

static bool load_tbl(const std::string &tbl_name, pb_t *cfg)
{
    char sql[1024];

    sprintf(sql, "SELECT * FROM `%s`;",
            tbl_name.c_str());
    LOG_TRACE("db",
            "SQL: `%s'", sql);

    db_query(sql, cfg, "item");
    LOG_TRACE("cp",
            "Load config from DB `%s'DONE.",
            tbl_name.c_str());
    return true;
}

static bool load_tbl(const std::string &tbl_name, oid_t sid)
{
    char sql[1024];

    sprintf(sql, "SELECT * FROM `%s`;",
            tbl_name.c_str());
    LOG_TRACE("db",
            "SQL: `%s'", sql);

    db_req(sql, OID_NIL, NULL, NULL, NULL);
    LOG_TRACE("cp",
            "Load config from DB `%s'DONE.",
            tbl_name.c_str());
    return true;
}

pb_t *config_load(const std::string &name, const std::string &path)
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
    } else if (ext == path) {
        res = load_tbl(path, cfg);
    }

    if (!res) {
        LOG_ERROR("cp",
                "Invalid config file format: %s.",
                path.c_str());
        pb_destroy(cfg);
        return NULL;
    }
    return cfg;
}

bool config_load(const std::string &path, oid_t sid)
{
    std::string ext = path;

    get_ext(ext);
    if (ext == path) {
        return load_tbl(path, sid);
    }
    LOG_ERROR("cp",
            "Invalid config file format: %s.",
            path.c_str());
    return false;
}
} // namespace elf

