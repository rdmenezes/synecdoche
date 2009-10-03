// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
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

/// \file
/// Abstraction of a set of executing applications,
/// connected to I/O files in various ways.
///
/// Shouldn't depend on CLIENT_STATE.

#include <sstream>

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#ifndef _WIN32

#include <unistd.h>
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#endif

#include "app.h"

#include <cctype>
#include <ctime>
#include <cstdio>
#include <cmath>
#include <cstdlib>

#include <ostream>

#include "client_state.h"
#include "client_types.h"

#include "error_numbers.h"
#include "filesys.h"
#include "file_names.h"
#include "parse.h"
#include "miofile.h"
#include "shmem.h"
#include "str_util.h"
#include "client_msgs.h"
#include "procinfo.h"
#include "sandbox.h"
#include "xml_write.h"

/// If we send app <abort> request, wait this long before killing it.
/// This gives it time to download symbol files (which can be several MB)
/// and write stack trace to stderr
#define ABORT_TIMEOUT   60
/// If we send app <quit> request, wait this long before killing it.
/// Shorter than ABORT_TIMEOUT because no stack trace is generated.
#define QUIT_TIMEOUT    10

ACTIVE_TASK::~ACTIVE_TASK() {
}

ACTIVE_TASK::ACTIVE_TASK() {
    result = NULL;
    wup = NULL;
    app_version = NULL;
    pid = 0;
    slot = 0;
    full_init_done = false;
    _task_state = PROCESS_UNINITIALIZED;
    scheduler_state = CPU_SCHED_UNINITIALIZED;
    signal = 0;
    graphics_mode_acked = MODE_UNSUPPORTED;
    graphics_mode_ack_timeout = 0;
    fraction_done = 0;
    episode_start_cpu_time = 0;
    run_interval_start_wall_time = gstate.now;
    debt_interval_start_cpu_time = 0;
    checkpoint_cpu_time = 0;
    checkpoint_wall_time = 0;
    current_cpu_time = 0;
    have_trickle_down = false;
    send_upload_file_status = false;
    too_large = false;
    needs_shmem = false;
    want_network = 0;
    premature_exit_count = 0;
    quit_time = 0;
    memset(&procinfo, 0, sizeof(procinfo));
#ifdef _WIN32
    pid_handle = 0;
    shm_handle = 0;
#endif
    premature_exit_count = 0;

    stats_mem = 0;
    stats_page = 0;
    stats_pagefault_rate = 0;
    stats_disk = 0;
    stats_checkpoint = 0;
}

static const char* task_state_name(TASK_STATE val) {
    switch (val) {
    case PROCESS_UNINITIALIZED: return "UNINITIALIZED";
    case PROCESS_EXECUTING: return "EXECUTING";
    case PROCESS_SUSPENDED: return "SUSPENDED";
    case PROCESS_ABORT_PENDING: return "ABORT_PENDING";
    case PROCESS_EXITED: return "EXITED";
    case PROCESS_WAS_SIGNALED: return "WAS_SIGNALED";
    case PROCESS_EXIT_UNKNOWN: return "EXIT_UNKNOWN";
    case PROCESS_ABORTED: return "ABORTED";
    case PROCESS_COULDNT_START: return "COULDNT_START";
    case PROCESS_QUIT_PENDING: return "QUIT_PENDING";
    }
    return "Unknown";
}

void ACTIVE_TASK::set_task_state(TASK_STATE val, const char* where) {
    _task_state = val;
    if (log_flags.task_debug) {
        msg_printf(result->project, MSG_INFO,
            "[task_debug] task_state=%s for %s from %s",
            task_state_name(val), result->name, where
        );
    }
}

/// Called when a process has exited or we've killed it.
void ACTIVE_TASK::cleanup_task() {
#ifdef _WIN32
    if (pid_handle) {
        CloseHandle(pid_handle);
        pid_handle = NULL;
    }

    // detach from shared mem.
    // This will destroy shmem seg since we're the last attachment
    if (app_client_shm.shm) {
        detach_shmem(shm_handle, app_client_shm.shm);
        app_client_shm.shm = NULL;
    }
#else
    int retval;

    if (app_client_shm.shm) {
        if (app_version->api_major_version() >= 6) {
            retval = detach_shmem_mmap(app_client_shm.shm, sizeof(SHARED_MEM));
        } else {
            retval = detach_shmem(app_client_shm.shm);
            if (retval) {
                msg_printf(wup->project, MSG_INTERNAL_ERROR,
                    "Couldn't detach shared memory: %s", boincerror(retval)
                );
            }
            retval = destroy_shmem(shmem_seg_name);
            if (retval) {
                msg_printf(wup->project, MSG_INTERNAL_ERROR,
                    "Couldn't destroy shared memory: %s", boincerror(retval)
                );
            }
        }
        app_client_shm.shm = NULL;
        gstate.retry_shmem_time = 0;
    }
#endif

    if (gstate.exit_after_finish) {
        exit(0);
    }
}

