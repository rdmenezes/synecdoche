// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Nicolas Alvarez, Peter Kortschack
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
/// monitoring and process control of running apps

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0x0                 // may be in ntstatus.h
#endif
#ifndef STATUS_DLL_INIT_FAILED
#define STATUS_DLL_INIT_FAILED 0xC0000142  // may be in ntstatus.h
#endif

#else
#include "config.h"
#include <unistd.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_CSIGNAL
#include <csignal>
#elif defined(HAVE_SIGNAL_H)
#include <signal.h>
#endif
#ifdef HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#endif

#include "app.h"

#include "filesys.h"
#include "error_numbers.h"
#include "util.h"
#include "str_util.h"
#include "parse.h"
#include "shmem.h"
#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"
#include "procinfo.h"
#include "sandbox.h"
#include "xml_write.h"

#ifdef _WIN32
bool ACTIVE_TASK::kill_all_children() {
    unsigned int i,j;
    std::vector<PROCINFO> ps;
    std::vector<PROCINFO> tps;

    procinfo_setup(ps);

    PROCINFO pi;
    pi.id = pid;
    tps.push_back(pi);

    for (i=0; i < tps.size(); i++) {
        PROCINFO tp = tps[i];
        for (j=0; j < ps.size(); j++) {
            PROCINFO p = ps[j];
            if (tp.id == p.parentid) {
                if (TerminateProcessById(p.id)) {
                    tps.push_back(p);
                }
            }
        }
    }
    return true;
}
#endif

/// Ask the process to exit gracefully,
/// i.e.\ by sending a <quit> message
///
/// \return 1 if shared memory is not set up, 0 on success.
int ACTIVE_TASK::request_exit() {
    if (!app_client_shm.shm) return 1;
    process_control_queue.msg_queue_send(
        "<quit/>",
        app_client_shm.shm->process_control_request
    );
    quit_time = gstate.now;
    return 0;
}

/// Send an abort message.
int ACTIVE_TASK::request_abort() {
    if (!app_client_shm.shm) return 1;
    process_control_queue.msg_queue_send(
        "<abort/>",
        app_client_shm.shm->process_control_request
    );
    return 0;
}

/// Kill process forcibly,
/// Unix: send a SIGKILL signal, Windows: TerminateProcess()
/// If \a restart is true, arrange for resulted to get restarted;
/// otherwise it ends with an error
///
/// \param[in] restart If true arrange for restart
/// \return Always returns 0.
int ACTIVE_TASK::kill_task(bool restart) {
#ifdef _WIN32
    TerminateProcessById(pid);
#else
#ifdef SANDBOX
    kill_via_switcher(pid);
    // Also kill app directly, just to be safe
    //
#endif
    kill(pid, SIGKILL);
#endif

    cleanup_task();
    if (restart) {
        set_task_state(PROCESS_UNINITIALIZED, "kill_task");
        gstate.request_enforce_schedule("Task restart");
    } else {
        set_task_state(PROCESS_ABORTED, "kill_task");
    }
    return 0;
}

/// We have sent a quit request to the process; see if it's exited.
/// This is called when the core client exits,
/// or when a project is detached or reset
///
/// \return True if the task has exited, false otherwise.
bool ACTIVE_TASK::has_task_exited() {
    bool exited = false;

    if (!process_exists()) return true;

#ifdef _WIN32
    unsigned long exit_code;
    if (GetExitCodeProcess(pid_handle, &exit_code)) {
        if (exit_code != STILL_ACTIVE) {
            exited = true;
        }
    }
#else
    if (waitpid(pid, 0, WNOHANG) == pid) {
        exited = true;
    }
#endif
    if (exited) {
        set_task_state(PROCESS_EXITED, "has_task_exited");
        cleanup_task();
    }
    return exited;
}


