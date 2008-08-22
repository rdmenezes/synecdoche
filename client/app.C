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

// Abstraction of a set of executing applications,
// connected to I/O files in various ways.
// Shouldn't depend on CLIENT_STATE.

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

#include <cctype>
#include <ctime>
#include <cstdio>
#include <cmath>
#include <cstdlib>

#endif

#ifdef SIM
#include "sim.h"
#else
#include "client_state.h"
#include "client_types.h"
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "file_names.h"
#include "parse.h"
#include "shmem.h"
#include "str_util.h"
#include "client_msgs.h"
#include "procinfo.h"
#include "sandbox.h"
#include "app.h"

using std::max;
using std::min;

#define ABORT_TIMEOUT   60
    // if we send app <abort> request, wait this long before killing it.
    // This gives it time to download symbol files (which can be several MB)
    // and write stack trace to stderr
#define QUIT_TIMEOUT    10
    // Same, for <quit>.  Shorter because no stack trace is generated

ACTIVE_TASK::~ACTIVE_TASK() {
#ifndef SIM
    cleanup_task();
#endif
}

#ifndef SIM

ACTIVE_TASK::ACTIVE_TASK() {
    result = NULL;
    wup = NULL;
    app_version = NULL;
    pid = 0;
    slot = 0;
    _task_state = PROCESS_UNINITIALIZED;
    scheduler_state = CPU_SCHED_UNINITIALIZED;
    signal = 0;
    strcpy(slot_dir, "");
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
}

static const char* task_state_name(int val) {
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

void ACTIVE_TASK::set_task_state(int val, const char* where) {
    _task_state = val;
    if (log_flags.task_debug) {
        msg_printf(result->project, MSG_INFO,
            "[task_debug] task_state=%s for %s from %s",
            task_state_name(val), result->name, where
        );
    }
}

#ifdef _WIN32

// call this when a process has exited but will be started again
// (e.g. suspend via quit, exited but no finish file).
// In these cases we want to keep the shmem and events
//
void ACTIVE_TASK::close_process_handles() {
    if (pid_handle) {
        CloseHandle(pid_handle);
        pid_handle = NULL;
    }
}
#endif

// called when a process has exited
//
void ACTIVE_TASK::cleanup_task() {
#ifdef _WIN32
    // detach from shared mem.
    // This will destroy shmem seg since we're the last attachment
    //
    if (app_client_shm.shm) {
        detach_shmem(shm_handle, app_client_shm.shm);
        app_client_shm.shm = NULL;
    }
#else
    int retval;
    
    if (app_client_shm.shm) {
#ifndef __EMX__
        if (app_version->api_major_version() >= 6) {
            retval = detach_shmem_mmap(app_client_shm.shm, sizeof(SHARED_MEM));
        } else
#endif
        {
            retval = detach_shmem(app_client_shm.shm);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Couldn't detach shared memory: %s", boincerror(retval)
                );
            }
            retval = destroy_shmem(shmem_seg_name);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
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
    get_slot_dir(slot, slot_dir, sizeof(slot_dir));
    relative_to_absolute(slot_dir, slot_path);
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
                    "[mem_usage_debug] %s: RAM %.2fMB, page %.2fMB, %.2f page faults/sec, user CPU %.3f, kernel CPU %.3f",
                    atp->result->name,
                    pi.working_set_size/MEGA, pi.swap_size/MEGA,
                    pi.page_fault_rate,
                    pi.user_time, pi.kernel_time
                );
            }
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

// Do periodic checks on running apps:
// - get latest CPU time and % done info
// - check if any has exited, and clean up
// - see if any has exceeded its CPU or disk space limits, and abort it
//
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
    action |= get_msgs();
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

// There's a new trickle file.
// Move it from slot dir to project dir
//
int ACTIVE_TASK::move_trickle_file() {
    char project_dir[256], new_path[1024], old_path[1024];
    int retval;

    get_project_dir(result->project, project_dir, sizeof(project_dir));
    sprintf(old_path, "%s/trickle_up.xml", slot_dir);
    sprintf(new_path,
        "%s/trickle_up_%s_%d.xml",
        project_dir, result->name, (int)time(0)
    );
    retval = boinc_rename(old_path, new_path);

    // if can't move it, remove
    //
    if (retval) {
        delete_project_owned_file(old_path, true);
        return ERR_RENAME;
    }
    return 0;
}