int ACTIVE_TASK::init(RESULT* rp) {
    result = rp;
    wup = rp->wup;
    app_version = rp->avp;
    max_cpu_time = rp->wup->rsc_fpops_bound/gstate.host_info.p_fpops;
    max_disk_usage = rp->wup->rsc_disk_bound;
    max_mem_usage = rp->wup->rsc_memory_bound;
    slot_dir = ::get_slot_dir(slot);
    slot_path = relative_to_absolute(slot_dir);
    return 0;
}

#if 0
// Deallocate memory to prevent unneeded reporting of memory leaks
//
void ACTIVE_TASK_SET::free_mem() {
    vector<ACTIVE_TASK*>::iterator at_iter;
    ACTIVE_TASK *at;

    at_iter = active_tasks.begin();
    while (at_iter != active_tasks.end()) {
        at = active_tasks[0];
        at_iter = active_tasks.erase(at_iter);
        delete at;
    }
}
#endif

void ACTIVE_TASK_SET::get_memory_usage() {
    static double last_mem_time=0;
    unsigned int i;
    int retval;

    double diff = gstate.now - last_mem_time;
    if (diff < 10) return;

    last_mem_time = gstate.now;
    std::vector<PROCINFO> piv;
    retval = procinfo_setup(piv);
    if (retval) {
        if (log_flags.mem_usage_debug) {
            msg_printf(0, MSG_INTERNAL_ERROR,
                "[mem_usage_debug] procinfo_setup() returned %d", retval
            );
        }
        return;
    }
    for (i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (atp->scheduler_state == CPU_SCHED_SCHEDULED) {
            PROCINFO& pi = atp->procinfo;
            unsigned long last_page_fault_count = pi.page_fault_count;
            memset(&pi, 0, sizeof(pi));
            pi.id = atp->pid;
            procinfo_app(pi, piv);
            pi.working_set_size_smoothed = .5*pi.working_set_size_smoothed + pi.working_set_size;

            int pf = pi.page_fault_count - last_page_fault_count;
            pi.page_fault_rate = pf/diff;
            if (log_flags.mem_usage_debug) {
                msg_printf(atp->result->project, MSG_INFO,
                    "[mem_usage_debug] %s: RAM %.2fMB, smoothed %.2fMB, page %.2fMB, %.2f page faults/sec, user CPU %.3f, kernel CPU %.3f",
                    atp->result->name,
                    pi.working_set_size/MEGA,
                    pi.working_set_size_smoothed/MEGA,
                    pi.swap_size/MEGA,
                    pi.page_fault_rate,
                    pi.user_time, pi.kernel_time
                );
            }
            atp->stats_mem = std::max(atp->stats_mem, pi.working_set_size);
            atp->stats_page = std::max(atp->stats_page, pi.swap_size);
            atp->stats_pagefault_rate = std::max(atp->stats_pagefault_rate, pi.page_fault_rate);
        }
    }

#if 0
    // the following is not useful because most OSs don't
    // move idle processes out of RAM, so physical memory is always full
    //
    procinfo_other(pi, piv);
    msg_printf(NULL, MSG_INFO, "All others: RAM %.2fMB, page %.2fMB, user %.3f, kernel %.3f",
        pi.working_set_size/MEGA, pi.swap_size/MEGA,
        pi.user_time, pi.kernel_time
    );
#endif
}

