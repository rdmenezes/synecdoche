// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 University of California
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

#include "client_state.h"

#include <cstring>
#include <errno.h>

#include <fstream>
#include <ostream>

#include "miofile.h"
#include "mfile.h"
#include "parse.h"
#include "str_util.h"
#include "util.h"
#include "error_numbers.h"
#include "filesys.h"
#include "file_names.h"
#include "client_msgs.h"
#include "pers_file_xfer.h"
#include "version.h"
#include "xml_write.h"

#define MAX_STATE_FILE_WRITE_ATTEMPTS 2

void CLIENT_STATE::set_client_state_dirty(const char* source) {
    if (log_flags.statefile_debug) {
        msg_printf(0, MSG_INFO, "[statefile_debug] set dirty: %s\n", source);
    }
    client_state_dirty = true;
}

static bool valid_state_file(const char* fname) {
    char buf[256];
    FILE* f = boinc_fopen(fname, "r");
    if (!f) return false;
    if (!fgets(buf, 256, f)) {
        fclose(f);
        return false;
    }
    if (!match_tag(buf, "<client_state>")) {
        fclose(f);
        return false;
    }
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</client_state>")) {
            fclose(f);
            return true;
        }
    }
    fclose(f);
    return false;
}

/// Parse the client_state.xml file.
int CLIENT_STATE::parse_state_file() {
    PROJECT *project=NULL;
    char buf[256];
    int retval=0;
    int failnum;
    const char *fname;

    // Look for a valid state file:
    // First the regular one, then the "next" one.
    //
    if (valid_state_file(STATE_FILE_NEXT)) {
        fname = STATE_FILE_NEXT;
    } else if (valid_state_file(STATE_FILE_NAME)) {
        fname = STATE_FILE_NAME;
    } else if (valid_state_file(STATE_FILE_PREV)) {
        fname = STATE_FILE_PREV;
    } else {
        if (log_flags.statefile_debug) {
            msg_printf(0, MSG_INFO,
                "[statefile_debug] CLIENT_STATE::parse_state_file(): No state file; will create one"
            );
        }

        // avoid warning messages about version
        //
        old_major_version = SYNEC_MAJOR_VERSION;
        old_minor_version = SYNEC_MINOR_VERSION;
        old_release = SYNEC_RELEASE;
        return ERR_FOPEN;
    }

    FILE* f = fopen(fname, "r");
    MIOFILE mf;
    mf.init_file(f);
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</client_state>")) {
            break;
        }
        if (match_tag(buf, "<client_state>")) {
            continue;
        }
        if (match_tag(buf, "<project>")) {
            PROJECT temp_project;
            retval = temp_project.parse_state(mf);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR, "Can't parse project in state file");
            } else {
                project = lookup_project(temp_project.get_master_url());
                if (project) {
                    project->copy_state_fields(temp_project);
                } else {
                    msg_printf(&temp_project, MSG_INTERNAL_ERROR,
                        "Project %s is in state file but no account file found",
                        temp_project.get_project_name()
                    );
                }
            }
            continue;
        }
        if (match_tag(buf, "<app>")) {
            APP* app = new APP;
            retval = app->parse(mf);
            if (!project) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Application %s outside project in state file",
                    app->name
                );
                delete app;
                continue;
            }
            if (project->anonymous_platform) {
                delete app;
                continue;
            }
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse application in state file"
                );
                delete app;
                continue;
            }
            retval = link_app(project, app);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't handle application %s in state file",
                    app->name
                );
                delete app;
                continue;
            }
            apps.push_back(app);
            continue;
        }
        if (match_tag(buf, "<file_info>")) {
            FILE_INFO* fip = new FILE_INFO;
            retval = fip->parse(mf, false);
            if (!project) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "File info outside project in state file"
                );
                delete fip;
                continue;
            }
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't handle file info in state file"
                );
                delete fip;
                continue;
            }
            retval = link_file_info(project, fip);
            if (project->anonymous_platform && retval == ERR_NOT_UNIQUE) {
                delete fip;
                continue;
            }
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                        "Can't handle file info %s in state file", fip->name.c_str());
                delete fip;
                continue;
            }
            file_infos.push_back(fip);
            // If the file had a failure before,
            // don't start another file transfer
            if (fip->had_failure(failnum)) {
                if (fip->pers_file_xfer) {
                    delete fip->pers_file_xfer;
                    fip->pers_file_xfer = NULL;
                }
            }
            if (fip->pers_file_xfer) {
                retval = fip->pers_file_xfer->init(fip, fip->upload_when_present);
                if (retval) {
                    msg_printf(project, MSG_INTERNAL_ERROR,
                            "Can't initialize file transfer for %s", fip->name.c_str());
                }
                retval = pers_file_xfers->insert(fip->pers_file_xfer);
                if (retval) {
                    msg_printf(project, MSG_INTERNAL_ERROR,
                            "Can't start persistent file transfer for %s", fip->name.c_str());
                }
            }
            continue;
        }
        if (match_tag(buf, "<app_version>")) {
            APP_VERSION* avp = new APP_VERSION;
            retval = avp->parse(mf);
            if (!project) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                        "Application version outside project in state file");
                delete avp;
                continue;
            }
            if (project->anonymous_platform) {
                delete avp;
                continue;
            }
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse application version in state file"
                );
                delete avp;
                continue;
            }
            if (strlen(avp->platform) == 0) {
                strlcpy(avp->platform, get_primary_platform().c_str(), sizeof(avp->platform));
            } else {
                if (!is_supported_platform(avp->platform)) {
                    // if it's a platform we haven't heard of,
                    // must be that the user tried out a 64 bit client
                    // and then reverted to a 32-bit client.
                    // Let's not throw away the app version and its WUs
                    //
                    msg_printf(project, MSG_INTERNAL_ERROR,
                        "App version has unsupported platform %s; changing to %s",
                        avp->platform, get_primary_platform().c_str()
                    );
                    strlcpy(avp->platform, get_primary_platform().c_str(), sizeof(avp->platform));
                }
            }
            retval = link_app_version(project, avp);
            if (retval) {
                delete avp;
                continue;
            }
            app_versions.push_back(avp);
            continue;
        }
        if (match_tag(buf, "<workunit>")) {
            WORKUNIT* wup = new WORKUNIT;
            retval = wup->parse(mf);
            if (!project) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Workunit outside project in state file"
                );
                delete wup;
                continue;
            }
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse workunit in state file"
                );
                delete wup;
                continue;
            }
            retval = link_workunit(project, wup);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't handle workunit in state file"
                );
                delete wup;
                continue;
            }
            workunits.push_back(wup);
            continue;
        }
        if (match_tag(buf, "<result>")) {
            RESULT* rp = new RESULT;
            retval = rp->parse_state(mf);
            if (!project) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Task %s outside project in state file",
                    rp->name
                );
                delete rp;
                continue;
            }
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse task in state file"
                );
                delete rp;
                continue;
            }
            retval = link_result(project, rp);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't link task %s in state file",
                    rp->name
                );
                delete rp;
                continue;
            }
            if (!strlen(rp->platform) || !is_supported_platform(rp->platform)) {
                strlcpy(rp->platform, get_primary_platform().c_str(), sizeof(rp->platform));
                rp->version_num = latest_version(rp->wup->app, rp->platform);
            }
            rp->avp = lookup_app_version(
                rp->wup->app, rp->platform, rp->version_num, rp->plan_class
            );
            if (!rp->avp) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "No app version for result: %s %d %s",
                    rp->platform, rp->version_num, rp->plan_class
                );
                delete rp;
                continue;
            }
            rp->wup->version_num = rp->version_num;
            results.push_back(rp);
            continue;
        }
        if (match_tag(buf, "<project_files>")) {
            if (!project) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Project files outside project in state file"
                );
                skip_unrecognized(buf, mf);
                continue;
            }
            project->parse_project_files(mf, false);
            project->link_project_files(false);
            continue;
        }
        if (match_tag(buf, "<host_info>")) {
            retval = host_info.parse(mf);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse host info in state file"
                );
            }
            continue;
        }
        if (match_tag(buf, "<time_stats>")) {
            retval = time_stats.parse(mf);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse time stats in state file"
                );
            }
            continue;
        }
        if (match_tag(buf, "<net_stats>")) {
            retval = net_stats.parse(mf);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse network stats in state file"
                );
            }
            continue;
        }
        if (match_tag(buf, "<active_task_set>")) {
            retval = active_tasks.parse(mf);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse active tasks in state file"
                );
            }
            continue;
        }
        if (parse_str(buf, "<platform_name>", statefile_platform_name)) {
            continue;
        }
        if (match_tag(buf, "<alt_platform>")) {
            continue;
        }
        if (parse_int(buf, "<user_run_request>", retval)) {
            run_mode.set(retval, 0);
            continue;
        }
        if (parse_int(buf, "<user_network_request>", retval)) {
            network_mode.set(retval, 0);
            continue;
        }
        if (parse_int(buf, "<core_client_major_version>", old_major_version)) {
            continue;
        }
        if (parse_int(buf, "<core_client_minor_version>", old_minor_version)) {
            continue;
        }
        if (parse_int(buf, "<core_client_release>", old_release)) {
            continue;
        }
        if (match_tag(buf, "<cpu_benchmarks_pending/>")) {
            run_cpu_benchmarks = true;
            continue;
        }
        if (match_tag(buf, "<work_fetch_no_new_work/>")) {
            work_fetch_no_new_work = true;
            continue;
        }
        if (match_tag(buf, "<proxy_info>")) {
            retval = proxy_info.parse(mf);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse proxy info in state file"
                );
            }
            continue;
        }
        if (parse_str(buf, "<host_venue>", main_host_venue, sizeof(main_host_venue))) {
            continue;
        }
