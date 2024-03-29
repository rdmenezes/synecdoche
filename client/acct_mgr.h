// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
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

#ifndef _ACCT_MGR_
#define _ACCT_MGR_

#include <string>
#include <vector>

#include "gui_http.h"
#include "client_types.h"

class XML_PARSER;

/// Represents info stored in acct_mgr_url.xml and acct_mgr_login.xml
///
struct ACCT_MGR_INFO {
    // the following used to be std::string but there
    // were mysterious bugs where setting it to "" didn't work
    //
    char acct_mgr_name[256];
    char acct_mgr_url[256];
    char login_name[256];
    char password_hash[256]; ///< MD5 of password.lowercase(login_name)
    char opaque[256];        ///< Whatever the AMS sends us
    char signing_key[MAX_KEY_LEN];
    char previous_host_cpid[64]; ///< The host CPID sent in last RPC
    double next_rpc_time;
    /// Whether to include GUI RPC port and password hash
    /// in AM RPCs (used for "farm management")
    bool send_gui_rpc_info;
    bool password_error;

    ACCT_MGR_INFO();
    int parse_login_file(FILE* f);
    int write_info();
    int init();
    void clear();
    bool poll();
};

struct OPTIONAL_BOOL {
    bool present;
    bool value;
    inline void init() { present=false; }
    inline void set(bool v) { value=v; present=true; }
};

struct OPTIONAL_DOUBLE {
    bool present;
    double value;
    inline void init() { present=false;}
    inline void set(double v) { value=v; present=true; }
};

// stuff after here related to RPCs to account managers

struct AM_ACCOUNT {
    std::string url;
    std::string authenticator;
    char url_signature[MAX_SIGNATURE_LEN];
    bool detach;
    bool update;
    OPTIONAL_BOOL dont_request_more_work;
    OPTIONAL_BOOL detach_when_done;
    OPTIONAL_DOUBLE resource_share;

    int parse(XML_PARSER& xp);
    AM_ACCOUNT() {}
    ~AM_ACCOUNT() {}
};

struct ACCT_MGR_OP: public GUI_HTTP_OP {
    bool via_gui;
    int error_num;
    /// A temporary copy while doing RPC.
    /// CLIENT_STATE::acct_mgr_info is authoritative.
    ACCT_MGR_INFO ami;
    std::string error_str;
    std::vector<AM_ACCOUNT> accounts;
    double repeat_sec;
    char* global_prefs_xml;
    char host_venue[256];

    int do_rpc(
        const std::string& url, const std::string& name,
        const std::string& password, bool via_gui
    );
    int parse(FILE* f);
    virtual void handle_reply(int http_op_retval);

    ACCT_MGR_OP();
    virtual ~ACCT_MGR_OP(){}
};

#endif
