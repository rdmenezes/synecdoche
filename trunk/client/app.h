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

#ifndef _TASK_
#define _TASK_

#include <cstdio>
#include <vector>

#include "common_defs.h"
#include "app_ipc.h"
#include "procinfo.h"

//forward declarations
//(we don't need to include the full declarations from client_state.h)
class CLIENT_STATE;
class PROJECT;
struct WORKUNIT;
struct RESULT;
struct APP_VERSION;
struct FILE_REF;
class FILE_INFO;

typedef int PROCESS_ID;

/// The stderr output of an application is truncated to this length
/// before sending to server,
/// to protect against apps that write unbounded amounts.
#define MAX_STDERR_LEN  65536


/// Represents a task in progress.
///
/// "CPU time" refers to the sum over all episodes.
/// (not counting the "lost" time after the last checkpoint
/// in episodes before the current one)
///
/// When an active task is created, it is assigned a "slot"
/// which determines the directory it runs in.
/// This doesn't change over the life of the active task;
/// thus the task can use the slot directory for temp files
/// that aren't tracked directly.
class ACTIVE_TASK {
    int _task_state;
public:
#ifdef _WIN32
    HANDLE pid_handle, shm_handle;
    bool kill_all_children();
#endif
    SHMEM_SEG_NAME shmem_seg_name;
    RESULT* result;
    WORKUNIT* wup;
    APP_VERSION* app_version;
    PROCESS_ID pid;
    PROCINFO procinfo;

    int slot;   ///< subdirectory of slots/ where this runs.
    inline int task_state() const {
        return _task_state;
    }
    void set_task_state(int, const char*);
    int scheduler_state;
    int next_scheduler_state; // temp
    int signal;

    /// App's estimate of how much of the work unit is done.
    /// Passed from the application via an API call;
    /// will be zero if the app doesn't use this call.
    double fraction_done;

    /// CPU time when adjust_debts() last ran.
    double debt_interval_start_cpu_time;

    /// CPU time at the start of current episode.
    double episode_start_cpu_time;

    /// Wall time at the start of the current run interval.
    double run_interval_start_wall_time;

    /// CPU at the last checkpoint.
    double checkpoint_cpu_time;

    /// Wall time at the last checkpoint.
    double checkpoint_wall_time;

    /// Most recent CPU time reported by app.
    double current_cpu_time;

    /// Disk used by output files and temp files of this task.
    int current_disk_usage(double&) const;

    /// Directory where process runs (relative).
    char slot_dir[256];

    /// Directory where process runs (absolute).
    /// This is used only to run graphics apps
    /// (that way don't have to worry about top-level dirs
    /// being non-readable, etc).
    char slot_path[512];

    double max_cpu_time;    ///< Abort if total CPU exceeds this.
    double max_disk_usage;  ///< Abort if disk usage (in+out+temp) exceeds this.
    double max_mem_usage;   ///< Abort if memory usage exceeds this.
    bool have_trickle_down;
    bool send_upload_file_status;
    bool too_large;                 ///< Working set too large to run now.
    bool needs_shmem;               ///< Waiting for a free shared memory segment.

    /// This task wants to do network comm.
    /// This is passed via share-memory message (app_status channel).
    int want_network;

    /// When we sent an abort message to this app
    /// kill it 5 seconds later if it doesn't exit.
    double abort_time;

    /// When we sent a quit message; kill if still there after 10 sec.
    double quit_time;
    int premature_exit_count;

    APP_CLIENT_SHM app_client_shm;        ///< Core/app shared mem.
    MSG_QUEUE graphics_request_queue;
    MSG_QUEUE process_control_queue;