static void limbo_message(ACTIVE_TASK& at) {
#ifdef _WIN32
    if (at.result->exit_status == STATUS_DLL_INIT_FAILED) {
        msg_printf(at.result->project, MSG_INFO,
            "Task %s exited with a DLL initialization error.",
            at.result->name
        );
        msg_printf(at.result->project, MSG_INFO,
            "If this happens repeatedly you may need to reboot your computer."
        );
    } else {
#endif
        msg_printf(at.result->project, MSG_INFO,
            "Task %s exited with zero status but no 'finished' file",
            at.result->name
        );
        msg_printf(at.result->project, MSG_INFO,
            "If this happens repeatedly you may need to reset the project."
        );
#ifdef _WIN32
    }
#endif
}

/// Handle a task that exited prematurely (i.e.\ the job isn't done).
///
/// \param[out] will_restart Reference to a bool-variable that will be set
///                          to true if the task should get restarted.
void ACTIVE_TASK::handle_premature_exit(bool& will_restart) {
    // if it exited because we sent it a quit message, don't count
    //
    if (task_state() == PROCESS_QUIT_PENDING) {
        set_task_state(PROCESS_UNINITIALIZED, "handle_premature_exit");
        will_restart = true;
        return;
    }

    // otherwise keep count of exits;
    // restart it unless this happens 100 times w/o a checkpoint
    //
    premature_exit_count++;
    if (premature_exit_count > 100) {
        set_task_state(PROCESS_ABORTED, "handle_premature_exit");
        result->exit_status = ERR_TOO_MANY_EXITS;
        gstate.report_result_error(*result, "too many exit(0)s");
        result->set_state(RESULT_ABORTED, "handle_premature_exit");
    } else {
        will_restart = true;
        limbo_message(*this);
        set_task_state(PROCESS_UNINITIALIZED, "handle_premature_exit");
    }
}

/// Deal with a process that has exited, for whatever reason:
/// - completion
/// - crash
/// - preemption via quit
#ifdef _WIN32
void ACTIVE_TASK::handle_exited_app(unsigned long exit_code)
#else
void ACTIVE_TASK::handle_exited_app(int stat)
#endif
{
    bool will_restart = false;

    if (log_flags.task_debug) {
        msg_printf(result->project, MSG_INFO,
            "[task_debug] Process for %s exited",
            result->name
        );
    }

    get_app_status_msg();
    get_trickle_up_msg();
    result->final_cpu_time = current_cpu_time;
    if (task_state() == PROCESS_ABORT_PENDING) {
        set_task_state(PROCESS_ABORTED, "handle_exited_app");
    } else {
#ifdef _WIN32
        result->exit_status = exit_code;
        switch(exit_code) {
        case STATUS_SUCCESS:
            // if another process killed the app, it looks like exit(0).
            // So check for the finish file
            //
            if (finish_file_present()) {
                set_task_state(PROCESS_EXITED, "handle_exited_app");
                break;
            }
            handle_premature_exit(will_restart);
            break;
        case 0xc000013a:        // control-C??
        case 0x40010004:        // vista shutdown?? can someone explain this?
        case STATUS_DLL_INIT_FAILED:
            // This can happen because:
            // - The OS is shutting down, and attempting to start
            //   any new application fails automatically.
            // - The OS has run out of desktop heap
            // - (reportedly) The computer has just come out of hibernation
            //
            handle_premature_exit(will_restart);
            break;
        default:
            char szError[1024];
            set_task_state(PROCESS_EXITED, "handle_exited_app");
            gstate.report_result_error(
                *result,
                "%s - exit code %d (0x%x)",
                windows_format_error_string(exit_code, szError, sizeof(szError)),
                exit_code, exit_code
            );
            if (log_flags.task_debug) {
                msg_printf(result->project, MSG_INFO,
                    "[task_debug] Process for %s exited",
                    result->name
                );
                msg_printf(result->project, MSG_INFO,
                    "[task_debug] exit code %d (0x%x): %s",
                    exit_code, exit_code,
                    windows_format_error_string(exit_code, szError, sizeof(szError))
                );
            }
            break;
        }
#else
        if (WIFEXITED(stat)) {
            result->exit_status = WEXITSTATUS(stat);

            if (result->exit_status) {
                set_task_state(PROCESS_EXITED, "handle_exited_app");
                gstate.report_result_error(
                    *result,
                    "process exited with code %d (0x%x, %d)",
                    result->exit_status, result->exit_status,
                    (-1<<8)|result->exit_status
                );
            } else {
                if (finish_file_present()) {
                    set_task_state(PROCESS_EXITED, "handle_exited_app");
                } else {
                    handle_premature_exit(will_restart);
                }
            }
            if (log_flags.task_debug) {
                msg_printf(result->project, MSG_INFO,
                    "[task_debug] exit status %d\n",
                    result->exit_status
                );
            }
        } else if (WIFSIGNALED(stat)) {
            int got_signal = WTERMSIG(stat);

            if (log_flags.task_debug) {
                msg_printf(result->project, MSG_INFO,
                    "[task_debug] process got signal %d", signal
                );
            }

            // if the process was externally killed, allow it to restart.
            //
            switch (got_signal) {
            case SIGHUP:
            case SIGINT:
            case SIGQUIT:
            case SIGKILL:
            case SIGTERM:
            case SIGSTOP:
                will_restart = true;
                set_task_state(PROCESS_UNINITIALIZED, "handle_exited_app");
                limbo_message(*this);
                break;
            default:
                result->exit_status = stat;
                set_task_state(PROCESS_WAS_SIGNALED, "handle_exited_app");
                signal = got_signal;
                gstate.report_result_error(
                    *result, "process got signal %d", signal
                );
            }
        } else {
            result->exit_status = -1;
            set_task_state(PROCESS_EXIT_UNKNOWN, "handle_exited_app");
            gstate.report_result_error(*result, "process exit, unknown");
            msg_printf(result->project, MSG_INTERNAL_ERROR,
                "process exited for unknown reason"
            );
        }
#endif
    }

    cleanup_task();         // Always release shared memory
    if (gstate.exit_after_finish) {
        exit(0);
    }

    if (!will_restart) {
        copy_output_files();
        read_stderr_file();
        client_clean_out_dir(slot_dir.c_str());
    }
    gstate.request_schedule_cpus("application exited");
    gstate.request_work_fetch("application exited");
}