// size of output files and files in slot dir
//
int ACTIVE_TASK::current_disk_usage(double& size) const {
    double x;
    unsigned int i;
    int retval;
    const FILE_INFO* fip;
    char path[1024];

    retval = dir_size(slot_dir, size);
    if (retval) return retval;
    for (i=0; i<result->output_files.size(); i++) {
        fip = result->output_files[i].file_info;
        get_pathname(fip, path, sizeof(path));
        retval = file_size(path, x);
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

bool ACTIVE_TASK_SET::is_slot_dir_in_use(const char* dir) const {
    char path[1024];
    unsigned int i;
    for (i=0; i<active_tasks.size(); i++) {
        get_slot_dir(active_tasks[i]->slot, path, sizeof(path));
        if (!strcmp(path, dir)) return true;
    }
    return false;
}

// Get a free slot,
// and make a slot dir if needed
//
int ACTIVE_TASK_SET::get_free_slot() const {
    int j, retval;
    char path[1024];

    for (j=0; ; j++) {
        if (is_slot_in_use(j)) continue;

        // make sure we can make an empty directory for this slot
        //
        get_slot_dir(j, path, sizeof(path));
        if (boinc_file_exists(path)) {
            if (is_dir(path)) {
                retval = client_clean_out_dir(path);
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
int ACTIVE_TASK::write(MIOFILE& fout) const {
    fout.printf(
        "<active_task>\n"
        "    <project_master_url>%s</project_master_url>\n"
        "    <result_name>%s</result_name>\n"
        "    <active_task_state>%d</active_task_state>\n"
        "    <app_version_num>%d</app_version_num>\n"
        "    <slot>%d</slot>\n"
        "    <checkpoint_cpu_time>%f</checkpoint_cpu_time>\n"
        "    <fraction_done>%f</fraction_done>\n"
        "    <current_cpu_time>%f</current_cpu_time>\n"
        "    <swap_size>%f</swap_size>\n"
        "    <working_set_size>%f</working_set_size>\n"
        "    <working_set_size_smoothed>%f</working_set_size_smoothed>\n"
        "    <page_fault_rate>%f</page_fault_rate>\n",
        result->project->master_url,
        result->name,
        task_state(),
        app_version->version_num,
        slot,
        checkpoint_cpu_time,
        fraction_done,
        current_cpu_time,
        procinfo.swap_size,
        procinfo.working_set_size,
        procinfo.working_set_size_smoothed,
        procinfo.page_fault_rate
    );
    fout.printf("</active_task>\n");
    return 0;
}

int ACTIVE_TASK::write_gui(MIOFILE& fout) const {
    fout.printf(
        "<active_task>\n"
        "    <active_task_state>%d</active_task_state>\n"
        "    <app_version_num>%d</app_version_num>\n"
        "    <slot>%d</slot>\n"
        "    <scheduler_state>%d</scheduler_state>\n"
        "    <checkpoint_cpu_time>%f</checkpoint_cpu_time>\n"
        "    <fraction_done>%f</fraction_done>\n"
        "    <current_cpu_time>%f</current_cpu_time>\n"
        "    <swap_size>%f</swap_size>\n"
        "    <working_set_size>%f</working_set_size>\n"
        "    <working_set_size_smoothed>%f</working_set_size_smoothed>\n"
        "    <page_fault_rate>%f</page_fault_rate>\n"
        "%s"
        "%s",
        task_state(),
        app_version->version_num,
        slot,
        scheduler_state,
        checkpoint_cpu_time,
        fraction_done,
        current_cpu_time,
        procinfo.swap_size,
        procinfo.working_set_size,
        procinfo.working_set_size_smoothed,
        procinfo.page_fault_rate,
        too_large?"   <too_large/>\n":"",
        needs_shmem?"   <needs_shmem/>\n":""
    );
    if (strlen(app_version->graphics_exec_path)) {
        fout.printf(
            "   <graphics_exec_path>%s</graphics_exec_path>\n"
            "   <slot_path>%s</slot_path>\n",
            app_version->graphics_exec_path,
            slot_path
        );
    }
    if (supports_graphics() && !gstate.disable_graphics) {
        fout.printf(
            "   <supports_graphics/>\n"
            "   <graphics_mode_acked>%d</graphics_mode_acked>\n",
            graphics_mode_acked
        );
    }
    fout.printf("</active_task>\n");
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
        else if (parse_int(buf, "<active_task_state>", dummy)) continue;
        else if (parse_double(buf, "<checkpoint_cpu_time>", checkpoint_cpu_time)) continue;
        else if (parse_double(buf, "<fraction_done>", fraction_done)) continue;
        else if (parse_double(buf, "<current_cpu_time>", current_cpu_time)) continue;
        else if (parse_int(buf, "<app_version_num>", n)) continue;
        else if (parse_double(buf, "<swap_size>", procinfo.swap_size)) continue;
        else if (parse_double(buf, "<working_set_size>", procinfo.working_set_size)) continue;
        else if (parse_double(buf, "<working_set_size_smoothed>", procinfo.working_set_size_smoothed)) continue;
        else if (parse_double(buf, "<page_fault_rate>", procinfo.page_fault_rate)) continue;
        else {
            if (log_flags.unparsed_xml) {
                msg_printf(0, MSG_INFO,
                    "[unparsed_xml] ACTIVE_TASK::parse(): unrecognized %s\n", buf
                );
            }
        }
    }
    return ERR_XML_PARSE;
}

void ACTIVE_TASK::reserve_coprocs() {
    gstate.coprocs.reserve_coprocs(
        app_version->coprocs, log_flags.cpu_sched_debug
    );
    coprocs_reserved = true;
}

void ACTIVE_TASK::free_coprocs() {
    if (!coprocs_reserved) return;
    gstate.coprocs.free_coprocs(
        app_version->coprocs, log_flags.cpu_sched_debug
    );
    coprocs_reserved = false;
}

// Write XML information about this active task set
//
int ACTIVE_TASK_SET::write(MIOFILE& fout) const {
    unsigned int i;
    int retval;

    fout.printf("<active_task_set>\n");
    for (i=0; i<active_tasks.size(); i++) {
        retval = active_tasks[i]->write(fout);
        if (retval) return retval;
    }
    fout.printf("</active_task_set>\n");
    return 0;
}

// Parse XML information about an active task set
//
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
            if (log_flags.unparsed_xml) {
                msg_printf(0, MSG_INFO,
                    "[unparsed_xml] ACTIVE_TASK_SET::parse(): unrecognized %s\n", buf
                );
            }
        }
    }
    return ERR_XML_PARSE;
}

void MSG_QUEUE::init(char* n) {
	strcpy(name, n);
	last_block = 0;
	msgs.clear();
}

void MSG_QUEUE::msg_queue_send(const char* msg, MSG_CHANNEL& channel) {
    if ((msgs.size()==0) && channel.send_msg(msg)) {
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
    if (msgs.size() > 0) {
		if (log_flags.app_msg_send) {
			msg_printf(NULL, MSG_INFO,
				"[app_msg_send] poll: %d msgs queued for %s:",
				(int)msgs.size(), name
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

// if the last message in the buffer is "msg", remove it and return 1
//
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

// scan the slot directory, looking for files with names
// of the form boinc_ufr_X.
// Then mark file X as being present (and uploadable)
//
int ACTIVE_TASK::handle_upload_files() {
    std::string filename;
    char buf[256], path[1024];
    int retval;

    DirScanner dirscan(slot_dir);
    while (dirscan.scan(filename)) {
        strcpy(buf, filename.c_str());
        if (strstr(buf, UPLOAD_FILE_REQ_PREFIX) == buf) {
            char* p = buf+strlen(UPLOAD_FILE_REQ_PREFIX);
            FILE_INFO* fip = result->lookup_file_logical(p);
            if (fip) {
                get_pathname(fip, path, sizeof(path));
                retval = md5_file(path, fip->md5_cksum, fip->nbytes);
                if (retval) {
                    fip->status = retval;
                } else {
                    fip->status = FILE_PRESENT;
                }
            } else {
                msg_printf(0, MSG_INTERNAL_ERROR, "Can't find uploadable file %s", p);
            }
            sprintf(path, "%s/%s", slot_dir, buf);
            delete_project_owned_file(path, true);  // delete the link file
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
    char path[256];
    sprintf(path, "%s/%s%s", slot_dir, UPLOAD_FILE_STATUS_PREFIX, frp->open_name);
    FILE* f = boinc_fopen(path, "w");
    if (!f) return;
    fprintf(f, "<status>%d</status>\n", fip->status);
    fclose(f);
    send_upload_file_status = true;
}

// a file upload has finished.
// If any running apps are waiting for it, notify them
//
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
    }
}

#endif

const char *BOINC_RCSID_778b61195e = "$Id: app.C 15287 2008-05-23 21:24:36Z davea $";