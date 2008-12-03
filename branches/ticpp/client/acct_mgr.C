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

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include <cstring>

#include "acct_mgr.h"

#include "error_numbers.h"
#include "client_msgs.h"
#include "str_util.h"
#include "file_names.h"
#include "filesys.h"
#include "client_state.h"
#include "gui_http.h"
#include "crypt.h"
#include "ticpp/ticpp.h"

static const char *run_mode_name[] = {"", "always", "auto", "never"};

/// Read the value for an instance of OPTINAL_BOOL from a stream.
/// This will be used for xml-parsing.
///
/// \param[in] in Reference to the stream object.
/// \param[in] out Reference to the OPTIONAL_BOOL instance which is the target.
/// \return The parameter \a in.
std::istream& operator >> (std::istream& in, OPTIONAL_BOOL& out) {
    bool tmp;
    in >> tmp;
    out.set(tmp);
    return in;
}

/// Read the value for an instance of OPTIONAL_DOUBLE from a stream.
/// This will be used for xml-parsing.
///
/// \param[in] in Reference to the stream object.
/// \param[in] out Reference to the OPTIONAL_DOUBLE instance which is the target.
/// \return The parameter \a in.
std::istream& operator >> (std::istream& in, OPTIONAL_DOUBLE& out) {
    double tmp;
    in >> tmp;
    out.set(tmp);
    return in;
}

ACCT_MGR_OP::ACCT_MGR_OP() {
}

/// do an account manager RPC;
/// if URL is null, detach from current account manager
int ACCT_MGR_OP::do_rpc(
    const std::string& _url, const std::string& name,
    const std::string& password_hash, bool _via_gui
) {
    int retval;
    unsigned int i;
    std::string url(_url);
    FILE *pwdf;

    error_num = ERR_IN_PROGRESS;
    via_gui = _via_gui;

    // if null URL, detach from current AMS
    if ((url.empty()) && (!gstate.acct_mgr_info.acct_mgr_url.empty())) {
        msg_printf(NULL, MSG_INFO, "Removing account manager info");
        gstate.acct_mgr_info.clear();
        boinc_delete_file(ACCT_MGR_URL_FILENAME);
        boinc_delete_file(ACCT_MGR_LOGIN_FILENAME);
        error_num = 0;
        for (i=0; i<gstate.projects.size(); i++) {
            PROJECT* p = gstate.projects[i];
            p->attached_via_acct_mgr = false;
            p->ams_resource_share = 0;
        }
        return 0;
    }

    canonicalize_master_url(url);
    if (!valid_master_url(url.c_str())) {
        error_num = ERR_INVALID_URL;
        return 0;
    }

    ami.acct_mgr_url = url;
    ami.acct_mgr_name.clear();
    ami.login_name = name;
    ami.password_hash = password_hash;

    FILE* f = boinc_fopen(ACCT_MGR_REQUEST_FILENAME, "w");
    if (!f) return ERR_FOPEN;
    fprintf(f,
        "<acct_mgr_request>\n"
        "   <name>%s</name>\n"
        "   <password_hash>%s</password_hash>\n"
        "   <host_cpid>%s</host_cpid>\n"
        "   <domain_name>%s</domain_name>\n"
        "   <client_version>%d.%d.%d</client_version>\n"
        "   <run_mode>%s</run_mode>\n",
        name.c_str(), password_hash.c_str(),
        gstate.host_info.host_cpid,
        gstate.host_info.domain_name,
        gstate.boinc_compat_version.major,
        gstate.boinc_compat_version.minor,
        gstate.boinc_compat_version.release,
        run_mode_name[gstate.run_mode.get_perm()]
    );
    if (!gstate.acct_mgr_info.previous_host_cpid.empty()) {
        fprintf(f,
            "   <previous_host_cpid>%s</previous_host_cpid>\n",
            gstate.acct_mgr_info.previous_host_cpid.c_str()
        );
    }

    // If the AMS requested it, send GUI RPC port and password hash.
    // This is for the "farm" account manager so it
    // can know where to send GUI RPC requests to
    // without having to configure each host
    //
    if (gstate.acct_mgr_info.send_gui_rpc_info) {
        if (gstate.cmdline_gui_rpc_port) {
            fprintf(f,"   <gui_rpc_port>%d</gui_rpc_port>\n", gstate.cmdline_gui_rpc_port);
        } else {
            fprintf(f,"   <gui_rpc_port>%d</gui_rpc_port>\n", GUI_RPC_PORT);
        }
        if (boinc_file_exists(GUI_RPC_PASSWD_FILE)) {
            char password[256];
            strcpy(password, "");
            pwdf = fopen(GUI_RPC_PASSWD_FILE, "r");
            if (pwdf) {
                if (fgets(password, 256, pwdf)) {
                    strip_whitespace(password);
                }
                fclose(pwdf);
            }
            fprintf(f,"   <gui_rpc_password>%s</gui_rpc_password>\n", password);
        }
    }
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        fprintf(f,
            "   <project>\n"
            "      <url>%s</url>\n"
            "      <project_name>%s</project_name>\n"
            "      <suspended_via_gui>%d</suspended_via_gui>\n"
            "      <account_key>%s</account_key>\n"
            "      <hostid>%d</hostid>\n"
            "%s"
            "   </project>\n",
            p->master_url,
            p->project_name,
            p->suspended_via_gui,
            p->authenticator,
            p->hostid,
            p->attached_via_acct_mgr?"      <attached_via_acct_mgr/>\n":""
        );
    }
    if (boinc_file_exists(GLOBAL_PREFS_FILE_NAME)) {
        FILE* fprefs = fopen(GLOBAL_PREFS_FILE_NAME, "r");
        if (fprefs) {
            copy_stream(fprefs, f);
            fclose(fprefs);
        }
    }
    if (!gstate.acct_mgr_info.opaque.empty()) {
        fprintf(f, "   <opaque>\n%s\n   </opaque>\n", gstate.acct_mgr_info.opaque);
    }
    fprintf(f, "</acct_mgr_request>\n");
    fclose(f);
    std::string buf = url + std::string("rpc.php");
    retval = gstate.gui_http.do_rpc_post(
        this, buf, ACCT_MGR_REQUEST_FILENAME, ACCT_MGR_REPLY_FILENAME
    );
    if (retval) {
        error_num = retval;
        return retval;
    }
    msg_printf(NULL, MSG_INFO, "Contacting account manager at %s", url.c_str());

    return 0;
}