bool ACTIVE_TASK::finish_file_present() const {
    std::ostringstream path;
    path << slot_dir << '/' << BOINC_FINISH_CALLED_FILE;
    return (boinc_file_exists(path.str()) != 0);
}

void ACTIVE_TASK_SET::send_trickle_downs() {
    bool sent;
    for (size_t i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        if (atp->have_trickle_down) {
            if (!atp->app_client_shm.shm) continue;
            sent = atp->app_client_shm.shm->trickle_down.send_msg("<have_trickle_down/>\n");
            if (sent) atp->have_trickle_down = false;
        }
        if (atp->send_upload_file_status) {
            if (!atp->app_client_shm.shm) continue;
            sent = atp->app_client_shm.shm->trickle_down.send_msg("<upload_file_status/>\n");
            if (sent) atp->send_upload_file_status = false;
       }
    }
}

void ACTIVE_TASK_SET::send_heartbeats() {
    char buf[256];
    double ar = gstate.available_ram();

    for (size_t i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        if (!atp->app_client_shm.shm) continue;
        sprintf(buf, "<heartbeat/>"
            "<wss>%f</wss>"
            "<max_wss>%f</max_wss>",
            atp->procinfo.working_set_size, ar
        );
        bool sent = atp->app_client_shm.shm->heartbeat.send_msg(buf);
        if (log_flags.app_msg_send) {
            if (sent) {
                msg_printf(atp->result->project, MSG_INFO,
                    "[app_msg_send sent heartbeat to %s", atp->result->name);
            } else {
                msg_printf(atp->result->project, MSG_INFO,
                    "[app_msg_send] failed to send heartbeat to %s", atp->result->name);
            }
        }
    }
}