/// Do periodic checks on running apps:
/// - get latest CPU time and % done info
/// - check if any has exited, and clean up
/// - see if any has exceeded its CPU or disk space limits, and abort it
bool ACTIVE_TASK_SET::poll() {
    bool action;
    unsigned int i;
    static double last_time = 0;
    if (gstate.now - last_time < 1.0) return false;
    last_time = gstate.now;

    action = check_app_exited();
    send_heartbeats();
    send_trickle_downs();
    graphics_poll();
    process_control_poll();
    get_memory_usage();
    action |= check_rsc_limits_exceeded();
    get_msgs();
    for (i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (atp->task_state() == PROCESS_ABORT_PENDING) {
            if (gstate.now > atp->abort_time + ABORT_TIMEOUT) {
                atp->kill_task(false);
            }
        }
        if (atp->task_state() == PROCESS_QUIT_PENDING) {
            if (gstate.now > atp->quit_time + QUIT_TIMEOUT) {
                atp->kill_task(true);
            }
        }
    }

    if (action) {
        gstate.set_client_state_dirty("ACTIVE_TASK_SET::poll");
    }

    return action;
}

/// Move a trickle file from the slot directory to the project directory.
/// If moving the file files it will be deleted.
///
/// \return Zero on success, #ERR_RENAME on error.
int ACTIVE_TASK::move_trickle_file() {
    std::string project_dir = get_project_dir(result->project);

    std::string old_path(slot_dir);
    old_path.append("/trickle_up.xml");
    std::ostringstream new_path;
    new_path << project_dir << "/trickle_up_" << result->name << '_' << time(0) << ".xml";

    int retval = boinc_rename(old_path.c_str(), new_path.str().c_str());

    // If can't move it, remove it.
    if (retval) {
        delete_project_owned_file(old_path.c_str(), true);
        return ERR_RENAME;
    }
    return 0;
}

/// Disk used by output files and temp files of this task.
int ACTIVE_TASK::current_disk_usage(double& size) const {
    double x;
    int retval;
    std::string path;

    retval = dir_size(slot_dir.c_str(), size);
    if (retval) return retval;
    for (size_t i=0; i<result->output_files.size(); i++) {
        const FILE_INFO* fip = result->output_files[i].file_info;

        path = get_pathname(fip);
        retval = file_size(path.c_str(), x);
        if (!retval) size += x;
    }
    return 0;
}

bool ACTIVE_TASK_SET::is_slot_in_use(int slot) const {
    unsigned int i;
    for (i=0; i<active_tasks.size(); i++) {
        if (active_tasks[i]->slot == slot) {
            return true;
        }
    }
    return false;
}

bool ACTIVE_TASK_SET::is_slot_dir_in_use(const std::string& dir) const {
    for (size_t i = 0; i < active_tasks.size(); ++i) {
        if (get_slot_dir(active_tasks[i]->slot) == dir) {
            return true;
        }
    }
    return false;
}

/// Get a free slot,
/// and make a slot dir if needed
int ACTIVE_TASK_SET::get_free_slot() const {
    int j, retval;

    for (j=0; ; j++) {
        if (is_slot_in_use(j)) continue;

        // make sure we can make an empty directory for this slot
        std::string slot_dir = get_slot_dir(j);
        if (boinc_file_exists(slot_dir.c_str())) {
            if (is_dir(slot_dir.c_str())) {
                retval = client_clean_out_dir(slot_dir.c_str());
                if (!retval) return j;
            }
        } else {
            retval = make_slot_dir(j);
            if (!retval) return j;
        }
    }
}

bool ACTIVE_TASK_SET::slot_taken(int slot) const {
    unsigned int i;
    for (i=0; i<active_tasks.size(); i++) {
        if (active_tasks[i]->slot == slot) return true;
    }
    return false;
}

// <active_task_state> is here for the benefit of 3rd-party software
// that reads the client state file
//
int ACTIVE_TASK::write(std::ostream& out) const {
    out << "<active_task>\n"
        << XmlTag("project_master_url", result->project->get_master_url())
        << XmlTag("result_name",        result->name)
        << XmlTag("active_task_state",  task_state())
        << XmlTag("app_version_num",    app_version->version_num)
        << XmlTag("slot", slot);
    if (full_init_done) {
        out << "<full_init_done/>\n";
    }
    out << XmlTag("checkpoint_cpu_time",        checkpoint_cpu_time)
        << XmlTag("fraction_done",              fraction_done)
        << XmlTag("current_cpu_time",           current_cpu_time)
        << XmlTag("swap_size",                  procinfo.swap_size)
        << XmlTag("working_set_size",           procinfo.working_set_size)
        << XmlTag("working_set_size_smoothed",  procinfo.working_set_size_smoothed)
        << XmlTag("page_fault_rate",            procinfo.page_fault_rate)
        << XmlTag("stats_mem",                  stats_mem)
        << XmlTag("stats_page",                 stats_page)
        << XmlTag("stats_pagefault_rate",       stats_pagefault_rate)
        << XmlTag("stats_disk",                 stats_disk)
        << XmlTag("stats_checkpoint",           stats_checkpoint)
    ;
    out << "</active_task>\n";
    return 0;
}

