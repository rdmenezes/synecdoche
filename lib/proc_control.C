// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 University of California
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
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/// \file
/// Code to run a BOINC application (main or graphics) under Windows
/// Don't include this in applications

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif
#include <string>
#include "win_util.h"

#include "proc_control.h"

#include "filesys.h"
#include "error_numbers.h"
#include "common_defs.h"
#include "util.h"
#include "str_util.h"
#include "parse.h"
#include "base64.h"

using std::string;

HANDLE sandbox_account_interactive_token = NULL;
HANDLE sandbox_account_service_token = NULL;

void get_sandbox_account_interactive_token() {
    FILE* f;
    char buf[256];
    string encoded_username_str;
    string encoded_password_str;
    string username_str;
    string domainname_str; 
    string password_str; 
    int retval = 0;
    static bool first=true;
    PSID sandbox_account_sid = NULL;

    if (!first) return;
    first = false;

    f = fopen(CLIENT_AUTH_FILENAME, "r");
    if (!f) return;
    while (fgets(buf, 256, f)) {
        if (parse_str(buf, "<username>", encoded_username_str)) continue;
        if (parse_str(buf, "<password>", encoded_password_str)) continue;
    }
    fclose(f);

    password_str = r_base64_decode(encoded_password_str); 

    if (string::npos != encoded_username_str.find('\\')) {
        domainname_str = encoded_username_str.substr(
            0, encoded_username_str.find('\\')
        );
        username_str = encoded_username_str.substr(
            encoded_username_str.rfind(_T('\\')) + 1,
            encoded_username_str.length() - encoded_username_str.rfind(_T('\\')) - 1
        );
        retval = LogonUser( 
            (char*) username_str.c_str(),
            (char*) domainname_str.c_str(), 
            (char*) password_str.c_str(), 
            LOGON32_LOGON_INTERACTIVE, 
            LOGON32_PROVIDER_DEFAULT, 
            &sandbox_account_interactive_token
        );
        if (retval) {
            GetAccountSid(domainname_str.c_str(), username_str.c_str(), &sandbox_account_sid);
        }
    } else {
        username_str = encoded_username_str;
        retval = LogonUser( 
            (char*) username_str.c_str(),
            NULL, 
            (char*) password_str.c_str(), 
            LOGON32_LOGON_INTERACTIVE, 
            LOGON32_PROVIDER_DEFAULT, 
            &sandbox_account_interactive_token
        );
        if (retval) {
            GetAccountSid(NULL, username_str.c_str(), &sandbox_account_sid);
        }
    }

    if (!retval) {
        sandbox_account_interactive_token = NULL;
        sandbox_account_sid = NULL;
    } else {
        // Adjust the permissions on the current desktop and window station
        //   to allow the sandbox user account to create windows and such.
        //
        if (!AddAceToWindowStation(GetProcessWindowStation(), sandbox_account_sid)) {
            fprintf(stderr, "Failed to add ACE to current WindowStation\n");
        }
        if (!AddAceToDesktop(GetThreadDesktop(GetCurrentThreadId()), sandbox_account_sid)) {
            fprintf(stderr, "Failed to add ACE to current Desktop\n");
        }
    }
}

void get_sandbox_account_service_token() {
    FILE* f;
    char buf[256];
    string encoded_username_str;
    string encoded_password_str;
    string username_str;
    string domainname_str; 
    string password_str; 
    int retval = 0;
    static bool first=true;

    if (!first) return;
    first = false;

    f = fopen(CLIENT_AUTH_FILENAME, "r");
    if (!f) return;
    while (fgets(buf, 256, f)) {
        if (parse_str(buf, "<username>", encoded_username_str)) continue;
        if (parse_str(buf, "<password>", encoded_password_str)) continue;
    }
    fclose(f);

    password_str = r_base64_decode(encoded_password_str); 

    if (string::npos != encoded_username_str.find('\\')) {
        domainname_str = encoded_username_str.substr(
            0, encoded_username_str.find('\\')
        );
        username_str = encoded_username_str.substr(
            encoded_username_str.rfind(_T('\\')) + 1,
            encoded_username_str.length() - encoded_username_str.rfind(_T('\\')) - 1
        );
        retval = LogonUser( 
            (char*) username_str.c_str(),
            (char*) domainname_str.c_str(), 
            (char*) password_str.c_str(), 
            LOGON32_LOGON_SERVICE, 
            LOGON32_PROVIDER_DEFAULT, 
            &sandbox_account_service_token
        );
    } else {
        username_str = encoded_username_str;
        retval = LogonUser( 
            (char*) username_str.c_str(),
            NULL, 
            (char*) password_str.c_str(), 
            LOGON32_LOGON_SERVICE, 
            LOGON32_PROVIDER_DEFAULT, 
            &sandbox_account_service_token
        );
    }

    if (!retval) {
        sandbox_account_service_token = NULL;
    }
}