void ACTIVE_TASK_SET::process_control_poll() {

    for (size_t i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        if (!atp->app_client_shm.shm) continue;

        // if app has had the same message in its send buffer for 180 sec,
        // assume it's hung and restart it
        //
        if (atp->process_control_queue.timeout(180)) {
            if (log_flags.task_debug) {
                msg_printf(atp->result->project, MSG_INFO,
                    "Restarting %s - message timeout", atp->result->name
                );
            }
            atp->kill_task(true);
        } else {
            atp->process_control_queue.msg_queue_poll(
                atp->app_client_shm.shm->process_control_request
            );
        }
    }
}

/// See if any processes have exited.
///
/// \return True if at least one process has exited, false otherwise.
bool ACTIVE_TASK_SET::check_app_exited() {
    bool found = false;

#ifdef _WIN32
    unsigned long exit_code;

    for (size_t i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        if (GetExitCodeProcess(atp->pid_handle, &exit_code)) {
            if (exit_code != STILL_ACTIVE) {
                found = true;
                atp->handle_exited_app(exit_code);
            }
        } else {
            if (log_flags.task_debug) {
                char errmsg[1024];
                msg_printf(atp->result->project, MSG_INFO,
                    "[task_debug] task %s GetExitCodeProcess() failed - %s GLE %d (0x%x)",
                    atp->result->name,
                    windows_format_error_string(
                        GetLastError(), errmsg, sizeof(errmsg)
                    ),
                    GetLastError(), GetLastError()
                );
            }

            // The process doesn't seem to be there.
            // Mark task as aborted so we don't check it again.
            //
            atp->set_task_state(PROCESS_ABORTED, "check_app_exited");
        }
    }
#else
    int pid, stat;

    if ((pid = waitpid(0, &stat, WNOHANG)) > 0) {
        ACTIVE_TASK* atp = lookup_pid(pid);
        if (!atp) {
            // if we're running benchmarks, exited process
            // is probably a benchmark process; don't show error
            //
            if (!gstate.are_cpu_benchmarks_running() && log_flags.task_debug) {
                msg_printf(0, MSG_INTERNAL_ERROR, "Process %d not found\n", pid);
            }
            return false;
        }
        atp->handle_exited_app(stat);
        found = true;
    }
#endif

    return found;
}

/// if an app has exceeded its maximum disk usage, abort it
bool ACTIVE_TASK::check_max_disk_exceeded() {
    double disk_usage;
    int retval;

    retval = current_disk_usage(disk_usage);
    if (retval) {
        msg_printf(this->wup->project, MSG_INTERNAL_ERROR,
            "Can't get task disk usage: %s", boincerror(retval)
        );
    } else {
        // Track the maximum disk usage:
        stats_disk = std::max(stats_disk, disk_usage);

        if (disk_usage > max_disk_usage) {
            msg_printf(
                result->project, MSG_INFO,
                "Aborting task %s: exceeded disk limit: %.2fMB > %.2fMB\n",
                result->name, disk_usage/MEGA, max_disk_usage/MEGA
            );
            abort_task(ERR_RSC_LIMIT_EXCEEDED, "Maximum disk usage exceeded");
            return true;
        }
    }
    return false;
}