int ACTIVE_TASK::write_gui(std::ostream& out) const {
    out << "<active_task>\n"
        << XmlTag("active_task_state",          task_state())
        << XmlTag("app_version_num",            app_version->version_num)
        << XmlTag("slot",                       slot)
        << XmlTag("scheduler_state",            scheduler_state)
        << XmlTag("checkpoint_cpu_time",        checkpoint_cpu_time)
        << XmlTag("fraction_done",              fraction_done)
        << XmlTag("current_cpu_time",           current_cpu_time)
        << XmlTag("swap_size",                  procinfo.swap_size)
        << XmlTag("working_set_size",           procinfo.working_set_size)
        << XmlTag("working_set_size_smoothed",  procinfo.working_set_size_smoothed)
        << XmlTag("page_fault_rate",            procinfo.page_fault_rate)
    ;
    if (too_large) {
        out << "   <too_large/>\n";
    }
    if (needs_shmem) {
        out << "   <needs_shmem/>\n";
    }
    if (strlen(app_version->graphics_exec_path)) {
        out << XmlTag("graphics_exec_path", app_version->graphics_exec_path);
        out << XmlTag("slot_path", slot_path);
    }
    if (supports_graphics() && !gstate.disable_graphics) {
        out << "   <supports_graphics/>\n";
        out << XmlTag("graphics_mode_acked", graphics_mode_acked);
    }
    out << "</active_task>\n";
    return 0;
}

int ACTIVE_TASK::parse(MIOFILE& fin) {
    char buf[256], result_name[256], project_master_url[256];
    int n, dummy;
    unsigned int i;
    PROJECT* project;

    strcpy(result_name, "");
    strcpy(project_master_url, "");

    while (fin.fgets(buf, 256)) {
        if (match_tag(buf, "</active_task>")) {
            project = gstate.lookup_project(project_master_url);
            if (!project) {
                msg_printf(
                    NULL, MSG_INTERNAL_ERROR,
                    "State file error: project %s not found\n",
                    project_master_url
                );
                return ERR_NULL;
            }
            result = gstate.lookup_result(project, result_name);
            if (!result) {
                msg_printf(
                    project, MSG_INTERNAL_ERROR,
                    "State file error: result %s not found\n",
                    result_name
                );
                return ERR_NULL;
            }

            // various sanity checks
            //
            if (result->got_server_ack
                || result->ready_to_report
                || result->state() != RESULT_FILES_DOWNLOADED
            ) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "State file error: result %s is in wrong state\n",
                    result_name
                );
                return ERR_BAD_RESULT_STATE;
            }

            wup = result->wup;
            app_version = gstate.lookup_app_version(
                result->app, result->platform, result->version_num,
                result->plan_class
            );
            if (!app_version) {
                msg_printf(
                    project, MSG_INTERNAL_ERROR,
                    "State file error: app %s platform %s version %d not found\n",
                    result->app->name, result->platform, result->version_num
                );
                return ERR_NULL;
            }

            // make sure no two active tasks are in same slot
            //
            for (i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
                ACTIVE_TASK* atp = gstate.active_tasks.active_tasks[i];
                if (atp->slot == slot) {
                    msg_printf(project, MSG_INTERNAL_ERROR,
                        "State file error: two tasks in slot %d\n", slot
                    );
                    return ERR_BAD_RESULT_STATE;
                }
            }
            return 0;
        }
        else if (parse_str(buf, "<result_name>", result_name, sizeof(result_name))) continue;
        else if (parse_str(buf, "<project_master_url>", project_master_url, sizeof(project_master_url))) continue;
        else if (parse_int(buf, "<slot>", slot)) continue;
        else if (parse_bool(buf, "full_init_done", full_init_done)) continue;
        else if (parse_int(buf, "<active_task_state>", dummy)) continue;
        else if (parse_double(buf, "<checkpoint_cpu_time>", checkpoint_cpu_time)) continue;
        else if (parse_double(buf, "<fraction_done>", fraction_done)) continue;
        else if (parse_double(buf, "<current_cpu_time>", current_cpu_time)) continue;
        else if (parse_int(buf, "<app_version_num>", n)) continue;
        else if (parse_double(buf, "<swap_size>", procinfo.swap_size)) continue;
        else if (parse_double(buf, "<working_set_size>", procinfo.working_set_size)) continue;
        else if (parse_double(buf, "<working_set_size_smoothed>", procinfo.working_set_size_smoothed)) continue;
        else if (parse_double(buf, "<page_fault_rate>", procinfo.page_fault_rate)) continue;

        else if (parse_double(buf, "<stats_mem>", stats_mem)) continue;
        else if (parse_double(buf, "<stats_page>", stats_mem)) continue;
        else if (parse_double(buf, "<stats_pagefault_rate>", stats_mem)) continue;
        else if (parse_double(buf, "<stats_disk>", stats_disk)) continue;
        else if (parse_int(buf, "<stats_checkpoint>", stats_checkpoint)) continue;
        else {
            handle_unparsed_xml_warning("ACTIVE_TASK::parse", buf);
        }
    }
    return ERR_XML_PARSE;
}