void AM_ACCOUNT::parse(const ticpp::Element* am_account) {
    detach = false;
    update = false;
    dont_request_more_work.init();
    detach_when_done.init();
    url.clear();
    url_signature.clear();
    authenticator.clear();
    resource_share.init();

    // URL field is mandatory, therfore parse it directly. This will throw
    // an exception if the requested element is not present.
    am_account->FirstChildElement("url")->GetText(&url);

    std::string child_name;
    for (ticpp::Element* child = am_account->FirstChildElement(false); child; child = child->NextSiblingElement(false)) {
        child->GetValue(&child_name);

        if      (child_name == "authenticator")          child->GetText(&authenticator);
        else if (child_name == "detach")              child->GetText(&detach);
        else if (child_name == "update")         child->GetText(&update);
        else if (child_name == "dont_request_more_work")         child->GetText(&dont_request_more_work);
        else if (child_name == "detach_when_done")         child->GetText(&detach_when_done);
        else if (child_name == "url_signature") {
            child->GetText(&url_signature);
            url_signature += '\n';
        } else if (child_name == "resource_share") {
            child->GetText(&resource_share);
            if (resource_share.value <= 0.0) {
                msg_printf(NULL, MSG_INFO, "Resource share out of range: %f", resource_share.value);
                resource_share.init();
            }
        } else if (log_flags.unparsed_xml) {
            msg_printf(NULL, MSG_INFO, "[unparsed_xml] AM_ACCOUNT: unrecognized %s", child_name.c_str());
        }
    }
}