#ifdef ENABLE_UPDATE_CHECK
        if (parse_double(buf, "<new_version_check_time>", new_version_check_time)) {
            continue;
        }
        if (parse_str(buf, "<newer_version>", newer_version)) {
            continue;
        }
#endif
        handle_unparsed_xml_warning("CLIENT_STATE::parse_state_file", buf);
        skip_unrecognized(buf, mf);
    }
    fclose(f);
    return 0;
}


/// Write the client_state.xml file.
int CLIENT_STATE::write_state_file() const {
    int retval, attempt;
#ifdef _WIN32
    char win_error_msg[4096];
#endif

    for (attempt=1; attempt<=MAX_STATE_FILE_WRITE_ATTEMPTS; attempt++) {
        if (attempt > 1) boinc_sleep(1.0);

        if (log_flags.statefile_debug) {
            msg_printf(0, MSG_INFO,
                "[statefile_debug] CLIENT_STATE::write_state_file(): Writing state file"
            );
        }

        {
            std::ofstream file(STATE_FILE_NEXT, std::ios::out);
            if (!file.is_open()) {
                if ((attempt == MAX_STATE_FILE_WRITE_ATTEMPTS) || log_flags.statefile_debug) {
                    msg_printf(0, MSG_INTERNAL_ERROR,
                        "Can't open %s; errno: %s",
                        STATE_FILE_NEXT, strerror(errno)
                    );
                }
                if (attempt < MAX_STATE_FILE_WRITE_ATTEMPTS) continue;
                return ERR_FOPEN;
            }
            file.exceptions(std::ios::badbit | std::ios::failbit);
            try {
                write_state(file);
                file.close();
            } catch (std::ios::failure& e) {
                if ((attempt == MAX_STATE_FILE_WRITE_ATTEMPTS) || log_flags.statefile_debug) {
                    msg_printf(NULL, MSG_INTERNAL_ERROR,
                        "Couldn't write state file: %s", strerror(errno)
                    );
                }
                if (attempt < MAX_STATE_FILE_WRITE_ATTEMPTS) continue;
                return ERR_FWRITE; //somewhat appropriate...
            }
        }

        // only attempt to rename the current state file if it exists.
        //
        if (boinc_file_exists(STATE_FILE_NAME)) {
            if (boinc_file_exists(STATE_FILE_PREV)) {
                retval = boinc_delete_file(STATE_FILE_PREV);
                if (retval) {
                    if ((attempt == MAX_STATE_FILE_WRITE_ATTEMPTS) || log_flags.statefile_debug) {
#ifdef _WIN32
                        msg_printf(0, MSG_USER_ERROR,
                            "Can't delete previous state file; %s",
                            windows_error_string(win_error_msg, sizeof(win_error_msg))
                        );
#else
                        msg_printf(0, MSG_USER_ERROR,
                            "Can't delete previous state file; error %d: %s",
                            errno, strerror(errno)
                        );
#endif
                    }
                    if (attempt < MAX_STATE_FILE_WRITE_ATTEMPTS) continue;
                }
            }

            retval = boinc_rename(STATE_FILE_NAME, STATE_FILE_PREV);
            if (retval) {
                if ((attempt == MAX_STATE_FILE_WRITE_ATTEMPTS) || log_flags.statefile_debug) {
#ifdef _WIN32
                    msg_printf(0, MSG_USER_ERROR,
                        "Can't rename current state file to previous state file; %s",
                        windows_error_string(win_error_msg, sizeof(win_error_msg))
                    );
#else
                    msg_printf(0, MSG_USER_ERROR,
                        "rename current state file to previous state file returned error %d: %s",
                        errno, strerror(errno)
                    );
#endif
                }
                if (attempt < MAX_STATE_FILE_WRITE_ATTEMPTS) continue;
            }
        }

        retval = boinc_rename(STATE_FILE_NEXT, STATE_FILE_NAME);
        if (log_flags.statefile_debug) {
            msg_printf(0, MSG_INFO,
                "[statefile_debug] CLIENT_STATE::write_state_file(): Done writing state file"
            );
        }
        if (!retval) break;     // Success!

        if ((attempt == MAX_STATE_FILE_WRITE_ATTEMPTS) || log_flags.statefile_debug) {
#ifdef _WIN32
            if (retval == ERROR_ACCESS_DENIED) {
                msg_printf(0, MSG_USER_ERROR,
                    "Can't rename state file; access denied; check file and directory permissions"
                );
            } else {
                msg_printf(0, MSG_USER_ERROR,
                    "Can't rename state file; %s",
                    windows_error_string(win_error_msg, sizeof(win_error_msg))
                );
            }
#elif defined (__APPLE__)
            msg_printf(0, MSG_USER_ERROR,
                "Can't rename %s to %s; check file and directory permissions\n"
                "rename returned error %d: %s",
                STATE_FILE_NEXT, STATE_FILE_NAME, errno, strerror(errno)
            );
            if (log_flags.statefile_debug) {
                system("ls -al /Library/Application\\ Support/Synecdoche\\ Data/client*.*");
            }
#else
        msg_printf(0, MSG_USER_ERROR,
            "Can't rename %s to %s; check file and directory permissions",
            STATE_FILE_NEXT, STATE_FILE_NAME
        );
#endif
        }
        if (attempt < MAX_STATE_FILE_WRITE_ATTEMPTS) continue;
        return ERR_RENAME;
    }
    return 0;
}

