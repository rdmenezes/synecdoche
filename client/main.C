// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard, Peter Kortschack
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
/// Synecdoche daemon, also called the core client.
///
/// This file contains no GUI-related code.

#ifdef _WIN32
#define _CONSOLE 1
#include "boinc_win.h"
#include "win_service.h"
#include "win_util.h"

extern HINSTANCE g_hClientLibraryDll;
static bool requested_suspend = false;
static bool requested_resume = false;

typedef BOOL (CALLBACK* ClientLibraryStartup)();
typedef void (CALLBACK* ClientLibraryShutdown)();
#ifndef _T
#define _T(X) X
#endif

#else
#include "config.h"
#ifdef HAVE_SYS_SOCKET_H
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include <sys/stat.h>
#include <syslog.h>
#include <cstdlib>
#include <unistd.h>
#include <csignal>
#endif

#if (defined (__APPLE__) && defined(SANDBOX) && defined(_DEBUG))
#include "SetupSecurity.h"
#endif

#include <cstring>

#include "main.h"

#include "diagnostics.h"
#include "error_numbers.h"
#include "str_util.h"
#include "util.h"
#include "unix_util.h"
#include "prefs.h"
#include "filesys.h"
#include "network.h"

#include "client_state.h"
#include "file_names.h"
#include "log_flags.h"
#include "client_msgs.h"
#include "http_curl.h"
#include "sandbox.h"

#ifdef HARDCODED_DIRS
#include "dirs.h"
#endif

int finalize();

/// Used on Windows 95/98/ME to determine when it is safe to leave
/// the WM_ENDSESSION message handler and allow Windows to finish
/// cleaning up.
static bool boinc_cleanup_completed = false;

#ifdef _WIN32
// The following 3 functions are called in a separate thread,
// so we can't do anything directly.
// Set flags telling the main thread what to do.
//
void quit_client() {
    gstate.requested_exit = true;
    while (!boinc_cleanup_completed) {
        boinc_sleep(1.0);
    }
}

void suspend_client() {
    requested_suspend = true;
}

void resume_client() {
    requested_resume = true;
}

BOOL WINAPI ConsoleControlHandler( DWORD dwCtrlType ){
    BOOL bReturnStatus = FALSE;
    BOINCTRACE("***** Console Event Detected *****\n");
    switch( dwCtrlType ){
    case CTRL_LOGOFF_EVENT:
        BOINCTRACE("Event: CTRL-LOGOFF Event\n");
        if (!gstate.executing_as_daemon) {
           quit_client();
        }
        bReturnStatus =  TRUE;
        break;
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        BOINCTRACE("Event: CTRL-C or CTRL-BREAK Event\n");
        quit_client();
        bReturnStatus =  TRUE;
        break;
    case CTRL_CLOSE_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        BOINCTRACE("Event: CTRL-CLOSE or CTRL-SHUTDOWN Event\n");
        quit_client();
        break;
    }
    return bReturnStatus;
}
#else
static void signal_handler(int signum) {
    msg_printf(NULL, MSG_INFO, "Received signal %d", signum);
    switch(signum) {
    case SIGHUP:
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
#ifdef SIGPWR
    case SIGPWR:
#endif
        gstate.requested_exit = true;
        break;
    default:
        msg_printf(NULL, MSG_INTERNAL_ERROR, "Signal not handled");
    }
}
#endif

