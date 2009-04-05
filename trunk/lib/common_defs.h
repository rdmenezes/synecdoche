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

#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

/// \file
/// \#defines or constants or enums that are shared by more than one BOINC component
/// (e.g. client, server, Manager, etc.)

const double KILO = 1024.0;
const double MEGA = 1024.0 * KILO;
const double GIGA = 1024.0 * MEGA;

const int GUI_RPC_PORT = 31416;

/// \name Run mode
/// @{
#define RUN_MODE_ALWAYS 1
#define RUN_MODE_AUTO   2
#define RUN_MODE_NEVER  3
/// restore permanent mode - used only in set_X_mode() GUI RPC
#define RUN_MODE_RESTORE    4
/// @}

/// \name Scheduler state
/// Values of ACTIVE_TASK::scheduler_state and ACTIVE_TASK::next_scheduler_state.
/// "SCHEDULED" is synonymous with "executing" except when CPU throttling
/// is in use.
/// @{
#define CPU_SCHED_UNINITIALIZED   0
#define CPU_SCHED_PREEMPTED       1
#define CPU_SCHED_SCHEDULED       2
/// @}

/// \name Official HTTP status codes
/// @{
#define HTTP_STATUS_CONTINUE                100
#define HTTP_STATUS_OK                      200
#define HTTP_STATUS_PARTIAL_CONTENT         206
#define HTTP_STATUS_MOVED_PERM              301
#define HTTP_STATUS_MOVED_TEMP              302
#define HTTP_STATUS_NOT_FOUND               404
#define HTTP_STATUS_PROXY_AUTH_REQ          407
#define HTTP_STATUS_RANGE_REQUEST_ERROR     416
#define HTTP_STATUS_INTERNAL_SERVER_ERROR   500
#define HTTP_STATUS_SERVICE_UNAVAILABLE     503
/// @}

/// \name Screensaver status
/// The core client can be requested to provide screensaver graphics (SSG).
/// The following are states of this function:

/// @{
/// requested to provide SSG
#define SS_STATUS_ENABLED                           1
/// not providing SSG, SS should blank screen
#define SS_STATUS_BLANKED                           3
/// not providing SS because suspended
#define SS_STATUS_BOINCSUSPENDED                    4
/// no apps executing
#define SS_STATUS_NOAPPSEXECUTING                   6
/// apps executing, but none graphical
#define SS_STATUS_NOGRAPHICSAPPSEXECUTING           7
/// not requested to provide SSG
#define SS_STATUS_QUIT                              8
#define SS_STATUS_NOPROJECTSDETECTED                9
/// SSG unsupported; client running as daemon
#define SS_STATUS_DAEMONALLOWSNOGRAPHICS            10
/// @}

/// \name graphics messages
/// @{
#define MODE_UNSUPPORTED        0
#define MODE_HIDE_GRAPHICS      1
#define MODE_WINDOW             2
#define MODE_FULLSCREEN         3
#define MODE_BLANKSCREEN        4
#define MODE_REREAD_PREFS       5
#define MODE_QUIT               6
#define NGRAPHICS_MSGS  7
/// @}

/// Message priorities
enum MSG_PRIORITY {
    /// Dummy value used for initialization.
    MSG_NO_PRIORITY = 0,

    /// Just a simple message, nothing that requires user intervention.
    /// write to stdout
    /// GUI: write to msg window
    MSG_INFO,

    /// Conditions that require user intervention;
    /// text should be user-friendly.
    /// write to stdout
    /// GUI: write to msg window in bold or red
    MSG_USER_ERROR,

    /// Conditions that indicate a problem or bug with BOINC itself,
    /// or with a BOINC project or account manager.
    /// treat same as MSG_INFO, but prepend with [error]
    MSG_INTERNAL_ERROR
};

/// bitmap defs for task_suspend_reason, network_suspend_reason
/// Note: doesn't need to be a bitmap, but keep for compatibility
enum SUSPEND_REASON {
    SUSPEND_REASON_BATTERIES        = 0x01,
    SUSPEND_REASON_USER_ACTIVE      = 0x02,
    SUSPEND_REASON_USER_REQ         = 0x04,
    SUSPEND_REASON_TIME_OF_DAY      = 0x08,
    SUSPEND_REASON_BENCHMARKS       = 0x10,
    SUSPEND_REASON_DISK_SIZE        = 0x20,
    SUSPEND_REASON_CPU_USAGE_LIMIT  = 0x40,
    SUSPEND_REASON_NO_RECENT_INPUT  = 0x80,
    SUSPEND_REASON_INITIAL_DELAY    = 0x100
};

/// \name Result states
/// States of a result on a client.
/// THESE MUST BE IN NUMERICAL ORDER
/// (because of the > comparison in RESULT::computing_done())
/// @{

/// New result
#define RESULT_NEW               0
/// Input files for result (WU, app version) are being downloaded
#define RESULT_FILES_DOWNLOADING 1
/// Files are downloaded, result can be (or is being) computed
#define RESULT_FILES_DOWNLOADED  2
/// Computation failed; no file upload
#define RESULT_COMPUTE_ERROR     3
/// Output files for result are being uploaded
#define RESULT_FILES_UPLOADING   4
/// Files are uploaded, notify scheduling server at some point
#define RESULT_FILES_UPLOADED    5
/// Result was aborted
#define RESULT_ABORTED           6
/// @}

/// Values of ACTIVE_TASK::task_state.
enum TASK_STATE {
    /// Process doesn't exist yet
    PROCESS_UNINITIALIZED = 0,

    /// Process is running, as far as we know
    PROCESS_EXECUTING,

    /// We've sent it a "suspend" message
    PROCESS_SUSPENDED,

    /// Process exceeded limits; send "abort" message, waiting to exit
    PROCESS_ABORT_PENDING,

    /// We've sent it a "quit" message, waiting to exit
    PROCESS_QUIT_PENDING,

    /// states in which the process has exited
    PROCESS_EXITED,
    PROCESS_WAS_SIGNALED,
    PROCESS_EXIT_UNKNOWN,

    /// aborted process has exited
    PROCESS_ABORTED,
    PROCESS_COULDNT_START
};

/// \name Network status
/// values of "network status"
/// @{
#define NETWORK_STATUS_ONLINE           0
#define NETWORK_STATUS_WANT_CONNECTION  1
#define NETWORK_STATUS_WANT_DISCONNECT  2
#define NETWORK_STATUS_LOOKUP_PENDING   3
/// @}

/// Reasons for making a scheduler RPC.
enum rpc_reason {
    NO_RPC_REASON = 0,
    RPC_REASON_USER_REQ,
    RPC_REASON_RESULTS_DUE,
    RPC_REASON_NEED_WORK,
    RPC_REASON_TRICKLE_UP,
    RPC_REASON_ACCT_MGR_REQ,
    RPC_REASON_INIT,
    RPC_REASON_PROJECT_REQ
};

struct VERSION_INFO {
    int major;
    int minor;
    int release;
    bool prerelease;
};

#ifdef _WIN32
#define RUN_MUTEX           "BoincSingleInstance"
#define REG_BLANK_NAME      "Blank"
#define REG_BLANK_TIME      "BlankTime"
#define REG_STARTUP_NAME    "BOINC"
#define CLIENT_AUTH_FILENAME    "client_auth.xml"
#else
#define LOCK_FILE_NAME      "lockfile"
#endif

#define GRAPHICS_APP_FILENAME "graphics_app"
#define ASSIGNED_WU_STR "asgn"
#define GUI_RPC_PASSWD_FILE "gui_rpc_auth.cfg"

#endif // COMMON_DEFS_H