/// Return the slot directory (relative path).
std::string ACTIVE_TASK::get_slot_dir() const {
    return slot_dir;
}

/// Write XML information about this active task set
int ACTIVE_TASK_SET::write(std::ostream& out) const {
    int retval;

    out << "<active_task_set>\n";
    for (size_t i=0; i<active_tasks.size(); i++) {
        retval = active_tasks[i]->write(out);
        if (retval) return retval;
    }
    out << "</active_task_set>\n";
    return 0;
}

/// Parse XML information about an active task set
int ACTIVE_TASK_SET::parse(MIOFILE& fin) {
    ACTIVE_TASK* atp;
    char buf[256];
    int retval;

    while (fin.fgets(buf, 256)) {
        if (match_tag(buf, "</active_task_set>")) return 0;
        else if (match_tag(buf, "<active_task>")) {
            atp = new ACTIVE_TASK;
            retval = atp->parse(fin);
            if (!retval) {
                if (slot_taken(atp->slot)) {
                    msg_printf(atp->result->project, MSG_INTERNAL_ERROR,
                        "slot %d in use; discarding result %s",
                        atp->slot, atp->result->name
                    );
                    retval = ERR_XML_PARSE;
                }
            }
            if (!retval) active_tasks.push_back(atp);
            else delete atp;
        } else {
            handle_unparsed_xml_warning("ACTIVE_TASK_SET::parse", buf);
        }
    }
    return ERR_XML_PARSE;
}

void MSG_QUEUE::init(const char* n) {
    strcpy(name, n);
    last_block = 0;
    msgs.clear();
}

void MSG_QUEUE::msg_queue_send(const char* msg, MSG_CHANNEL& channel) {
    if (msgs.empty() && channel.send_msg(msg)) {
        if (log_flags.app_msg_send) {
            msg_printf(NULL, MSG_INFO, "[app_msg_send] sent %s to %s", msg, name);
        }
        last_block = 0;
        return;
    }
    if (log_flags.app_msg_send) {
        msg_printf(NULL, MSG_INFO, "[app_msg_send] deferred %s to %s", msg, name);
    }
    msgs.push_back(std::string(msg));
    if (!last_block) last_block = gstate.now;
}

void MSG_QUEUE::msg_queue_poll(MSG_CHANNEL& channel) {
    if (!msgs.empty()) {
        if (log_flags.app_msg_send) {
            msg_printf(NULL, MSG_INFO,
                "[app_msg_send] poll: %lu msgs queued for %s:",
                msgs.size(), name
            );
        }
        if (channel.send_msg(msgs[0].c_str())) {
            if (log_flags.app_msg_send) {
                msg_printf(NULL, MSG_INFO, "[app_msg_send] poll: delayed sent %s", (msgs[0].c_str()));
            }
            msgs.erase(msgs.begin());
            last_block = 0;
        }
        for (unsigned int i=0; i<msgs.size(); i++) {
            if (log_flags.app_msg_send) {
                msg_printf(NULL, MSG_INFO, "[app_msg_send] poll:  deferred: %s", (msgs[0].c_str()));
            }
        }
    }
}