static void init_core_client(int argc, const char** argv) {
    setbuf(stdout, 0);
    setbuf(stderr, 0);

#ifdef HARDCODED_DIRS
    gstate.data_directory = DIRECTORIES::localstate;
    gstate.data_directory.append("/synecdoche");
#endif

    gstate.parse_cmdline(argc, argv);

    if (!gstate.data_directory.empty()) {
        if (chdir(gstate.data_directory.c_str()) < 0) {
            perror("chdir");
            exit(1);
        }
    }
#ifdef _WIN32
    if (!config.allow_multiple_clients) {
        chdir_to_data_dir();
    }
#endif

#ifndef _WIN32
    if (g_use_sandbox)
        // Set file creation mask to be writable by both user and group and
        // world-executable but neither world-readable nor world-writable
        // Our umask will be inherited by all our child processes
        //
        umask (6);
#endif

    // Initialize the BOINC Diagnostics Framework
    int flags =
        BOINC_DIAG_DUMPCALLSTACKENABLED |
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_TRACETOSTDOUT;

    if (gstate.redirect_io || gstate.executing_as_daemon || gstate.detach_console) {
        flags |=
            BOINC_DIAG_REDIRECTSTDERR |
            BOINC_DIAG_REDIRECTSTDOUT;
    }

    diagnostics_init(flags, "stdoutdae", "stderrdae");

    // Read config.
    read_config_file(true);

    // Until the config file has been read, log files are unbounded.
    diagnostics_set_max_file_sizes(config.max_stdout_file_size, config.max_stderr_file_size);

    // Win32 - detach from console if requested
#ifdef _WIN32
    if (gstate.detach_console) {
        FreeConsole();
    }
#endif

    // Unix: install signal handlers
#ifndef _WIN32
    // Handle quit signals gracefully
    boinc_set_signal_handler(SIGHUP, signal_handler);
    boinc_set_signal_handler(SIGINT, signal_handler);
    boinc_set_signal_handler(SIGQUIT, signal_handler);
    boinc_set_signal_handler(SIGTERM, signal_handler);
#ifdef SIGPWR
    boinc_set_signal_handler(SIGPWR, signal_handler);
#endif
#endif

    // Windows: install console controls
#ifdef _WIN32
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleControlHandler, TRUE)){
        if (!gstate.executing_as_daemon) {
            fprintf(stderr, "Failed to register the console control handler\n");
        } else {
            LogEventErrorMessage(TEXT("Failed to register the console control handler\n"));
        }
        exit(1);
    }
#endif
}

#ifdef _WIN32
    char event_message[2048];
#endif

int initialize() {
    int retval;

#ifdef _WIN32
    g_hClientLibraryDll = LoadLibrary("synecdoche.dll");
    if(!g_hClientLibraryDll) {
        stprintf(event_message,
            TEXT("Synecdoche Daemon Error Message\n"
                 "Failed to initialize the Synecdoche Client Library.\n"
                 "Load failed: %s\n"), windows_error_string(event_message, sizeof(event_message))
        );
        if (!gstate.executing_as_daemon) {
            fprintf(stderr, event_message);
        } else {
            LogEventErrorMessage(event_message);
        }
    }
#endif

    if (!config.allow_multiple_clients) {
        retval = wait_client_mutex(".", 10);
        if (retval) {
            fprintf(stderr,
                "Another instance of Synecdoche is running\n"
            );
#ifdef _WIN32
            if (!gstate.executing_as_daemon) {
                LogEventErrorMessage(
                    TEXT("Another instance of Synecdoche is running")
                );
            }
#endif
            return ERR_EXEC;
        }
    }


    // Initialize WinSock
#if defined(_WIN32) && defined(USE_WINSOCK)
    if ( WinsockInitialize() != 0 ) {
        if (!gstate.executing_as_daemon) {
            fprintf(stderr,
                TEXT("Synecdoche Daemon Error Message\n"
                     "Failed to initialize the Windows Sockets interface\n"
                     "Terminating Application...\n")
            );
        } else {
            LogEventErrorMessage(
                TEXT("Synecdoche Daemon Error Message\n"
                     "Failed to initialize the Windows Sockets interface\n"
                     "Terminating Application...\n")
            );
        }
        return ERR_IO;
    }
#endif

    curl_init();

#ifdef _WIN32
    if(g_hClientLibraryDll) {
        ClientLibraryStartup fnClientLibraryStartup;
        fnClientLibraryStartup = (ClientLibraryStartup)GetProcAddress(g_hClientLibraryDll, _T("ClientLibraryStartup"));
        if(fnClientLibraryStartup) {
            if(!fnClientLibraryStartup()) {
                stprintf(event_message,
                    TEXT("Synecdoche Daemon Error Message\n"
                        "Failed to initialize the Synecdoche Idle Detection Interface.\n"
                        "Synecdoche will not be able to determine if the user is idle or not...\n"
                        "Load failed: %s\n"), windows_error_string(event_message, sizeof(event_message))
                );
                if (!gstate.executing_as_daemon) {
                    fprintf(stderr, event_message);
                } else {
                    LogEventErrorMessage(event_message);
                }
            }
        }
    }
#endif
    return 0;
}