void CLIENT_STATE::write_state(std::ostream& out) const {
    out << "<client_state>\n";

    host_info.write(out, false);
    time_stats.write(out, false);
    net_stats.write(out);
    for (size_t pn=0; pn<projects.size(); pn++) {
        const PROJECT* p = projects[pn];
        size_t i;
        p->write_state(out);
        for (i=0; i<apps.size(); i++) {
            if (apps[i]->project == p) {
                apps[i]->write(out);
            }
        }
        for (i=0; i<file_infos.size(); i++) {
            if (file_infos[i]->project == p) {
                file_infos[i]->write(out, false);
            }
        }
        for (i=0; i<app_versions.size(); i++) {
            if (app_versions[i]->project == p) {
                app_versions[i]->write(out);
            }
        }
        for (i=0; i<workunits.size(); i++) {
            if (workunits[i]->project == p) workunits[i]->write(out);
        }
        for (i=0; i<results.size(); i++) {
            if (results[i]->project == p) results[i]->write(out, false);
        }
        p->write_project_files(out);
    }
    active_tasks.write(out);
    out << XmlTag<std::string>("platform_name", get_primary_platform())
        << XmlTag<int>("core_client_major_version", core_client_version.major)
        << XmlTag<int>("core_client_minor_version", core_client_version.minor)
        << XmlTag<int>("core_client_release", core_client_version.release)
        << XmlTag<int>("user_run_request", run_mode.get_perm())
        << XmlTag<int>("user_network_request", network_mode.get_perm())
    ;
    if (cpu_benchmarks_pending) out << "<cpu_benchmarks_pending/>\n";

#ifdef ENABLE_UPDATE_CHECK
    out << XmlTag<double>("new_version_check_time", new_version_check_time);
    if (!newer_version.empty()) {
        out << XmlTag<std::string>("newer_version", newer_version);
    }
#endif
    for (size_t i=1; i<platforms.size(); i++) {
        out << XmlTag<std::string>("alt_platform", platforms[i].name);
    }
    proxy_info.write(out);
    if (strlen(main_host_venue)) {
        out << XmlTag<const char*>("host_venue", main_host_venue);
    }
    out << "</client_state>\n";
}