/// If the last message in the buffer is "msg", remove it and return 1.
int MSG_QUEUE::msg_queue_purge(const char* msg) {
    int count = msgs.size();
    if (!count) return 0;
    std::vector<std::string>::iterator iter = msgs.begin();
    for (int i=0; i<count-1; i++) {
        iter++;
    }
    if (log_flags.app_msg_send) {
        msg_printf(NULL, MSG_INFO,
            "[app_msg_send] purge: wanted  %s last msg is %s in %s",
            msg, iter->c_str(), name
        );
    }
    if (!strcmp(msg, iter->c_str())) {
        if (log_flags.app_msg_send) {
            msg_printf(NULL, MSG_INFO, "[app_msg_send] purged %s from %s", msg, name);
        }
        iter = msgs.erase(iter);
        return 1;
    }
    return 0;
}

bool MSG_QUEUE::timeout(double diff) {
    if (!last_block) return false;
    if (gstate.now - last_block > diff) {
        return true;
    }
    return false;
}

void ACTIVE_TASK_SET::report_overdue() const {
    unsigned int i;
    const ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        double diff = (gstate.now - atp->result->report_deadline)/86400;
        if (diff > 0) {
            msg_printf(atp->result->project, MSG_USER_ERROR,
                "Task %s is %.2f days overdue.", atp->result->name, diff
            );
            msg_printf(atp->result->project, MSG_USER_ERROR,
                "You may not get credit for it.  Consider aborting it."
            );
        }
    }
}

/// Scan the slot directory, looking for files with names
/// of the form boinc_ufr_X.
/// Then mark file X as being present (and uploadable)
int ACTIVE_TASK::handle_upload_files() {
    std::string filename;
    int retval;

    DirScanner dirscan(slot_dir);
    while (dirscan.scan(filename)) {
        if (starts_with(filename, UPLOAD_FILE_REQ_PREFIX)) {
            std::string& link_filename = filename;
            // strip the prefix
            std::string real_filename = filename.substr(strlen(UPLOAD_FILE_REQ_PREFIX));

            FILE_INFO* fip = result->lookup_file_logical(real_filename.c_str());
            if (fip) {
                std::string path = get_pathname(fip);
                retval = md5_file(path.c_str(), fip->md5_cksum, fip->nbytes);
                if (retval) {
                    fip->status = retval;
                } else {
                    fip->status = FILE_PRESENT;
                }
            } else {
                msg_printf(wup->project, MSG_INTERNAL_ERROR, "Can't find uploadable file %s", real_filename.c_str());
            }
            std::string path(slot_dir);
            path.append("/").append(link_filename);
            delete_project_owned_file(path.c_str(), true);  // delete the link file
        }
    }
    return 0;
}

void ACTIVE_TASK_SET::handle_upload_files() {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        atp->handle_upload_files();
    }
}

bool ACTIVE_TASK_SET::want_network() const {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        const ACTIVE_TASK* atp = active_tasks[i];
        if (atp->want_network) return true;
    }
    return false;
}

void ACTIVE_TASK_SET::network_available() {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (atp->want_network) {
            atp->send_network_available();
        }
    }
}

void ACTIVE_TASK::upload_notify_app(const FILE_INFO* fip, const FILE_REF* frp) {
    std::ostringstream path;
    path << slot_dir << '/' << UPLOAD_FILE_STATUS_PREFIX << frp->open_name;
    FILE* f = boinc_fopen(path.str().c_str(), "w");
    if (!f) return;
    fprintf(f, "<status>%d</status>\n", fip->status);
    fclose(f);
    send_upload_file_status = true;
}

/// Check if everything was initialized before, including things like
/// soft links in the slot directory.
///
/// \return true if everything is initialized.
bool ACTIVE_TASK::is_full_init_done() const {
    return full_init_done;
}

/// a file upload has finished.
/// If any running apps are waiting for it, notify them.
void ACTIVE_TASK_SET::upload_notify_app(const FILE_INFO* fip) {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        const RESULT* rp = atp->result;
        const FILE_REF* frp = rp->lookup_file(fip);
        if (frp) {
            atp->upload_notify_app(fip, frp);
        }
    }
}

void ACTIVE_TASK_SET::init() {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        atp->init(atp->result);
        atp->scheduler_state = CPU_SCHED_PREEMPTED;
        atp->read_task_state_file();
    }
}