void ACCT_MGR_OP::parse(const ticpp::Element* acct_mgr_reply) {
    accounts.clear();
    error_str.clear();
    error_num = 0;
    repeat_sec = 0;
    host_venue.clear();
    ami.opaque.clear();

    std::string child_name;
    for (ticpp::Element* child = acct_mgr_reply->FirstChildElement(false); child; child = child->NextSiblingElement(false)) {
        child->GetValue(&child_name);

        if      (child_name == "error_num")          child->GetText(&error_num);
        else if (child_name == "error")              child->GetText(&error_str);
        else if (child_name == "repeat_sec")         child->GetText(&repeat_sec);
        else if (child_name == "host_venue")         child->GetText(&host_venue);
        else if (child_name == "name")               child->GetText(&ami.acct_mgr_name);
        else if (child_name == "opaque")             child->GetText(&ami.opaque);
        else if (child_name == "signing_key")        child->GetText(&ami.signing_key);
        else if (child_name == "global_preferences") child->GetText(&global_prefs_xml);
        else if (child_name == "message") {
            std::string message;
            child->GetText(&message);
            msg_printf(NULL, MSG_INFO, "Account manager: %s", message.c_str());
        } else if (child_name == "account") {
            try {
                AM_ACCOUNT account;
                account.parse(child);
                accounts.push_back(account);
            } catch (ticpp::Exception& ex) {
                msg_printf(NULL, MSG_INTERNAL_ERROR, "Can't parse account in account manager reply: %s", ex.what());
            }
        } else if (log_flags.unparsed_xml) {
            msg_printf(NULL, MSG_INFO, "[unparsed_xml] ACCT_MGR_OP::parse: unrecognized %s", child_name.c_str());
        }
    }
}