/// Write the client_state.xml file if necessary.
/// \todo Write no more often than X seconds.
int CLIENT_STATE::write_state_file_if_needed() {
    int retval;
    if (client_state_dirty) {
        client_state_dirty = false;
        retval = write_state_file();
        if (retval) return retval;
    }
    return 0;
}

/// Look for app_versions.xml file in the project directory.
/// If it's found get app versions from there,
/// and use "anonymous platform" mechanism for this project.
void CLIENT_STATE::check_anonymous() {
    unsigned int i;
    FILE* f;
    int retval;

    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        std::string path(get_project_dir(p));
        path.append("/" APP_INFO_FILE_NAME);
        f = fopen(path.c_str(), "r");
        if (!f) continue;
        msg_printf(p, MSG_INFO,
            "Found %s; using anonymous platform", APP_INFO_FILE_NAME
        );

        // flag as anonymous even if can't parse file
        p->anonymous_platform = true;

        retval = parse_app_info(p, f);
        if (retval) {
            msg_printf(p, MSG_USER_ERROR,
                "Parse error in %s; check XML syntax", APP_INFO_FILE_NAME
            );
        }
        fclose(f);
    }
}

int CLIENT_STATE::parse_app_info(PROJECT* p, FILE* in) {
    char buf[256];
    MIOFILE mf;
    mf.init_file(in);

    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "<app_info>")) continue;
        if (match_tag(buf, "</app_info>")) return 0;
        if (match_tag(buf, "<file_info>")) {
            FILE_INFO* fip = new FILE_INFO;
            if (fip->parse(mf, false)) {
                delete fip;
                continue;
            }
            if (link_file_info(p, fip)) {
                delete fip;
                continue;
            }
            fip->status = FILE_PRESENT;
            file_infos.push_back(fip);
            continue;
        }
        if (match_tag(buf, "<app>")) {
            APP* app = new APP;
            if (app->parse(mf)) {
                delete app;
                continue;
            }
            if (lookup_app(p, app->name)) {
                delete app;
                continue;
            }
            link_app(p, app);
            apps.push_back(app);
            continue;
        }
        if (match_tag(buf, "<app_version>")) {
            APP_VERSION* avp = new APP_VERSION;
            if (avp->parse(mf)) {
                delete avp;
                continue;
            }
            if (strlen(avp->platform) == 0) {
                strlcpy(avp->platform, get_primary_platform().c_str(), sizeof(avp->platform));
            }
            if (link_app_version(p, avp)) {
                delete avp;
                continue;
            }
            app_versions.push_back(avp);
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(p, MSG_INFO,
                "[unparsed_xml] Unparsed line in %s: %s",
                APP_INFO_FILE_NAME,
                buf
            );
        }
    }
    return ERR_XML_PARSE;
}