/// Check if any of the active tasks have exceeded their
/// resource limits on disk, CPU time or memory
bool ACTIVE_TASK_SET::check_rsc_limits_exceeded() {
    static double last_disk_check_time = 0;
    bool do_disk_check = false;
    bool did_anything = false;

    double ram_left = gstate.available_ram();
    double max_ram = gstate.max_available_ram();

    // Some slot dirs have lots of files,
    // so only check every min(disk_interval, 300) secs
    //
    double min_interval = gstate.global_prefs.disk_interval;
    if (min_interval < 300) min_interval = 300;
    if (gstate.now > last_disk_check_time + min_interval) {
        do_disk_check = true;
    }
    for (size_t i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (atp->task_state() != PROCESS_EXECUTING) continue;
        if (atp->current_cpu_time > atp->max_cpu_time) {
            msg_printf(atp->result->project, MSG_INFO,
                "Aborting task %s: exceeded CPU time limit %f\n",
                atp->result->name, atp->max_cpu_time
            );
            atp->abort_task(ERR_RSC_LIMIT_EXCEEDED, "Maximum CPU time exceeded");
            did_anything = true;
            continue;
        }
        if (atp->procinfo.working_set_size_smoothed > max_ram) {
            msg_printf(atp->result->project, MSG_INFO,
                "Aborting task %s: exceeded memory limit %.2fMB > %.2fMB\n",
                atp->result->name,
                atp->procinfo.working_set_size_smoothed/MEGA, max_ram/MEGA
            );
            atp->abort_task(ERR_RSC_LIMIT_EXCEEDED, "Maximum memory exceeded");
            did_anything = true;
            continue;
        }
        if (do_disk_check && atp->check_max_disk_exceeded()) {
            did_anything = true;
            continue;
        }
        ram_left -= atp->procinfo.working_set_size_smoothed;
    }
    if (ram_left < 0) {
        gstate.request_schedule_cpus("RAM usage limit exceeded");
    }
    if (do_disk_check) {
        last_disk_check_time = gstate.now;
    }
    return did_anything;
}

/// Abort a task.
/// If process is running, send it an "abort" message,
/// Set a flag so that if it doesn't exit within 5 seconds,
/// kill it by OS-specific mechanism (e.g. KILL signal).
/// This is done when app has exceeded CPU, disk, or mem limits,
/// or when the user has requested it.
///
/// \param[in] exit_status The exit status that should get reported.
/// \param[in] msg Message explaining why the tast was aborted.
/// \return Always returns 0.
int ACTIVE_TASK::abort_task(int exit_status, const char* msg) {
    if (task_state() == PROCESS_EXECUTING || task_state() == PROCESS_SUSPENDED) {
        set_task_state(PROCESS_ABORT_PENDING, "abort_task");
        abort_time = gstate.now;
        request_abort();
    } else {
        set_task_state(PROCESS_ABORTED, "abort_task");
    }
    result->exit_status = exit_status;
    gstate.report_result_error(*result, msg);
    result->set_state(RESULT_ABORTED, "abort_task");
    return 0;
}

/// Check for the stderr file, copy to result record.
///
/// \return True on success, false otherwise.
bool ACTIVE_TASK::read_stderr_file() {
    std::string stderr_file;

    // truncate stderr output to the last 63KB;
    // it's unlikely that more than that will be useful
    //
    int max_len = 63*1024;
    std::string path = slot_dir + std::string("/")
                                + std::string(STDERR_FILE);
    if (!boinc_file_exists(path.c_str())) return false;
    if (read_file_string(path.c_str(), stderr_file, max_len, true)) return false;

    result->stderr_out += "<stderr_txt>\n";
    result->stderr_out += stderr_file;
    result->stderr_out += "\n</stderr_txt>\n";
    return true;
}

/// Tell a running app to reread project preferences.
/// This is called when project prefs change,
/// or when a user file has finished downloading.
int ACTIVE_TASK::request_reread_prefs() {
    int retval;

    link_user_files();

    retval = write_app_init_file();
    if (retval) return retval;
    graphics_request_queue.msg_queue_send(
        xml_graphics_modes[MODE_REREAD_PREFS],
        app_client_shm.shm->graphics_request
    );
    return 0;
}

/// Tell a running app to reread the app_info file
/// (e.g.\ because proxy settings have changed)
int ACTIVE_TASK::request_reread_app_info() {
    int retval = write_app_init_file();
    if (retval) return retval;
    process_control_queue.msg_queue_send(
        "<reread_app_info/>",
        app_client_shm.shm->process_control_request
    );
    return 0;
}

/// Tell all running apps of a project to reread prefs.
void ACTIVE_TASK_SET::request_reread_prefs(PROJECT* project) {

    for (size_t i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (atp->result->project != project) continue;
        if (!atp->process_exists()) continue;
        atp->request_reread_prefs();
    }
}

void ACTIVE_TASK_SET::request_reread_app_info() {
    for (size_t i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        atp->request_reread_app_info();
    }
}