void ACCT_MGR_OP::handle_reply(int http_op_retval) {
    unsigned int i;
    int retval;
    bool verified;
    PROJECT* pp;
    bool sig_ok;

    if (http_op_retval == 0) {
        try {
            ticpp::Document doc(ACCT_MGR_REPLY_FILENAME);
            doc.LoadFile();
            parse(doc.FirstChildElement("acct_mgr_reply"));
            retval = 0;
        } catch (ticpp::Exception&) {
            retval = ERR_XML_PARSE;
        }
    } else {
        error_num = http_op_retval;
    }

    gstate.acct_mgr_info.password_error = false;
    if (error_num == ERR_BAD_PASSWD && !via_gui) {
        gstate.acct_mgr_info.password_error = true;
    }
    // check both error_str and error_num since an account manager may only
    // return a BOINC based error code for password failures or invalid
    // email addresses
    if (!error_str.empty()) {
        msg_printf(NULL, MSG_USER_ERROR,
            "Account manager error: %d %s", error_num, error_str.c_str()
        );
        if (!error_num) {
            error_num = ERR_XML_PARSE;
        }
    } else if (error_num) {
        msg_printf(NULL, MSG_USER_ERROR,
            "Account manager error: %s", boincerror(error_num)
        );
    }

    if (error_num) {
        return;
    }

    msg_printf(NULL, MSG_INFO, "Account manager contact succeeded");

    // demand a signing key
    sig_ok = true;
    if (ami.signing_key.empty()) {
        msg_printf(NULL, MSG_INTERNAL_ERROR, "No signing key from account manager");
        sig_ok = false;
    }

    // don't accept new signing key if we already have one
    if ((!gstate.acct_mgr_info.signing_key.empty()) && (gstate.acct_mgr_info.signing_key != ami.signing_key)) {
        msg_printf(NULL, MSG_INTERNAL_ERROR, "Inconsistent signing key from account manager");
        sig_ok = false;
    }

    if (sig_ok) {
        gstate.acct_mgr_info.acct_mgr_url = ami.acct_mgr_url;
        gstate.acct_mgr_info.acct_mgr_name = ami.acct_mgr_name;
        gstate.acct_mgr_info.signing_key = ami.signing_key;
        gstate.acct_mgr_info.login_name = ami.login_name;
        gstate.acct_mgr_info.password_hash = ami.password_hash;
        gstate.acct_mgr_info.opaque = ami.opaque;

        // process projects
        for (i = 0; i < accounts.size(); ++i) {
            AM_ACCOUNT& acct = accounts[i];
            retval = verify_string2(acct.url.c_str(), acct.url_signature.c_str(), ami.signing_key.c_str(), verified);
            if (retval || !verified) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Bad signature for URL %s", acct.url.c_str());
                continue;
            }
            pp = gstate.lookup_project(acct.url.c_str());
            if (pp) {
                if (acct.detach) {
                    gstate.detach_project(pp);
                } else {
                    // BAM! leaves authenticator blank if our request message
                    // had the current account info
                    if (!acct.authenticator.empty() && strcmp(pp->authenticator, acct.authenticator.c_str())) {
                        msg_printf(pp, MSG_INFO,
                            "Already attached under another account"
                        );
                    } else {
                        //msg_printf(pp, MSG_INFO, "Already attached");
                        pp->attached_via_acct_mgr = true;
                        if (acct.dont_request_more_work.present) {
                            pp->dont_request_more_work = acct.dont_request_more_work.value;
                        }
                        if (acct.detach_when_done.present) {
                            pp->detach_when_done = acct.detach_when_done.value;
                            if (pp->detach_when_done) {
                                pp->dont_request_more_work = true;
                            }
                        }

                        // initiate a scheduler RPC if requested by AMS
                        if (acct.update) {
                            pp->sched_rpc_pending = RPC_REASON_ACCT_MGR_REQ;
                            pp->min_rpc_time = 0;
                        }
                        if (acct.resource_share.present) {
                            pp->ams_resource_share = acct.resource_share.value;
                            pp->resource_share = pp->ams_resource_share;
                        } else {
                            // no host-specific resource share;
                            // if currently have one, restore to value from web
                            if (pp->ams_resource_share >= 0) {
                                pp->ams_resource_share = -1;
                                PROJECT p2;
                                strcpy(p2.master_url, pp->master_url);
                                retval = p2.parse_account_file();
                                if (!retval) {
                                    pp->resource_share = p2.resource_share;
                                } else {
                                    pp->resource_share = 100;
                                }
                            }
                        }
                    }
                }
            } else {
                if (!acct.detach) {
                    msg_printf(NULL, MSG_INFO, "Attaching to %s", acct.url.c_str());
                    gstate.add_project(acct.url.c_str(), acct.authenticator.c_str(), "", true);
                    if (acct.dont_request_more_work.present) {
                        pp->dont_request_more_work = acct.dont_request_more_work.value;
                    }
                }
            }
        }

        bool read_prefs = false;
        if ((!host_venue.empty()) && (host_venue != gstate.main_host_venue)) {
            strlcpy(gstate.main_host_venue, host_venue.c_str(), sizeof(gstate.main_host_venue));
            read_prefs = true;
        }

        // process prefs if any
        if (!global_prefs_xml.empty()) {
            retval = gstate.save_global_prefs(global_prefs_xml.c_str(), ami.acct_mgr_url.c_str(), ami.acct_mgr_url.c_str());
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR, "Can't save global prefs");
            }
            read_prefs = true;
        }

        // process prefs if prefs or venue changed
        if (read_prefs) {
            gstate.read_global_prefs();
        }
    }

    gstate.acct_mgr_info.previous_host_cpid = gstate.host_info.host_cpid;
    if (repeat_sec) {
        gstate.acct_mgr_info.next_rpc_time = gstate.now + repeat_sec;
    } else {
        gstate.acct_mgr_info.next_rpc_time = gstate.now + 86400;
    }
    gstate.acct_mgr_info.write_info();
    gstate.set_client_state_dirty("account manager RPC");
}

