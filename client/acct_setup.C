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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#endif

#include "acct_setup.h"

#include <cstring>
#include <string>
#include "client_state.h"
#include "file_names.h"
#include "parse.h"
#include "filesys.h"
#include "str_util.h"
#include "util.h"
#include "client_msgs.h"
#include "miofile.h"

void PROJECT_INIT::clear() {
    url.clear();
    strcpy(name, "");
    strcpy(account_key, "");
}

PROJECT_INIT::PROJECT_INIT() {
    clear();
}

int PROJECT_INIT::init() {
    char    buf[256];
    FILE*   p;

    clear();
    p = fopen(PROJECT_INIT_FILENAME, "r");
    if (p) {
        MIOFILE mf;
        mf.init_file(p);
        while(mf.fgets(buf, sizeof(buf))) {
            if (match_tag(buf, "</project_init>")) break;
            else if (parse_str(buf, "<name>", name, 256)) continue;
            else if (parse_str(buf, "<url>", url)) {
                canonicalize_master_url(url);
                continue;
            } else if (parse_str(buf, "<account_key>", account_key, 256)) {
                continue;
            }
        }
        fclose(p);
    }
    return 0;
}

int PROJECT_INIT::remove() {
    clear();
    return boinc_delete_file(PROJECT_INIT_FILENAME);
}

void ACCOUNT_IN::parse(const char* buf) {
    url = "";
    email_addr = "";
    passwd_hash = "";
    user_name = "";

    parse_str(buf, "<url>", url);
    parse_str(buf, "<email_addr>", email_addr);
    parse_str(buf, "<passwd_hash>", passwd_hash);
    parse_str(buf, "<user_name>", user_name);
    canonicalize_master_url(url);
}

int GET_PROJECT_CONFIG_OP::do_rpc(const std::string& master_url) {
    int retval;
    std::string url = master_url;

    canonicalize_master_url(url);

    url += "get_project_config.php";

    msg_printf(NULL, MSG_INFO,
        "Fetching configuration file from %s", url.c_str()
    );

    retval = gui_http->do_rpc(this, url, GET_PROJECT_CONFIG_FILENAME);
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

void GET_PROJECT_CONFIG_OP::handle_reply(int http_op_retval) {
    if (http_op_retval) {
        error_num = http_op_retval;
    } else {
        error_num = read_file_string(GET_PROJECT_CONFIG_FILENAME, reply);
    }
}

int LOOKUP_ACCOUNT_OP::do_rpc(const ACCOUNT_IN& ai) {
    int retval;
    std::string url(ai.url);

    canonicalize_master_url(url);

    url += "lookup_account.php?email_addr=";
    url += escape_url(ai.email_addr);

    url += "&passwd_hash=";
    url += escape_url(ai.passwd_hash);

    retval = gui_http->do_rpc(this, url, LOOKUP_ACCOUNT_FILENAME);
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

void LOOKUP_ACCOUNT_OP::handle_reply(int http_op_retval) {
    if (http_op_retval) {
        error_num = http_op_retval;
    } else {
        error_num = read_file_string(LOOKUP_ACCOUNT_FILENAME, reply);
    }
}

int CREATE_ACCOUNT_OP::do_rpc(const ACCOUNT_IN& ai) {
    int retval;
    std::string url(ai.url);

    canonicalize_master_url(url);

    url += "create_account.php?email_addr=";
    url += escape_url(ai.email_addr);

    url += "&passwd_hash=";
    url += escape_url(ai.passwd_hash);

    url += "&user_name=";
    url += escape_url(ai.user_name);

    retval = gui_http->do_rpc(this, url, CREATE_ACCOUNT_FILENAME);
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

void CREATE_ACCOUNT_OP::handle_reply(int http_op_retval) {
    if (http_op_retval) {
        error_num = http_op_retval;
    } else {
        error_num = read_file_string(CREATE_ACCOUNT_FILENAME, reply);
    }
}
#ifdef ENABLE_UPDATE_CHECK
int GET_CURRENT_VERSION_OP::do_rpc() {
    int retval;

    const std::string download_list_url("http://boinc.berkeley.edu/download.php?xml=1");
    retval = gui_http->do_rpc(
        this, download_list_url, GET_CURRENT_VERSION_FILENAME
    );
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

static bool is_version_newer(const char* p) {
    int maj=0, min=0, rel=0;

    sscanf(p, "%d.%d.%d", &maj, &min, &rel);
    if (maj > gstate.core_client_version.major) return true;
    if (maj < gstate.core_client_version.major) return false;
    if (min > gstate.core_client_version.minor) return true;
    if (min < gstate.core_client_version.minor) return false;
    if (rel > gstate.core_client_version.release) return true;
    return false;
}

static bool parse_version(FILE* f, char* new_version) {
    char buf[256], buf2[256];
    bool same_platform = false, newer_version = false;
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</version>")) {
            return (same_platform && newer_version);
        }
        if (parse_str(buf, "<dbplatform>", buf2, sizeof(buf2))) {
            same_platform = (gstate.get_primary_platform() == buf2);
        }
        if (parse_str(buf, "<version_num>", buf2, sizeof(buf2))) {
            newer_version = is_version_newer(buf2);
            strcpy(new_version, buf2);
        }
    }
    return false;
}

void GET_CURRENT_VERSION_OP::handle_reply(int http_op_retval) {
    char buf[256], new_version[256];
    if (http_op_retval) {
        error_num = http_op_retval;
        return;
    }
    gstate.new_version_check_time = gstate.now;
    FILE* f = boinc_fopen(GET_CURRENT_VERSION_FILENAME, "r");
    if (f) {
        while (fgets(buf, 256, f)) {
            if (match_tag(buf, "<version>")) {
                if (parse_version(f, new_version)) {
                    msg_printf(0, MSG_USER_ERROR,
                        "A new version of BOINC (%s) is available for your computer",
                        new_version
                    );
                    msg_printf(0, MSG_USER_ERROR,
                        "Visit http://boinc.berkeley.edu/download.php to get it."
                    );
                    gstate.newer_version = std::string(new_version);
                    break;
                }
            }
        }
        fclose(f);
    }
}

#define NEW_VERSION_CHECK_PERIOD (14*86400)

void CLIENT_STATE::new_version_check() {
    if (( new_version_check_time == 0) ||
        (now - new_version_check_time > NEW_VERSION_CHECK_PERIOD)) {
            // get_current_version_op.handle_reply() will update new_version_check_time
            get_current_version_op.do_rpc();
        }
}
#endif