/// Rend quit signal to all tasks in the project
/// (or all tasks, if proj==0).
/// If they don't exit in 5 seconds,
/// send them a kill signal and wait up to 5 more seconds to exit.
/// This is called when the core client exits,
/// or when a project is detached or reset.
int ACTIVE_TASK_SET::exit_tasks(PROJECT* proj) {
    request_tasks_exit(proj);

    // Wait 5 seconds for them to exit normally; if they don't then kill them
    //
    if (wait_for_exit(5, proj)) {
        kill_tasks(proj);
    }
    wait_for_exit(5, proj);

    // get final checkpoint_cpu_times
    //
    get_msgs();

    gstate.request_schedule_cpus("exit_tasks");
    return 0;
}

/// Wait up to wait_time seconds for processes to exit.
/// If proj is zero, wait for all processes, else that project's.
/// NOTE: it's bad form to sleep, but it would be complex to avoid it here.
int ACTIVE_TASK_SET::wait_for_exit(double wait_time, PROJECT* proj) {
    bool all_exited;
    unsigned int i,n;
    ACTIVE_TASK *atp;

    for (i=0; i<10; i++) {
        all_exited = true;

        for (n=0; n<active_tasks.size(); n++) {
            atp = active_tasks[n];
            if (proj && atp->wup->project != proj) continue;
            if (!atp->has_task_exited()) {
                all_exited = false;
                break;
            }
        }

        if (all_exited) return 0;
        boinc_sleep(wait_time/10.0);
    }

    return ERR_NOT_EXITED;
}

int ACTIVE_TASK_SET::abort_project(PROJECT* project) {
    std::vector<ACTIVE_TASK*>::iterator task_iter;
    ACTIVE_TASK* atp;

    exit_tasks(project);
    task_iter = active_tasks.begin();
    while (task_iter != active_tasks.end()) {
        atp = *task_iter;
        if (atp->result->project == project) {
            task_iter = active_tasks.erase(task_iter);
            delete atp;
        } else {
            task_iter++;
        }
    }
    project->long_term_debt = 0;
    return 0;
}

/// Suspend all currently running tasks.
/// called only from CLIENT_STATE::suspend_tasks(),
/// e.g. because on batteries, time of day, benchmarking, CPU throttle, etc.
void ACTIVE_TASK_SET::suspend_all(bool cpu_throttle) {
    // Only allow apps to be removed from memory if they are not suspended
    // because of CPU throttling.
    bool leave_in_mem = true;
    if (!cpu_throttle) {
        leave_in_mem = gstate.global_prefs.leave_apps_in_memory;
    }

    for (ACTIVE_TASK_PVEC::iterator it = active_tasks.begin(); it != active_tasks.end(); ++it) {
        ACTIVE_TASK* task = *it;
        if (task->task_state() != PROCESS_EXECUTING) {
            continue;
        }
        if (cpu_throttle) {
            // If we're doing CPU throttling, don't bother suspending apps
            // that don't use a full CPU.
            if ((task->result->project->non_cpu_intensive) || (task->app_version->avg_ncpus < 1.0)) {
                continue;
            }
        }
        task->preempt(!leave_in_mem);
    }
}

/// Resume all currently scheduled tasks.
void ACTIVE_TASK_SET::unsuspend_all() {
    ACTIVE_TASK* atp;
    for (size_t i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        if (atp->task_state() == PROCESS_UNINITIALIZED) {
            if (atp->start()) {
                msg_printf(atp->wup->project, MSG_INTERNAL_ERROR,
                    "Couldn't restart task %s", atp->result->name
                );
            }
        } else if (atp->task_state() == PROCESS_SUSPENDED) {
            atp->unsuspend();
        }
    }
}

/// Check to see if any tasks are running.
/// called if benchmarking and waiting for suspends to happen
bool ACTIVE_TASK_SET::is_task_executing() const {
    for (size_t i=0; i<active_tasks.size(); i++) {
        const ACTIVE_TASK* atp = active_tasks[i];
        if (atp->task_state() == PROCESS_EXECUTING) {
            return true;
        }
    }
    return false;
}