int ACCT_MGR_INFO::write_info() {
    FILE* p;
    if (!acct_mgr_url.empty()) {
        p = fopen(ACCT_MGR_URL_FILENAME, "w");
        if (p) {
            fprintf(p,
                "<acct_mgr>\n"
                "    <name>%s</name>\n"
                "    <url>%s</url>\n",
                acct_mgr_name.c_str(),
                acct_mgr_url.c_str()
            );
            if (send_gui_rpc_info) fprintf(p,"    <send_gui_rpc_info/>\n");
            if (!signing_key.empty()) {
                fprintf(p, "    <signing_key>\n%s\n</signing_key>\n", signing_key.c_str());
            }
            fprintf(p, "</acct_mgr>\n");
            fclose(p);
        }
    }

    if (!login_name.empty()) {
        p = fopen(ACCT_MGR_LOGIN_FILENAME, "w");
        if (p) {
            fprintf(
                p,
                "<acct_mgr_login>\n"
                "    <login>%s</login>\n"
                "    <password_hash>%s</password_hash>\n"
                "    <previous_host_cpid>%s</previous_host_cpid>\n"
                "    <next_rpc_time>%f</next_rpc_time>\n"
                "    <opaque>\n%s\n"
                "    </opaque>\n"
                "</acct_mgr_login>\n",
                login_name.c_str(),
                password_hash.c_str(),
                previous_host_cpid.c_str(),
                next_rpc_time,
                opaque.c_str()
            );
            fclose(p);
        }
    }
    return 0;
}

void ACCT_MGR_INFO::clear() {
    acct_mgr_name.clear();
    acct_mgr_url.clear();
    login_name.clear();
    password_hash.clear();
    signing_key.clear();
    previous_host_cpid.clear();
    opaque.clear();
    next_rpc_time = 0;
    send_gui_rpc_info = false;
    password_error = false;
}

ACCT_MGR_INFO::ACCT_MGR_INFO() {
    clear();
}

void ACCT_MGR_INFO::parse_login_file(const ticpp::Element* acct_mgr_login) {
    std::string child_name;
    for (ticpp::Element* child = acct_mgr_login->FirstChildElement(false); child; child = child->NextSiblingElement(false)) {
        child->GetValue(&child_name);

        if      (child_name == "login")              child->GetText(&login_name);
        else if (child_name == "password_hash")      child->GetText(&password_hash);
        else if (child_name == "previous_host_cpid") child->GetText(&previous_host_cpid);
        else if (child_name == "next_rpc_time")      child->GetText(&next_rpc_time);
        else if (child_name == "opaque")             child->GetText(&opaque);
        else if (log_flags.unparsed_xml) {
            msg_printf(NULL, MSG_INFO, "[unparsed_xml] ACCT_MGR_INFO::parse_login: unrecognized %s", child_name.c_str());
        }
    }
}

int ACCT_MGR_INFO::init() {
    clear();
    try {
        ticpp::Document acct_mgr_url(ACCT_MGR_URL_FILENAME);
        acct_mgr_url.LoadFile();

        ticpp::Element* acct_mgr = acct_mgr_url.FirstChildElement("acct_mgr");
        std::string child_name;
        for (ticpp::Element* child = acct_mgr->FirstChildElement(false); child; child = child->NextSiblingElement(false)) {
            child->GetValue(&child_name);

            if      (child_name == "name")              child->GetText(&acct_mgr_name);
            else if (child_name == "url")               child->GetText(&acct_mgr_url);
            else if (child_name == "send_gui_rpc_info") child->GetText(&send_gui_rpc_info);
            else if (child_name == "signing_key")       child->GetText(&signing_key);
            else if (log_flags.unparsed_xml) {
                msg_printf(NULL, MSG_INFO, "[unparsed_xml] ACCT_MGR_INFO::init: unrecognized %s", child_name.c_str());
            }
        }

        ticpp::Document acct_mgr_login(ACCT_MGR_LOGIN_FILENAME);
        acct_mgr_login.LoadFile();

        parse_login_file(acct_mgr_login.FirstChildElement("acct_mgr_login"));
    } catch (ticpp::Exception&) {
        // Those errors were just ignored...
    }
    return 0;
}

bool ACCT_MGR_INFO::poll() {
    if (gstate.acct_mgr_op.error_num == ERR_IN_PROGRESS) return false;

    if ((login_name.empty()) && (password_hash.empty())) {
        return false;
    }

    if (gstate.now > next_rpc_time) {
        next_rpc_time = gstate.now + 86400;
        gstate.acct_mgr_op.do_rpc(acct_mgr_url, login_name, password_hash, false);
        return true;
    }
    return false;
}
