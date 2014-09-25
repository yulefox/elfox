/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/net/message.h>
#include <map>
#include <string>

namespace elf {
struct message_handler_t {
    pb_new init;
    msg_proc proc;
};

typedef std::map<std::string, message_handler_t *> reg_map;

static reg_map s_regs;

void message_unregist_all(void)
{
    reg_map::iterator itr = s_regs.begin();

    for (; itr != s_regs.end(); ++itr) {
        E_DELETE itr->second;
    }
}

void message_regist(const std::string &name, pb_new init, msg_proc proc)
{
    message_handler_t *hdl = E_NEW message_handler_t;

    hdl->init = init;
    hdl->proc = proc;
    s_regs[name] = hdl;
}

void message_handle(recv_message_t *msg)
{
    assert(msg);

    reg_map::const_iterator itr = s_regs.find(msg->name);

    if (itr != s_regs.end()) {
        message_handler_t *hdl = itr->second;

        msg->pb = hdl->init();
        net_decode(msg);
        hdl->proc(*msg);
    } else if (msg->ctx != NULL) {
        net_error(msg->ctx);
        net_close(msg->peer);
        LOG_WARN("net", "%lld INVALID message: %s.",
                msg->peer,
                msg->name.c_str());
    }
}
} // namespace elf

