// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Peter Kortschack
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

#ifndef ACCT_MGR_H
#define ACCT_MGR_H

#include <string>
#include <vector>

#include "miofile.h"
#include "gui_http.h"
#include "client_types.h"
#include "ticpp/ticpp.h"

/// represents info stored in acct_mgr_url.xml and acct_mgr_login.xml
struct ACCT_MGR_INFO {
    // the following used to be std::string but there
    // were mysterious bugs where setting it to "" didn't work
    std::string acct_mgr_name;
    std::string acct_mgr_url;
    std::string login_name;
    std::string password_hash; ///< md5 of password.lowercase(login_name)
    std::string opaque; ///< whatever the AMS sends us
    std::string signing_key;
    std::string previous_host_cpid; ///< the host CPID sent in last RPC
    double next_rpc_time;
    /// whether to include GUI RPC port and password hash
    /// in AM RPCs (used for "farm management")
    bool send_gui_rpc_info;
    bool password_error;

    ACCT_MGR_INFO();
    void parse_login_file(const ticpp::Element* acct_mgr_login);
    int write_info();
    int init();
    void clear();
    bool poll();
};

struct OPTIONAL_BOOL {
    bool present;
    bool value;
    void init() { present = false; }
    void set(bool v) { value = v; present = true; }
};

/// Read the value for an instance of OPTINAL_BOOL from a stream.
std::istream& operator >> (std::istream& in, OPTIONAL_BOOL& out);

struct OPTIONAL_DOUBLE {
    bool present;
    double value;
    void init() { present = false; }
    void set(double v) { value = v; present = true; }
};

/// Read the value for an instance of OPTIONAL_DOUBLE from a stream.
std::istream& operator >> (std::istream& in, OPTIONAL_DOUBLE& out);


// stuff after here related to RPCs to account managers

struct AM_ACCOUNT {
    std::string url;
    std::string authenticator;
    std::string url_signature;
    bool detach;
    bool update;
    OPTIONAL_BOOL dont_request_more_work;
    OPTIONAL_BOOL detach_when_done;
    OPTIONAL_DOUBLE resource_share;

    void parse(const ticpp::Element* acct_mgr_reply);
    AM_ACCOUNT() {}
    ~AM_ACCOUNT() {}
};

struct ACCT_MGR_OP: public GUI_HTTP_OP {
    bool via_gui;
    int error_num;
    /// a temporary copy while doing RPC.
    /// CLIENT_STATE::acct_mgr_info is authoratative
    ACCT_MGR_INFO ami;
    std::string error_str;
    std::vector<AM_ACCOUNT> accounts;
    double repeat_sec;
    std::string global_prefs_xml;
    std::string host_venue;

    int do_rpc(
        const std::string& url, const std::string& name,
        const std::string& password, bool via_gui
    );
    void parse(const ticpp::Element* acct_mgr_reply);
    virtual void handle_reply(int http_op_retval);

    ACCT_MGR_OP();
    virtual ~ACCT_MGR_OP(){}
};

#endif // ACCT_MGR_H