void CLIENT_STATE::write_state_gui(std::ostream& out) const {
    unsigned int i, j;

    out << "<client_state>\n";

    host_info.write(out, false);
    time_stats.write(out, false);
    net_stats.write(out);

    for (j=0; j<projects.size(); j++) {
        const PROJECT* p = projects[j];
        p->write_state(out, true);
        for (i=0; i<apps.size(); i++) {
            if (apps[i]->project == p) {
                apps[i]->write(out);
            }
        }
        for (i=0; i<app_versions.size(); i++) {
            if (app_versions[i]->project == p) app_versions[i]->write(out);
        }
        for (i=0; i<workunits.size(); i++) {
            if (workunits[i]->project == p) workunits[i]->write(out);
        }
        for (i=0; i<results.size(); i++) {
            if (results[i]->project == p) results[i]->write_gui(out);
        }
    }
    out << XmlTag<std::string>("platform_name",     get_primary_platform())
        << XmlTag<int>("core_client_major_version", core_client_version.major)
        << XmlTag<int>("core_client_minor_version", core_client_version.minor)
        << XmlTag<int>("core_client_release",       core_client_version.release)
    ;
    if (executing_as_daemon) {
        out << "<executing_as_daemon/>\n";
    }
    if (work_fetch_no_new_work) {
        out << "<work_fetch_no_new_work/>\n";
    }

    global_prefs.write(out);

    if (strlen(main_host_venue)) {
        out << XmlTag<const char*>("host_venue", main_host_venue);
    }

    out << "</client_state>\n";
}

void CLIENT_STATE::write_tasks_gui(std::ostream& out) const {
    for (size_t i=0; i<results.size(); i++) {
        const RESULT* rp = results[i];
        rp->write_gui(out);
    }
}

void CLIENT_STATE::write_file_transfers_gui(std::ostream& out) const {
    out << "<file_transfers>\n";

    for (size_t i=0; i<file_infos.size(); i++) {
        const FILE_INFO* fip = file_infos[i];
        if (fip->pers_file_xfer
           || (fip->upload_when_present && fip->status == FILE_PRESENT && !fip->uploaded)
        ) {
            fip->write_gui(out);
        }
    }
    out << "</file_transfers>\n";
}