    /// Info related to app's graphics mode (win, screensaver, etc.).
    int graphics_mode_acked;            ///< Mode acked by app.
    int graphics_mode_before_ss;        ///< Mode before last screensaver request.
    double graphics_mode_ack_timeout;

#if (defined (__APPLE__) && (defined(__i386__) || defined(__x86_64__)))
    // PowerPC apps emulated on i386 Macs crash if running graphics
    int powerpc_emulated_on_i386;
    int is_native_i386_app(char*);
#endif
    GRAPHICS_MSG graphics_msg;
    void request_graphics_mode(GRAPHICS_MSG&);
    int request_reread_prefs();
    int request_reread_app_info();
    void check_graphics_mode_ack();
    int link_user_files();

    /// Make a unique key for core/app shared memory segment.
    int get_shmem_seg_name();
    bool runnable() {
        return _task_state == PROCESS_UNINITIALIZED
            || _task_state == PROCESS_EXECUTING
            || _task_state == PROCESS_SUSPENDED;
    }

    ACTIVE_TASK();
    ~ACTIVE_TASK();
    int init(RESULT*);
    void close_process_handles();
    void cleanup_task();

    /// Start a process.
    int start(bool first_time);

    /// Ask the process to exit gracefully.
    int request_exit();

    /// Send "abort" message.
    int request_abort();
    bool process_exists();

    /// Kill process forcibly.
    int kill_task(bool restart);

    /// Ask a process to stop executing.
    int suspend();

    /// Undo a suspend.
    int unsuspend();

    /// Abort a task.
    int abort_task(int exit_status, const char*);

    /// Return true if this task has exited.
    bool has_task_exited();

    /// Preempt (via suspend or quit) a running task.
    int preempt(bool quit_task);

    /// Resume the task if it was previously running; otherwise start it.
    int resume_or_start(bool);
    void send_network_available();
#ifdef _WIN32
    void handle_exited_app(unsigned long);
#else
    void handle_exited_app(int stat);
#endif

    /// Handle a task that exited prematurely (i.e. the job isn't done).
    void handle_premature_exit(bool&);

    bool check_max_disk_exceeded();

    bool get_app_status_msg();
    bool get_trickle_up_msg();
    double est_cpu_time_to_completion();
    bool read_stderr_file();
    bool finish_file_present();
    bool supports_graphics() const;

    /// Write the app init file.
    int write_app_init_file();
    int move_trickle_file();
    int handle_upload_files();
    void upload_notify_app(const FILE_INFO*, const FILE_REF*);
    int copy_output_files();

    int write(MIOFILE&) const;
    int write_gui(MIOFILE&) const;
    int parse(MIOFILE&);
};

class ACTIVE_TASK_SET {
public:
    typedef std::vector<ACTIVE_TASK*> active_tasks_v;
    active_tasks_v active_tasks;
    ACTIVE_TASK* lookup_pid(int);
    ACTIVE_TASK* lookup_result(const RESULT*);
    void init();
    bool poll();
    void suspend_all(bool leave_apps_in_memory=true);
    void unsuspend_all();
    bool is_task_executing();
    void request_tasks_exit(PROJECT* p=0);
    int wait_for_exit(double, PROJECT* p=0);
    int exit_tasks(PROJECT* p=0);
    void kill_tasks(PROJECT* p=0);
    int abort_project(PROJECT*);
    bool get_msgs();

    /// See if any processes have exited.
    bool check_app_exited();
    bool check_rsc_limits_exceeded();
    bool check_quit_timeout_exceeded();
    bool is_slot_in_use(int) const;
    bool is_slot_dir_in_use(const char*) const;
    int get_free_slot() const;
    void send_heartbeats();
    void send_trickle_downs();
    void report_overdue() const;
    void handle_upload_files();
    void upload_notify_app(const FILE_INFO*);

    /// Does any task want network?
    bool want_network() const;

    /// Notify tasks that network is available.
    void network_available();
    void free_mem();
    bool slot_taken(int) const;
    void get_memory_usage();

    // graphics-related functions
    void graphics_poll();
    void process_control_poll();
    void request_reread_prefs(PROJECT*);
    void request_reread_app_info();

    int write(MIOFILE&) const;
    int parse(MIOFILE&);
};

#endif