int boinc_main_loop() {
    int retval;

    retval = initialize();
    if (retval) return retval;

    retval = gstate.init();
    if (retval) {
        fprintf(stderr, "gstate.init() failed: %d\n", retval);
#ifdef _WIN32
        if (gstate.executing_as_daemon) {
            stprintf(event_message, TEXT("gstate.init() failed: %d\n"), retval);
            LogEventErrorMessage(event_message);
        }
#endif
        return retval;
    }

#ifdef _WIN32
    if (gstate.executing_as_daemon) {
        LogEventInfoMessage(
            TEXT("Synecdoche service is initialization completed, now begining process execution...\n")
        );
    }
#endif

    // must parse env vars after gstate.init();
    // otherwise items will get overwritten with state file info
    //
    gstate.parse_env_vars();

    if (gstate.projects.empty()) {
        msg_printf(NULL, MSG_INFO,
            "This computer is not attached to any projects"
        );
    }

    while (1) {
        if (!gstate.poll_slow_events()) {
            gstate.do_io_or_sleep(POLL_INTERVAL);
        }
        fflush(stdout);

        if (gstate.time_to_exit()) {
            msg_printf(NULL, MSG_INFO, "Time to exit");
            break;
        }
        if (gstate.requested_exit) {
            msg_printf(NULL, MSG_INFO, "Exit requested by user");
            break;
        }
#ifdef _WIN32
        if (requested_suspend) {
            gstate.run_mode.set(RUN_MODE_NEVER, 3600.0);
            gstate.network_mode.set(RUN_MODE_NEVER, 3600.0);
            requested_suspend = false;
        }
        if (requested_resume) {
            gstate.run_mode.set(RUN_MODE_RESTORE, 0);
            gstate.network_mode.set(RUN_MODE_RESTORE, 0);
            requested_resume = false;
        }
#endif
    }
    return finalize();
}

int finalize() {
    static bool finalized = false;
    if (finalized) return 0;
    finalized = true;
    gstate.quit_activities();

#ifdef _WIN32
    if(g_hClientLibraryDll) {
        ClientLibraryShutdown fnClientLibraryShutdown;
        fnClientLibraryShutdown = (ClientLibraryShutdown)GetProcAddress(g_hClientLibraryDll, _T("ClientLibraryShutdown"));
        if(fnClientLibraryShutdown) {
            fnClientLibraryShutdown();
        }
        if(!FreeLibrary(g_hClientLibraryDll)) {
            stprintf(event_message,
                TEXT("Synecdoche Daemon Error Message\n"
                    "Failed to cleanup the Synecdoche Idle Detection Interface\n"
                    "Unload failed: %s\n"
                ),
                windows_error_string(event_message, sizeof(event_message))
            );
            if (!gstate.executing_as_daemon) {
                fprintf(stderr, event_message);
            } else {
                LogEventErrorMessage(event_message);
            }
        }
        g_hClientLibraryDll = NULL;
    }

#ifdef USE_WINSOCK
    if (WinsockCleanup()) {
        stprintf(event_message,
            TEXT("Synecdoche Daemon Error Message\n"
                "Failed to cleanup the Windows Sockets interface\n"
                "Unload failed: %s\n"
            ),
            windows_error_string(event_message, sizeof(event_message))
        );
        if (!gstate.executing_as_daemon) {
            fprintf(stderr, event_message);
        } else {
            LogEventErrorMessage(event_message);
        }
        return ERR_IO;
    }
#endif

#endif

    curl_cleanup();
    boinc_cleanup_completed = true;
    return 0;
}