/// Send quit message to all app processes
/// This is called when the core client exits,
/// or when a project is detached or reset
void ACTIVE_TASK_SET::request_tasks_exit(PROJECT* proj) {
    for (size_t i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (proj && atp->wup->project != proj) continue;
        if (!atp->process_exists()) continue;
        atp->request_exit();
    }
}

/// Send kill signal to all app processes
/// Don't wait for them to exit
void ACTIVE_TASK_SET::kill_tasks(PROJECT* proj) {
    for (size_t i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (proj && atp->wup->project != proj) continue;
        if (!atp->process_exists()) continue;
        atp->kill_task(false);
    }
}

/// Ask a process to stop executing (but stay in mem).
/// Done by sending it a <suspend> message.
///
/// \return Always returns 0.
int ACTIVE_TASK::suspend() {
    if (!app_client_shm.shm) return 0;
    if (task_state() != PROCESS_EXECUTING) {
        msg_printf(result->project, MSG_INFO, "Internal error: expected process %s to be executing", result->name);
    }
    int n = process_control_queue.msg_queue_purge("<resume/>");
    if (n == 0) {
        process_control_queue.msg_queue_send("<suspend/>", app_client_shm.shm->process_control_request);
    }
    set_task_state(PROCESS_SUSPENDED, "suspend");
    return 0;
}

/// Undo a suspend: send a <resume> message
///
/// \return Always returns 0.
int ACTIVE_TASK::unsuspend() {
    if (!app_client_shm.shm) return 0;
    if (task_state() != PROCESS_SUSPENDED) {
        msg_printf(result->project, MSG_INFO, "Internal error: expected process %s to be suspended", result->name);
    }
    if (log_flags.cpu_sched) {
        msg_printf(result->project, MSG_INFO, "[cpu_sched] Resuming %s", result->name);
    }
    int n = process_control_queue.msg_queue_purge("<suspend/>");
    if (n == 0) {
        process_control_queue.msg_queue_send("<resume/>", app_client_shm.shm->process_control_request);
    }
    set_task_state(PROCESS_EXECUTING, "unsuspend");
    return 0;
}

void ACTIVE_TASK::send_network_available() {
    if (!app_client_shm.shm) return;
    process_control_queue.msg_queue_send(
        "<network_available/>",
        app_client_shm.shm->process_control_request
    );
    return;
}

/// See if the app has placed a new message in shared mem
/// (with CPU done, frac done, etc).
/// If so parse it and return true.
bool ACTIVE_TASK::get_app_status_msg() {
    char msg_buf[MSG_CHANNEL_SIZE];

    if (!app_client_shm.shm) {
        msg_printf(result->project, MSG_INFO,
            "Task %s: no shared memory segment", result->name
        );
        return false;
    }
    if (!app_client_shm.shm->app_status.get_msg(msg_buf)) {
        return false;
    }
    if (log_flags.app_msg_receive) {
        msg_printf(this->wup->project, MSG_INFO,
            "[app_msg_receive] got msg from slot %d: %s", slot, msg_buf
        );
    }
    want_network = 0;
    current_cpu_time = checkpoint_cpu_time = 0.0;
    double fd;
    if (parse_double(msg_buf, "<fraction_done>", fd)) {
        // fraction_done will be reported as zero
        // until the app's first call to boinc_fraction_done().
        // So ignore zeros.
        //
        if (fd) fraction_done = fd;
    }
    parse_double(msg_buf, "<current_cpu_time>", current_cpu_time);
    parse_double(msg_buf, "<checkpoint_cpu_time>", checkpoint_cpu_time);
    parse_double(msg_buf, "<fpops_per_cpu_sec>", result->fpops_per_cpu_sec);
    parse_double(msg_buf, "<fpops_cumulative>", result->fpops_cumulative);
    parse_double(msg_buf, "<intops_per_cpu_sec>", result->intops_per_cpu_sec);
    parse_double(msg_buf, "<intops_cumulative>", result->intops_cumulative);
    parse_int(msg_buf, "<want_network>", want_network);
    if (current_cpu_time < 0) {
        msg_printf(result->project, MSG_INFO,
            "app reporting negative CPU: %f", current_cpu_time
        );
        current_cpu_time = 0;
    }
    if (checkpoint_cpu_time < 0) {
        msg_printf(result->project, MSG_INFO,
            "app reporting negative checkpoint CPU: %f", checkpoint_cpu_time
        );
        checkpoint_cpu_time = 0;
    }
    return true;
}

