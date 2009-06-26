// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Nicolas Alvarez, Peter Kortschack
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
#include <string>

#include "network.h"
#include "gui_http.h"
#include "acct_setup.h"

class GUI_RPC_CONN {
public:
    int sock;
    bool auth_needed; ///< If true, don't allow operations other than authentication.
    bool is_local; ///< Connection is from local host.
    GUI_HTTP gui_http;

    GUI_RPC_CONN(int);
    ~GUI_RPC_CONN();
    int handle_rpc();
    int handle_write();
    bool needs_write() const {
        return !write_buffer.empty();
    }
private:
    std::string nonce;
    std::string write_buffer;

    GET_PROJECT_CONFIG_OP get_project_config_op;
    LOOKUP_ACCOUNT_OP lookup_account_op;
    CREATE_ACCOUNT_OP create_account_op;

    /// Handle an authorization request by creating and sending a nonce.
    void handle_auth1(MIOFILE& fout);

    /// Check if the response to the challenge sent by handle_auth1 is correct.
    void handle_auth2(const char* buf, MIOFILE& fout);

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

    GUI_RPC_CONN_SET();
    char password[256];
    void get_fdset(FDSET_GROUP&) const;
    void got_select(FDSET_GROUP&);
    int init(bool last_time);
    void close();
    void send_quits();
    bool quits_sent() const;
    bool poll();
};

#endif