/// Entry point.
///
/// \todo Clean up the Windows-specific initialization code.
int main(int argc, const char** argv) {
    int retval = 0;

#ifdef _WIN32
    int i;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // Allow the system to know it is running as a Windows service
    // and adjust its diagnostics schemes accordingly.
    if ( (argc > 1) && ((*argv[1] == '-') || (*argv[1] == '/')) ) {
        if ( stricmp( "daemon", argv[1]+1 ) == 0 ) {
            gstate.executing_as_daemon = true;
            LogEventInfoMessage(
                TEXT("Synecdoche service is initializing...\n")
            );
        }
    }

    // This bit of silliness is required to properly detach when run from within a command
    // prompt under Win32.  The root cause of the problem is that CMD.EXE does not return
    // control to the user until the spawned program exits, detaching from the console is
    // not enough.  So we need to do the following.  If the -detach flag is given, trap it
    // prior to the main setup in init_core_client.  Reinvoke the program, changing the
    // -detach into -detach_phase_two, and then exit.  At this point, cmd.exe thinks all is
    // well, and returns control to the user.  Meanwhile the second invocation will grok the
    // -detach_phase_two flag, and detach itself from the console, finally getting us to
    // where we want to be.
    for (i = 1; i < argc; i++) {
        // FIXME FIXME.  Duplicate instances of -detach may cause this to be
        // executed unnecessarily.  At worst, I think it leads to a few extra
        // processes being created and destroyed.
        if (strcmp(argv[i], "-detach") == 0 || strcmp(argv[i], "--detach") == 0) {
            argv[i] = "-detach_phase_two";

            std::string commandLine;

            commandLine.append("\"").append(argv[0]).append("\"");
            for (i = 1; i < argc; i++) {
                commandLine.append(" ").append(argv[i]);
            }

            memset(&si, 0, sizeof(si));
            si.cb = sizeof(si);

            // CreateProcess may modify lpCommandLine
            char* lpCommandLine = new char[commandLine.length() + 1];
            strcpy(lpCommandLine, commandLine.c_str());

            // If process creation succeeds, we exit, if it fails punt and continue
            // as usual.  We won't detach properly, but the program will run.
            if (CreateProcess(NULL, lpCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                exit(0);
            }
            delete[] lpCommandLine;
            break;
        }
    }
#else
    // non-Apple Unix
    int i;

    for (i=1; i<argc; i++) {
        if (strcmp(argv[i], "-daemon") == 0 || strcmp(argv[i], "--daemon") == 0) {
            syslog(LOG_DAEMON|LOG_INFO,
                "Starting BOINC as daemon, listening on port %d.", GUI_RPC_PORT
            );
            // from <unistd.h>:
            // Detach from the controlling terminal and run in the background as system daemon.
            // Don't change working directory to root ("/"), but redirect
            // standard input, standard output and standard error to /dev/null.
            retval = daemon(1, 0);
            break;
        }
    }
#endif

    init_core_client(argc, argv);

#ifdef _WIN32

    SERVICE_TABLE_ENTRY dispatchTable[] = {
        { TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)service_main },
        { NULL, NULL }
    };

    if ( (argc > 1) && ((*argv[1] == '-') || (*argv[1] == '/')) ) {
        if ( stricmp( "daemon", argv[1]+1 ) == 0 ) {

            printf( "\nStartServiceCtrlDispatcher being called.\n" );
            printf( "This may take several seconds.  Please wait.\n" );

            if (!StartServiceCtrlDispatcher(dispatchTable)) {
                LogEventErrorMessage(TEXT("StartServiceCtrlDispatcher failed."));
            }
        } else {
            retval = boinc_main_loop();
        }
    } else {
        retval = boinc_main_loop();
    }
#else

#ifdef SANDBOX
    // Make sure owners, groups and permissions are correct
    // for the current setting of g_use_sandbox
    //
#if defined(_DEBUG) && defined(__APPLE__)
    // GDB can't attach to applications which are running as a different user
    // or group, so fix up data with current user and group during debugging
    //
    if (check_security(g_use_sandbox, false)) {
        SetBOINCDataOwnersGroupsAndPermissions();
    }
#endif  // _DEBUG && __APPLE__
    i = check_security(g_use_sandbox, false);
    if (i) {
        printf(
            "File ownership or permissions are set in a way that\n"
            "does not allow sandboxed execution of Synecdoche applications.\n"
            "To use BOINC anyway, use the -insecure command line option.\n"
            "To change ownership/permission, reinstall Synecdoche"
#ifdef __APPLE__
            " or run\n the shell script Mac_SA_Secure.sh"
#else
            " or run\n the shell script secure.sh"
#endif
            ". (Error code %d)\n", i
        );
        return ERR_USER_PERMISSION;
    }
#endif  // SANDBOX

    retval = boinc_main_loop();
#endif

    return retval;
}
