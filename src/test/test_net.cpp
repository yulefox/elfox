/*
 * Copyright (C) 2013 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/net/net.h>
#include <elf/net/message.h>
/*
#include <test/pb/message.h>
#include <test/pb/Msg/Login.pb.h>
*/
#include <tut/tut.hpp>

    /*
namespace Game {
int OnInit(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnGetCurrentRole(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnUpgradeAbility(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnSyncPos(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnBattleResult(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnSetQuest(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnSetEvent(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnAddExp(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnGetMails(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnGetScene(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnKillRole(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnPassLevel(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnFailLevel(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnGameMaster(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnChangeSection(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnOperMail(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnGetCandidates(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnSetCurrentRole(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnGetConfig(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnOperItems(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnGetItems(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnMoveItems(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnUpgradeItem(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnGetBuddies(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnGetRoles(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnAutoLevel(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnGenerateNPCs(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnOperBuddy(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnForward(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnGetQuests(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnEnterGame(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnSetExp(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnChangeScene(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnLogout(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnCreateRole(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnDeleteRole(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

static int DoLogin(elf::oid_t peer)
{
    Login::Req req;

    req.set_user("fox");
    elf::message_send(peer, req);
    LOG_TEST("SEND"); return 0;
}

int OnLogin(const elf::recv_message_t &msg)
{
    Login::Req *req = static_cast<Login::Req *>(msg.pb);
    Login::Res res;

    assert(req);
    res.set_code(0x1000);
    elf::message_send(msg.peer, res);

    LOG_TEST("OnLogin"); return 0;
}

int OnChat(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnSetCurrentScene(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnSyncRole(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }

int OnSyncRoles(const elf::recv_message_t &msg) {
    LOG_TEST("TEST"); return 0; }
}
*/

namespace tut {
struct net {
    net() {
    }

    ~net() {
    }
};

typedef test_group<net> factory;
typedef factory::object object;

static tut::factory tf("net");

template<>
template<>
void object::test<1>() {
    static int frame = 0;

    set_test_name("Initialize network environment");
    elf::net_listen("test_server", "localhost", 6670);
    elf::net_connect(0, elf::OID_NIL, "test_client", "localhost", 6670);

    while (true) {
        elf::net_proc();
        usleep(50000);
        if (++frame % 20 == 0) {
            putchar('.');
            fflush(stdout);
        }
    }
    ensure(true);
}
}

