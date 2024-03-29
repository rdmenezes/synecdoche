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

#ifndef _ACCT_SETUP_H_
#define _ACCT_SETUP_H_

#include "gui_http.h"
#include "error_numbers.h"

/// represents the contents of project_info.xml
///
struct PROJECT_INIT {
    std::string url;
    char name[256];
    char account_key[256];

    PROJECT_INIT();
    int init();
    int remove();
    void clear();
};

struct ACCOUNT_IN {
    std::string url;
    /// this is the account identifier.  on systems that use
    /// usernames it is the username, on systems that use
    /// email addresses it is an email address for the user.
    std::string email_addr;
    /// this is the suggested friendly name for the user
    /// during account creation.
    std::string user_name;
    std::string passwd_hash;

    void parse(const char* buf);
};

struct GET_PROJECT_CONFIG_OP: public GUI_HTTP_OP {
    std::string reply;
    int error_num;

    GET_PROJECT_CONFIG_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
    virtual ~GET_PROJECT_CONFIG_OP(){}
    int do_rpc(const std::string& url);
    virtual void handle_reply(int http_op_retval);
};

struct LOOKUP_ACCOUNT_OP: public GUI_HTTP_OP {
    std::string reply;
    int error_num;

    LOOKUP_ACCOUNT_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
    virtual ~LOOKUP_ACCOUNT_OP(){}
    int do_rpc(const ACCOUNT_IN& ai);
    virtual void handle_reply(int http_op_retval);
};

struct CREATE_ACCOUNT_OP: public GUI_HTTP_OP {
    std::string reply;
    int error_num;

    CREATE_ACCOUNT_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
    virtual ~CREATE_ACCOUNT_OP(){}
    int do_rpc(const ACCOUNT_IN& ai);
    virtual void handle_reply(int http_op_retval);
};
#ifdef ENABLE_UPDATE_CHECK
struct GET_CURRENT_VERSION_OP: public GUI_HTTP_OP {
    int error_num;

    GET_CURRENT_VERSION_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
    virtual ~GET_CURRENT_VERSION_OP(){}
    int do_rpc();
    virtual void handle_reply(int http_op_retval);
};
#endif

struct PROJECT_ATTACH {
    int error_num;
    std::vector<std::string> messages;
};

#endif