bool ACTIVE_TASK::get_trickle_up_msg() {
    char msg_buf[MSG_CHANNEL_SIZE];

    if (!app_client_shm.shm) return false;
    if (app_client_shm.shm->trickle_up.get_msg(msg_buf)) {
        if (match_tag(msg_buf, "<have_new_trickle_up/>")) {
            int retval = move_trickle_file();
            if (!retval) {
                wup->project->trickle_up_pending = true;
            }
        }
        if (match_tag(msg_buf, "<have_new_upload_file/>")) {
            handle_upload_files();
        }
        return true;
    }
    return false;
}

/// Check for msgs from active tasks.
void ACTIVE_TASK_SET::get_msgs() {
    for (size_t i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        double old_time = atp->checkpoint_cpu_time;
        if (atp->get_app_status_msg()) {
            if (old_time != atp->checkpoint_cpu_time) {
                gstate.request_enforce_schedule("Checkpoint reached");
                atp->checkpoint_wall_time = gstate.now;
                atp->premature_exit_count = 0;
                if (log_flags.task_debug) {
                    msg_printf(atp->wup->project, MSG_INFO,
                        "[task_debug] result %s checkpointed",
                        atp->result->name
                    );
                } else if (log_flags.checkpoint_debug) {
                    msg_printf(atp->wup->project, MSG_INFO,
                        "[checkpoint_debug] result %s checkpointed",
                        atp->result->name
                    );
                }
                atp->stats_checkpoint++;
                atp->write_task_state_file();
            }
        }
        atp->get_trickle_up_msg();
    }
}

/// Write checkpoint state to a file in the slot dir.
/// This avoids rewriting the state file on each checkpoint.
void ACTIVE_TASK::write_task_state_file() {
    std::ostringstream path;
    path << slot_dir << '/' << TASK_STATE_FILENAME;
    std::ofstream ofs(path.str().c_str());
    if (!ofs) {
        throw std::runtime_error(std::string("Can't open file: ") + path.str());
    }
    ofs << "<active_task>\n"
        << XmlTag<std::string>("project_master_url", result->project->get_master_url())
        << XmlTag<char*> ("result_name",         result->name)
        << XmlTag<double>("checkpoint_cpu_time", checkpoint_cpu_time)
        << "</active_task>\n";
}

/// Read the task state file in case it's more recent then the main state file.
/// Called on startup.
void ACTIVE_TASK::read_task_state_file() {
    std::ostringstream path;
    path << slot_dir << '/' << TASK_STATE_FILENAME;
    std::ifstream ifs(path.str().c_str());
    if (!ifs) {
        return;
    }
    std::string buf;
    std::string master_url;
    std::string result_name;
    double new_checkpoint_cpu_time = -1.0;
    while (std::getline(ifs, buf)) {
        if (parse_str(buf.c_str(), "<project_master_url>", master_url)) {
            continue;
        } else if (parse_str(buf.c_str(), "<result_name>", result_name)) {
            continue;
        } else {
            parse_double(buf.c_str(), "<checkpoint_cpu_time>", new_checkpoint_cpu_time);
        }
    }
    
    // sanity checks - project and result name must match 
    if ((master_url != result->project->get_master_url()) || (result_name != result->name)) {
        return;
    }
    if (new_checkpoint_cpu_time > checkpoint_cpu_time) { 
        checkpoint_cpu_time = new_checkpoint_cpu_time; 
    } 
} 
