// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include <fstream>
#include <sstream>
#endif

#include "scheduler_op.h"

#include "str_util.h"
#include "util.h"
#include "parse.h"
#include "miofile.h"
#include "error_numbers.h"
#include "filesys.h"

#include "client_state.h"
#include "client_types.h"
#include "client_msgs.h"
#include "file_names.h"
#include "log_flags.h"
#include "main.h"

SCHEDULER_OP::SCHEDULER_OP(HTTP_OP_SET* h) {
    cur_proj = NULL;
    state = SCHEDULER_OP_STATE_IDLE;
    http_op.http_op_state = HTTP_STATE_IDLE;
    http_ops = h;
}

/// See if there's a pending master file fetch.
/// If so, start it and return true.
bool SCHEDULER_OP::check_master_fetch_start() {
    int retval;

    PROJECT* p = gstate.next_project_master_pending();
    if (!p) return false;
    retval = init_master_fetch(p);
    if (retval) {
        msg_printf(p, MSG_INTERNAL_ERROR,
            "Couldn't start download of scheduler list: %s", boincerror(retval)
        );
        p->master_fetch_failures++;
        backoff(p, "scheduler list fetch failed\n");
        return false;
    }
    msg_printf(p, MSG_INFO, "Fetching scheduler list");
    return true;
}

/// Try to get work from eligible project with biggest long term debt.
/// PRECONDITION: \link CLIENT_STATE::compute_work_requests compute_work_requests()\endlink has been called
/// to fill in PROJECT::work_request
/// and CLIENT_STATE::overall_work_fetch_urgency
int SCHEDULER_OP::init_get_work() {
    int retval;

    PROJECT* p = gstate.next_project_need_work();
    if (p) {
        retval = init_op_project(p, RPC_REASON_NEED_WORK);
        if (retval) {
            return retval;
        }
    }
    return 0;
}


/// Try to initiate an RPC to the given project.
/// If there are multiple schedulers, start with a random one.
/// User messages and backoff() is done at this level.
///
/// \param[in] p Pointer to a PROJECT instance for the project a rpc should
///              be initiated for.
/// \param[in] r The reason for initiating a rpc.
/// \return Zero on success, nonzero on error.
int SCHEDULER_OP::init_op_project(PROJECT* p, rpc_reason r) {
    reason = r;
    if (log_flags.sched_op_debug) {
        msg_printf(p, MSG_INFO, "[sched_op_debug] Starting scheduler request");
    }

    // If project has no schedulers,
    // skip everything else and just get its master file.
    if (p->scheduler_urls.empty()) {
        int retval = init_master_fetch(p);
        if (retval) {
            std::ostringstream err_msg;
            err_msg << "Scheduler list fetch initialization failed: " << retval << '\n';
            backoff(p, err_msg.str());
        }
        return retval;
    }

    if (reason == RPC_REASON_INIT) {
        p->work_request = 1;
        if (!gstate.cpu_benchmarks_done()) {
            gstate.cpu_benchmarks_set_defaults();
        }
    }

    url_index = 0;
    int retval = gstate.make_scheduler_request(p);
    if (!retval) {
        retval = start_rpc(p);
    }
    if (retval) {
        std::ostringstream err_msg;
        err_msg << "scheduler request to " << p->get_scheduler_url(url_index, url_random)
                << " failed: " << boincerror(retval) << '\n';
        backoff(p, err_msg.str());
    } else {
        // RPC started OK, so we must have network connectivity.
        // Now's a good time to check for new BOINC versions
        // and project list
#ifdef ENABLE_UPDATE_CHECK
        gstate.new_version_check();
#endif
    }
    return retval;
}

