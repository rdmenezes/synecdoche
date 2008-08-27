// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Nicolas Alvarez
// Copyright (C) 2005 University of California
//
// Synecdoche is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Synecdoche is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _GUI_RPC_SERVER_H
#define _GUI_RPC_SERVER_H

#include <vector>
#include <set>

#include "network.h"
#include "gui_http.h"
#include "acct_setup.h"
#include "miofile.h"

// FSM states for auto-update

/// No get_screensaver_mode() yet
#define AU_SS_INIT          0
/// Got a get_screensaver_mode()
#define AU_SS_GOT           1
/// Send a QUIT next time
#define AU_SS_QUIT_REQ      2
/// QUIT sent
#define AU_SS_QUIT_SENT     3

#define AU_MGR_INIT         0
#define AU_MGR_GOT          1
#define AU_MGR_QUIT_REQ     2
#define AU_MGR_QUIT_SENT    3
    
class GUI_RPC_CONN {
public:
    int sock;
    bool auth_needed; ///< If true, don't allow operations other than authentication.
    bool is_local; ///< Connection is from local host.
    int au_ss_state;
    int au_mgr_state;
    GUI_HTTP gui_http;

    GUI_RPC_CONN(int);
    ~GUI_RPC_CONN();
    int handle_rpc();
private:
    char nonce[256];

    GET_PROJECT_CONFIG_OP get_project_config_op;
    LOOKUP_ACCOUNT_OP lookup_account_op;
    CREATE_ACCOUNT_OP create_account_op;

    void handle_auth1(MIOFILE&);
    void handle_auth2(const char*, MIOFILE&);
    void handle_get_project_config(const char* buf, MIOFILE& fout);
    void handle_get_project_config_poll(const char*, MIOFILE& fout);
    void handle_lookup_account(const char* buf, MIOFILE& fout);
    void handle_lookup_account_poll(const char*, MIOFILE& fout);
    void handle_create_account(const char* buf, MIOFILE& fout);
    void handle_create_account_poll(const char*, MIOFILE& fout);
};

// authentication for GUI RPCs:
// 1) if a IPaddr-list file is found, accept only from those addrs
// 2) if a password file file is found, ALSO demand password auth

class GUI_RPC_CONN_SET {
    std::vector<GUI_RPC_CONN*> gui_rpcs;
    std::set<unsigned long> allowed_remote_ip_addresses;
    int get_allowed_hosts();
    int get_password();
    int insert(GUI_RPC_CONN*);
    bool check_allowed_list(unsigned long ip_addr) const;
    bool remote_hosts_file_exists;
public:
    int lsock;
    /// Time of the last RPC that needs network access to handle
    double time_of_last_rpc_needing_network;

    GUI_RPC_CONN_SET();
    char password[256];
    void get_fdset(FDSET_GROUP&, FDSET_GROUP&) const;
    void got_select(const FDSET_GROUP&);
    int init(bool last_time);
    void close();
    bool recent_rpc_needs_network(double interval) const;
    void send_quits();
    bool quits_sent() const;
    bool poll();
};

#endif
