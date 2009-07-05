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

#ifndef TASK_H_INCLUDED
#define TASK_H_INCLUDED

#include <cstdio>
#include <string>
#include <vector>

#include "common_defs.h"
#include "app_ipc.h"
#include "procinfo.h"

// forward declarations
// (we don't need to include the full declarations from client_types.h)
class CLIENT_STATE;
class PROJECT;
class WORKUNIT;
class RESULT;
class APP_VERSION;
class FILE_REF;
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
private:
    TASK_STATE _task_state;

    /// Determines if everything is set up for starting the science application
    /// including all links in the slot directory.
    bool full_init_done;

    /// Directory where process runs (relative).
    std::string slot_dir;

    /// Directory where process runs (absolute).
    /// This is used only to run graphics apps
    /// (that way don't have to worry about top-level dirs
    /// being non-readable, etc).
    std::string slot_path;

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
    inline TASK_STATE task_state() const {
        return _task_state;
    }
    void set_task_state(TASK_STATE val, const char* where);
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
    int current_disk_usage(double& size) const;

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

    // Info related to app's graphics mode (win, screensaver, etc.).
    int graphics_mode_acked;            ///< Mode acked by app.
    int graphics_mode_before_ss;        ///< Mode before last screensaver request.
    double graphics_mode_ack_timeout;

    // Statistics collection (not used for anything else)
    double stats_mem;               ///< Max size of the working set.
    double stats_page;              ///< Max size of the page file.
    double stats_pagefault_rate;    ///< Max page fault rate.
    double stats_disk;              ///< Max size of the working directory.
    int stats_checkpoint;           ///< Number of checkpoints.

#if (defined (__APPLE__) && (defined(__i386__) || defined(__x86_64__)))
    // PowerPC apps emulated on i386 Macs crash if running graphics
    int powerpc_emulated_on_i386;
    int is_native_i386_app(const char* exec_path) const;
#endif
    GRAPHICS_MSG graphics_msg;
    void request_graphics_mode(GRAPHICS_MSG& msg);
    int request_reread_prefs();
    int request_reread_app_info();
    void check_graphics_mode_ack();
    int link_user_files();

    /// Make a unique key for core/app shared memory segment.
    int get_shmem_seg_name();
    bool runnable() const {
        return _task_state == PROCESS_UNINITIALIZED
            || _task_state == PROCESS_EXECUTING
            || _task_state == PROCESS_SUSPENDED;
    }

    ACTIVE_TASK();
    ~ACTIVE_TASK();
    int init(RESULT* rp);
    void cleanup_task();

    /// Start a process.
    int start();

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
    int abort_task(int exit_status, const char* msg);

    /// Return true if this task has exited.
    bool has_task_exited();

    /// Preempt (via suspend or quit) a running task.
    int preempt(bool quit_task);

    /// Resume the task if it was previously running; otherwise start it.
    int resume_or_start(bool first_time);
    void send_network_available();
#ifdef _WIN32
    void handle_exited_app(unsigned long exit_code);
#else
    void handle_exited_app(int stat);
#endif

    /// Handle a task that exited prematurely (i.e. the job isn't done).
    void handle_premature_exit(bool& will_restart);

    bool check_max_disk_exceeded();

    bool get_app_status_msg();
    bool get_trickle_up_msg();
    double est_cpu_time_to_completion() const;
    bool read_stderr_file();
    bool finish_file_present() const;
    bool supports_graphics() const;
    
    /// Return the slot directory (relative path).
    std::string get_slot_dir() const;

    /// Write the app init file.
    int write_app_init_file();

    /// Move a trickle file from the slot directory to the project directory.
    int move_trickle_file();

    /// Check if everything was initialized before.
    bool is_full_init_done() const;

    int handle_upload_files();
    void upload_notify_app(const FILE_INFO* fip, const FILE_REF* frp);
    int copy_output_files();

    int write(MIOFILE& fout) const;
    int write_gui(MIOFILE& fout) const;
    int parse(MIOFILE& fin);
};
typedef std::vector<ACTIVE_TASK*> ACTIVE_TASK_PVEC;

class ACTIVE_TASK_SET {
public:
    ACTIVE_TASK_PVEC active_tasks;
    ACTIVE_TASK* lookup_pid(int pid);
    ACTIVE_TASK* lookup_result(const RESULT* result);
    void init();
    bool poll();

    /// Suspend all currently running tasks.
    void suspend_all(bool cpu_throttle);

    void unsuspend_all();
    bool is_task_executing();
    void request_tasks_exit(PROJECT* p=0);
    int wait_for_exit(double, PROJECT* p=0);
    int exit_tasks(PROJECT* p=0);
    void kill_tasks(PROJECT* p=0);
    int abort_project(PROJECT* project);
    bool get_msgs();

    /// See if any processes have exited.
    bool check_app_exited();
    bool check_rsc_limits_exceeded();
    bool check_quit_timeout_exceeded();
    bool is_slot_in_use(int slot) const;
    bool is_slot_dir_in_use(const std::string& dir) const;
    int get_free_slot() const;
    void send_heartbeats();
    void send_trickle_downs();
    void report_overdue() const;
    void handle_upload_files();
    void upload_notify_app(const FILE_INFO* fip);

    /// Does any task want network?
    bool want_network() const;

    /// Notify tasks that network is available.
    void network_available();
    void free_mem();
    bool slot_taken(int slot) const;
    void get_memory_usage();

    // graphics-related functions
    void graphics_poll();
    void process_control_poll();
    void request_reread_prefs(PROJECT* project);
    void request_reread_app_info();

    int write(MIOFILE& fout) const;
    int parse(MIOFILE& fin);
};

#endif // TASK_H_INCLUDED