/// Back off contacting this project's schedulers,
/// and output an error msg if needed.
///
/// One of the following errors occurred:
/// - connection failure in fetching master file
/// - connection failure in scheduler RPC
/// - got master file, but it didn't have any <scheduler> elements
/// - tried all schedulers, none responded
/// - sent nonzero work request, got a reply with no work
///
/// \param[in] p Pointer to a PROJECT instance of the project for which a
///              back off is requested.
/// \param[in] reason_msg A string describing the reason for the requested
///                       back off.
void SCHEDULER_OP::backoff(PROJECT* p, const std::string& reason_msg) {
    if (p->master_fetch_failures >= gstate.master_fetch_retry_cap) {
        std::ostringstream buf;
        buf << p->master_fetch_failures << "consecutive failures fetching scheduler list";
        p->master_url_fetch_pending = true;
        p->set_min_rpc_time(gstate.now + gstate.master_fetch_interval, buf.str().c_str());
        return;
    }

    // If nrpc failures is master_fetch_period,
    // then set master_url_fetch_pending and initialize again.
    if (p->nrpc_failures == gstate.master_fetch_period) {
        p->master_url_fetch_pending = true;
        p->min_rpc_time = 0;
        p->nrpc_failures = 0;
        p->master_fetch_failures++;
    }

    // If network is down, don't count it as RPC failure.
    if (!net_status.need_physical_connection) {
        p->nrpc_failures++;
    }

    int n = p->nrpc_failures;
    if (n > gstate.retry_cap) {
        n = gstate.retry_cap;
    }
    double exp_backoff = calculate_exponential_backoff(n, gstate.sched_retry_delay_min, gstate.sched_retry_delay_max);
    p->set_min_rpc_time(gstate.now + exp_backoff, reason_msg.c_str());
}

/// Low-level routine to initiate an RPC.
/// If successful, creates an HTTP_OP that must be polled.
/// PRECONDITION: the request file has been created.
int SCHEDULER_OP::start_rpc(PROJECT* p) {
    int retval;
    char request_file[1024], reply_file[1024];

    // if requesting work, round up to 1 sec
    //
    if (p->work_request>0 && p->work_request<1) {
        p->work_request = 1;
    }

    safe_strcpy(scheduler_url, p->get_scheduler_url(url_index, url_random));
    if (log_flags.sched_ops) {
        msg_printf(p, MSG_INFO,
            "Sending scheduler request: %s.  Requesting %.0f seconds of work, reporting %d completed tasks",
            rpc_reason_string(reason), p->work_request, p->nresults_returned
        );
    }

    get_sched_request_filename(*p, request_file, sizeof(request_file));
    get_sched_reply_filename(*p, reply_file, sizeof(reply_file));

    http_op.set_proxy(&gstate.proxy_info);
    retval = http_op.init_post(scheduler_url, request_file, reply_file);
    if (retval) {
        if (log_flags.sched_ops) {
            msg_printf(p, MSG_INFO,
                "Scheduler request failed: %s", boincerror(retval)
            );
        }
        backoff(p, "scheduler request failed");
        return retval;
    }
    retval = http_ops->insert(&http_op);
    if (retval) {
        if (log_flags.sched_ops) {
            msg_printf(p, MSG_INFO,
                "Scheduler request failed: %s", boincerror(retval)
            );
        }
        backoff(p, "scheduler request failed");
        return retval;
    }
    p->rpc_seqno++;
    cur_proj = p;    // remember what project we're talking to
    state = SCHEDULER_OP_STATE_RPC;
    return 0;
}

/// Initiate a fetch of a project's master URL file.
int SCHEDULER_OP::init_master_fetch(PROJECT* p) {
    int retval;
    char master_filename[256];

    get_master_filename(*p, master_filename, sizeof(master_filename));

    if (log_flags.sched_op_debug) {
        msg_printf(p, MSG_INFO, "[sched_op_debug] Fetching master file");
    }
    http_op.set_proxy(&gstate.proxy_info);
    retval = http_op.init_get(p->master_url, master_filename, true);
    if (retval) return retval;
    retval = http_ops->insert(&http_op);
    if (retval) return retval;
    cur_proj = p;
    state = SCHEDULER_OP_STATE_GET_MASTER;
    return 0;
}