// CreateEnvironmentBlock
typedef BOOL (WINAPI *tCEB)(LPVOID *lpEnvironment, HANDLE hToken, BOOL bInherit);
// DestroyEnvironmentBlock
typedef BOOL (WINAPI *tDEB)(LPVOID lpEnvironment);

/// Run application, Windows.
/// chdir into the given directory, and run a program there.
/// argv is set up Unix-style, i.e. argv[0] is the program name
int run_app_windows(
    const char* dir, const char* file, int argc, char *const argv[], HANDLE& id
) {
    int retval;
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    LPVOID environment_block = NULL;
    char cmdline[1024];
    char error_msg[1024];

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    strcpy(cmdline, "");
    for (int i=0; i<argc; i++) {
        strcat(cmdline, argv[i]);
        if (i<argc-1) {
            strcat(cmdline, " ");
        }
    }

    get_sandbox_account_interactive_token();
    if (sandbox_account_interactive_token != NULL) {

        // Find CreateEnvironmentBlock/DestroyEnvironmentBlock pointers
        tCEB    pCEB = NULL;
        tDEB    pDEB = NULL;
        HMODULE hUserEnvLib = NULL;

        hUserEnvLib = LoadLibrary("userenv.dll");
        if (hUserEnvLib) {
            pCEB = (tCEB) GetProcAddress(hUserEnvLib, "CreateEnvironmentBlock");
            pDEB = (tDEB) GetProcAddress(hUserEnvLib, "DestroyEnvironmentBlock");
        }


        // Retrieve the current window station and desktop names
        char szWindowStation[256];
        memset(szWindowStation, 0, sizeof(szWindowStation));
        char szDesktop[256];
        memset(szDesktop, 0, sizeof(szDesktop));
        char szDesktopName[512];
        memset(szDesktopName, 0, sizeof(szDesktopName));

        if (!GetUserObjectInformation(
                GetProcessWindowStation(),
                UOI_NAME,
                &szWindowStation,
                sizeof(szWindowStation),
                NULL)
            ) {
            windows_error_string(error_msg, sizeof(error_msg));
            fprintf(stderr, "GetUserObjectInformation failed: %s\n", error_msg);
        }
        if (!GetUserObjectInformation(
                GetThreadDesktop(GetCurrentThreadId()),
                UOI_NAME,
                &szDesktop,
                sizeof(szDesktop),
                NULL)
            ) {
            windows_error_string(error_msg, sizeof(error_msg));
            fprintf(stderr, "GetUserObjectInformation failed: %s\n", error_msg);
        }

        // Construct the destination desktop name
        strncat(szDesktopName, szWindowStation, sizeof(szDesktopName) - strlen(szDesktopName));
        strncat(szDesktopName, "\\", sizeof(szDesktopName) - strlen(szDesktopName));
        strncat(szDesktopName, szDesktop, sizeof(szDesktopName) - strlen(szDesktopName));

        // Tell CreateProcessAsUser which desktop to use explicitly.
        startup_info.lpDesktop = szDesktopName;
                 
        // Construct an environment block that contains environment variables that don't
        //   describe the current user.
        if (!pCEB(&environment_block, sandbox_account_interactive_token, FALSE)) {
            windows_error_string(error_msg, sizeof(error_msg));
            fprintf(stderr, "CreateEnvironmentBlock failed: %s\n", error_msg);
        }

        retval = CreateProcessAsUser( 
            sandbox_account_interactive_token, 
            file, 
            cmdline, 
            NULL, 
            NULL, 
            FALSE, 
            CREATE_NEW_PROCESS_GROUP|CREATE_UNICODE_ENVIRONMENT, 
            environment_block, 
            dir, 
            &startup_info, 
            &process_info 
        );

        if (!pDEB(environment_block)) {
            windows_error_string(error_msg, sizeof(error_msg));
            fprintf(stderr, "DestroyEnvironmentBlock failed: %s\n", error_msg);
        }

        if (hUserEnvLib) {
            pCEB = NULL;
            pDEB = NULL;
            FreeLibrary(hUserEnvLib);
        }
    } else {
        retval = CreateProcess(
            file,
            cmdline,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            dir,
            &startup_info,
            &process_info
        );
    }
    if (!retval) {
        windows_error_string(error_msg, sizeof(error_msg));
        fprintf(stderr, "CreateProcess failed: '%s'\n", error_msg);
        return -1; // CreateProcess returns 1 if successful, false if it failed.
    }

    id = process_info.hProcess;
    return 0;
}