/// Parse a master file.
///
/// \param[in] p Pointer to a PROJECT instance for which the master file
///              should be parsed.
/// \return A vector of strings containing all scheduler URLs read from 
///         the master file.
std::vector<std::string> SCHEDULER_OP::parse_master_file(PROJECT* p) const {
    std::vector<std::string> urls;
    std::string master_filename = get_master_filename(*p);
    std::ifstream in(master_filename.c_str());
    if (!in) {
        msg_printf(p, MSG_INTERNAL_ERROR, "Can't open scheduler list file");
        return urls;
    }
    p->scheduler_urls.clear();
    std::string buf;
    while (std::getline(in, buf)) {

        // allow for the possibility of > 1 tag per line here
        // (UMTS may collapse lines)
        std::string buf2(buf);
        std::string str;
        while (parse_str(buf2.c_str(), "<scheduler>", str)) {
            push_unique(str, urls);
            std::string::size_type pos = buf2.find("</scheduler>");
            if (pos != std::string::npos) {
                buf2.erase(0, pos + 12);
            }
        }

        // check for new syntax: <link ...>
        std::string::size_type pos = 0;
        while (pos != std::string::npos) {
            pos = buf.find("<link rel=\"boinc_scheduler\" href=\"", pos);
            if (pos != std::string::npos) {
                pos += 34;
                std::string::size_type end = buf.find('\"', pos);
                if (end != std::string::npos) {
                    str = buf.substr(pos, end - pos);
                    strip_whitespace(str);
                    push_unique(str, urls);
                }
            }
        }
    }
    in.close();
    if (log_flags.sched_op_debug) {
        msg_printf(p, MSG_INFO,
            "[sched_op_debug] Found %lu scheduler URLs in master file\n",
            urls.size()
        );
    }
    return urls;
}

/// A master file has just been read,
/// transfer scheduler URLs to project.
/// Return true if any of them is new.
bool SCHEDULER_OP::update_urls(PROJECT* p, std::vector<std::string> &urls) {
    unsigned int i, j;
    bool found, any_new;

    any_new = false;
    for (i=0; i<urls.size(); i++) {
        found = false;
        for (j=0; j<p->scheduler_urls.size(); j++) {
            if (urls[i] == p->scheduler_urls[j]) {
                found = true;
                break;
            }
        }
        if (!found) any_new = true;
    }

    p->scheduler_urls.clear();
    for (i=0; i<urls.size(); i++) {
        p->scheduler_urls.push_back(urls[i]);
    }

    return any_new;
}

/// Poll routine. If an operation is in progress, check for completion.
///
/// \return True if some action was performed.
bool SCHEDULER_OP::poll() {
    int retval, nresults;
    bool changed, scheduler_op_done;

    switch(state) {
    case SCHEDULER_OP_STATE_GET_MASTER:
        // here we're fetching the master file for a project
        if (http_op.http_op_state == HTTP_STATE_DONE) {
            state = SCHEDULER_OP_STATE_IDLE;
            cur_proj->master_url_fetch_pending = false;
            http_ops->remove(&http_op);
            gstate.set_client_state_dirty("master URL fetch done");
            if (http_op.http_op_retval == 0) {
                if (log_flags.sched_op_debug) {
                    msg_printf(cur_proj, MSG_INFO,
                        "[sched_op_debug] Got master file; parsing"
                    );
                }
                std::vector<std::string> urls = parse_master_file(cur_proj);
                if (urls.empty()) {
                    // master file parse failed.
                    cur_proj->master_fetch_failures++;
                    backoff(cur_proj, "Couldn't parse scheduler list");
                } else {
                    // parse succeeded
                    msg_printf(cur_proj, MSG_INFO, "Master file download succeeded");
                    cur_proj->master_fetch_failures = 0;
                    changed = update_urls(cur_proj, urls);

                    // reenable scheduler RPCs if have new URLs
                    if (changed) {
                        cur_proj->min_rpc_time = 0;
                        cur_proj->nrpc_failures = 0;
                    }
                }
            } else {
                // master file fetch failed.
                char buf[256];
                sprintf(buf, "Scheduler list fetch failed: %s",
                    boincerror(http_op.http_op_retval)
                );
                cur_proj->master_fetch_failures++;
                backoff(cur_proj, buf);
            }
            gstate.request_work_fetch("Master fetch complete");
            cur_proj = NULL;
            return true;
        }
        break;
    case SCHEDULER_OP_STATE_RPC:

        // here we're doing a scheduler RPC
        scheduler_op_done = false;
        if (http_op.http_op_state == HTTP_STATE_DONE) {
            state = SCHEDULER_OP_STATE_IDLE;
            http_ops->remove(&http_op);
            if (http_op.http_op_retval) {
                if (log_flags.sched_ops) {
                    msg_printf(cur_proj, MSG_INFO,
                        "Scheduler request failed: %s", http_op.error_msg
                    );
                }

                // scheduler RPC failed.  Try another scheduler if one exists
                while (1) {
                    url_index++;
                    if (url_index == (int)cur_proj->scheduler_urls.size()) {
                        break;
                    }
                    retval = start_rpc(cur_proj);
                    if (!retval) return true;
                }
                if (url_index == (int) cur_proj->scheduler_urls.size()) {
                    backoff(cur_proj, "scheduler request failed");
                    scheduler_op_done = true;

                    // if project suspended, don't retry failed RPC
                    if (cur_proj->suspended_via_gui) {
                        cur_proj->sched_rpc_pending = NO_RPC_REASON;
                    }
                }
            } else {
                retval = gstate.handle_scheduler_reply(cur_proj, scheduler_url, nresults);
                switch (retval) {
                case 0:
                    break;
                case ERR_PROJECT_DOWN:
                    backoff(cur_proj, "project is down");
                    break;
                default:
                    backoff(cur_proj, "can't parse scheduler reply");
                    break;
                }
                cur_proj->work_request = 0;    // don't ask again right away
            }
            cur_proj = NULL;
            gstate.request_work_fetch("RPC complete");
            return true;
        }
    }
    return false;
}

void SCHEDULER_OP::abort(PROJECT* p) {
    if (state != SCHEDULER_OP_STATE_IDLE && cur_proj == p) {
        gstate.http_ops->remove(&http_op);
        state = SCHEDULER_OP_STATE_IDLE;
        cur_proj = NULL;
    }
}

SCHEDULER_REPLY::SCHEDULER_REPLY() {
    global_prefs_xml = 0;
    project_prefs_xml = 0;
    code_sign_key = 0;
    code_sign_key_signature = 0;
}

SCHEDULER_REPLY::~SCHEDULER_REPLY() {
    if (global_prefs_xml) free(global_prefs_xml);
    if (project_prefs_xml) free(project_prefs_xml);
    if (code_sign_key) free(code_sign_key);
    if (code_sign_key_signature) free(code_sign_key_signature);
}

/// Parse a scheduler reply.
/// Some of the items go into the SCHEDULER_REPLY object.
/// Others are copied straight to the PROJECT.
int SCHEDULER_REPLY::parse(FILE* in, PROJECT* project) {
    char buf[256], msg_buf[1024], pri_buf[256];
    int retval;
    MIOFILE mf;
    std::string delete_file_name;
    mf.init_file(in);
    bool found_start_tag = false;
    double cpid_time = 0;

    hostid = 0;
    request_delay = 0;
    next_rpc_delay = 0;
    global_prefs_xml = 0;
    project_prefs_xml = 0;
    strcpy(host_venue, project->host_venue);
        // the project won't send us a venue if it's doing maintenance
        // or doesn't check the DB because no work.
        // Don't overwrite the host venue in that case.
    strcpy(master_url, "");
    code_sign_key = 0;
    code_sign_key_signature = 0;
    message_ack = false;
    project_is_down = false;
    send_file_list = false;
    send_time_stats_log = 0;
    send_job_log = 0;
    messages.clear();
    scheduler_version = 0;

    // First line should either be tag (HTTP 1.0) or
    // hex length of response (HTTP 1.1)
    //
    while (fgets(buf, 256, in)) {
        if (!found_start_tag) {
            if (match_tag(buf, "<scheduler_reply")) {
                found_start_tag = true;
            }
            continue;
        }
        if (match_tag(buf, "</scheduler_reply>")) {

            // update statistics after parsing the scheduler reply
            // add new record if vector is empty or we have a new day
            //
            if (project->statistics.empty() || project->statistics.back().day!=dday()) {

                // delete old stats
                while (!project->statistics.empty()) {
                    DAILY_STATS& ds = project->statistics[0];
                    if (dday() - ds.day > config.save_stats_days*86400) {
                        project->statistics.erase(project->statistics.begin());
                    } else {
                        break;
                    }
                }

                DAILY_STATS nds;
                project->statistics.push_back(nds);
            }
            DAILY_STATS& ds = project->statistics.back();
            ds.day=dday();
            ds.user_total_credit=project->user_total_credit;
            ds.user_expavg_credit=project->user_expavg_credit;
            ds.host_total_credit=project->host_total_credit;
            ds.host_expavg_credit=project->host_expavg_credit;

            project->write_statistics_file();

            if (cpid_time) {
                project->cpid_time = cpid_time;
            } else {
                project->cpid_time = project->user_create_time;
            }
            return 0;
        }
        else if (parse_str(buf, "<project_name>", project->project_name, sizeof(project->project_name))) {
            continue;
        }
        else if (parse_str(buf, "<master_url>", master_url, sizeof(master_url))) {
            continue;
        }
        else if (parse_str(buf, "<symstore>", project->symstore, sizeof(project->symstore))) continue;
        else if (parse_str(buf, "<user_name>", project->user_name, sizeof(project->user_name))) continue;
        else if (parse_double(buf, "<user_total_credit>", project->user_total_credit)) continue;
        else if (parse_double(buf, "<user_expavg_credit>", project->user_expavg_credit)) continue;
        else if (parse_double(buf, "<user_create_time>", project->user_create_time)) continue;
        else if (parse_double(buf, "<cpid_time>", cpid_time)) continue;
        else if (parse_str(buf, "<team_name>", project->team_name, sizeof(project->team_name))) continue;
        else if (parse_int(buf, "<hostid>", hostid)) continue;
        else if (parse_double(buf, "<host_total_credit>", project->host_total_credit)) continue;
        else if (parse_double(buf, "<host_expavg_credit>", project->host_expavg_credit)) continue;
        else if (parse_str(buf, "<host_venue>", host_venue, sizeof(host_venue))) continue;
        else if (parse_double(buf, "<host_create_time>", project->host_create_time)) continue;
        else if (parse_double(buf, "<request_delay>", request_delay)) continue;
        else if (parse_double(buf, "<next_rpc_delay>", next_rpc_delay)) continue;
        else if (match_tag(buf, "<global_preferences>")) {
            retval = dup_element_contents(
                in,
                "</global_preferences>",
                &global_prefs_xml
            );
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't parse global prefs in scheduler reply: %s",
                    boincerror(retval)
                );
                return retval;
            }
        } else if (match_tag(buf, "<project_preferences>")) {
            retval = dup_element_contents(
                in,
                "</project_preferences>",
                &project_prefs_xml
            );
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't parse project prefs in scheduler reply: %s",
                    boincerror(retval)
                );
                return retval;
            }
        } else if (match_tag(buf, "<gui_urls>")) {
            std::string parsed_gui_urls;
            retval = copy_element_contents(in, "</gui_urls>", parsed_gui_urls);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't parse GUI URLs in scheduler reply: %s",
                    boincerror(retval)
                );
                return retval;
            }
            project->gui_urls = "<gui_urls>\n" + parsed_gui_urls + "</gui_urls>\n";
            continue;
        } else if (match_tag(buf, "<code_sign_key>")) {
            retval = dup_element_contents(
                in,
                "</code_sign_key>",
                &code_sign_key
            );
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't parse code sign key in scheduler reply: %s",
                    boincerror(retval)
                );
                return ERR_XML_PARSE;
            }
        } else if (match_tag(buf, "<code_sign_key_signature>")) {
            retval = dup_element_contents(
                in,
                "</code_sign_key_signature>",
                &code_sign_key_signature
            );
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't parse code sign key signature in scheduler reply: %s",
                    boincerror(retval)
                );
                return ERR_XML_PARSE;
            }
        } else if (match_tag(buf, "<app>")) {
            APP app;
            retval = app.parse(mf);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't parse application in scheduler reply: %s",
                    boincerror(retval)
                );
            } else {
                apps.push_back(app);
            }
        } else if (match_tag(buf, "<file_info>")) {
            FILE_INFO file_info;
            retval = file_info.parse(mf, true);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't parse file info in scheduler reply: %s",
                    boincerror(retval)
                );
            } else {
                file_infos.push_back(file_info);
            }
        } else if (match_tag(buf, "<app_version>")) {
            APP_VERSION av;
            retval = av.parse(mf);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't parse application version in scheduler reply: %s",
                    boincerror(retval)
                );
            } else {
                app_versions.push_back(av);
            }
        } else if (match_tag(buf, "<workunit>")) {
            WORKUNIT wu;
            retval = wu.parse(mf);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't parse workunit in scheduler reply: %s",
                    boincerror(retval)
                );
            } else {
                workunits.push_back(wu);
            }
        } else if (match_tag(buf, "<result>")) {
            RESULT result;      // make sure this is here so constructor
                                // gets called each time
            retval = result.parse_server(mf);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't parse task in scheduler reply: %s",
                    boincerror(retval)
                );
            } else {
                results.push_back(result);
            }
        } else if (match_tag(buf, "<result_ack>")) {
            RESULT result;
            retval = result.parse_name(in, "</result_ack>");
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't parse ack in scheduler reply: %s",
                    boincerror(retval)
                );
            } else {
                result_acks.push_back(result);
            }
        } else if (match_tag(buf, "<result_abort>")) {
            RESULT result;
            retval = result.parse_name(in, "</result_abort>");
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't parse result abort in scheduler reply: %s",
                    boincerror(retval)
                );
            } else {
                result_abort.push_back(result);
            }
        } else if (match_tag(buf, "<result_abort_if_not_started>")) {
            RESULT result;
            retval = result.parse_name(in, "</result_abort_if_not_started>");
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't parse result abort-if-not-started in scheduler reply: %s",
                    boincerror(retval)
                );
            } else {
                result_abort_if_not_started.push_back(result);
            }
        } else if (parse_str(buf, "<delete_file_info>", delete_file_name)) {
            file_deletes.push_back(delete_file_name);
        } else if (parse_str(buf, "<message", msg_buf, sizeof(msg_buf))) {
            parse_attr(buf, "priority", pri_buf, sizeof(pri_buf));
            USER_MESSAGE um(msg_buf, pri_buf);
            messages.push_back(um);
            continue;
        } else if (match_tag(buf, "<message_ack/>")) {
            message_ack = true;
        } else if (match_tag(buf, "<project_is_down/>")) {
            project_is_down = true;
        } else if (parse_str(buf, "<email_hash>", project->email_hash, sizeof(project->email_hash))) {
            continue;
        } else if (parse_str(buf, "<cross_project_id>", project->cross_project_id, sizeof(project->cross_project_id))) {
            continue;
        } else if (match_tag(buf, "<trickle_down>")) {
            retval = gstate.handle_trickle_down(project, in);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "handle_trickle_down failed: %s", boincerror(retval)
                );
            }
            continue;
        } else if (parse_bool(buf, "non_cpu_intensive", project->non_cpu_intensive)) {
            continue;
        } else if (parse_bool(buf, "ended", project->ended)) {
            continue;
        } else if (match_tag(buf, "<request_file_list/>")) {
            send_file_list = true;
        } else if (parse_int(buf, "<send_time_stats_log>", send_time_stats_log)){
            continue;
        } else if (parse_int(buf, "<send_job_log>", send_job_log)) {
            continue;
        } else if (parse_int(buf, "<scheduler_version>", scheduler_version)) {
            continue;
        } else if (match_tag(buf, "<project_files>")) {
            retval = project->parse_project_files(mf, true);
        } else if (match_tag(buf, "<!--")) {
            continue;
        } else {
            handle_unparsed_xml_warning("SCHEDULER_REPLY::parse", buf);
        }
    }
    if (found_start_tag) {
        msg_printf(project, MSG_INTERNAL_ERROR, "No close tag in scheduler reply");
    } else {
        msg_printf(project, MSG_INTERNAL_ERROR, "No start tag in scheduler reply");
    }

    return ERR_XML_PARSE;
}
