// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Nicolas Alvarez
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

/// \file
/// Determine which platforms are supported and provide a way
/// of exposing that information to the rest of the client.

#ifdef _WIN32
#include "boinc_win.h"
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;
#endif

#ifndef _WIN32
#include "config.h"
#endif

#include "client_state.h"

#include <cstdio>

#include <string>
#include <ostream>

#include "client_types.h"
#include "error_numbers.h"
#include "log_flags.h"
#include "str_util.h"
#include "util.h"
#include "xml_write.h"

/// return the primary platform id.
std::string CLIENT_STATE::get_primary_platform() const {
    return platforms[0].name;
}


/// add a platform to the vector.
void CLIENT_STATE::add_platform(const char* platform) {
    PLATFORM pp;
    pp.name = platform;
    platforms.push_back(pp);
}


/// determine the list of supported platforms.
void CLIENT_STATE::detect_platforms() {

#if defined(_WIN32) && !defined(__CYGWIN32__)
#if defined(_WIN64) && defined(_M_X64)

    add_platform("windows_x86_64");
    add_platform("windows_intelx86");

#else
    // see if 32-bit client is running on 64-bit machine 
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process"); 
    if (fnIsWow64Process) { 
        BOOL bIsWow64 = FALSE; 
        if (fnIsWow64Process(GetCurrentProcess(), &bIsWow64)) { 
            if (bIsWow64) { 
                add_platform("windows_x86_64"); 
            } 
        } 
    } 
    add_platform("windows_intelx86");
#endif

#elif defined(__APPLE__)
#if defined(__x86_64__)

    add_platform("x86_64-apple-darwin");

#endif

#if defined(__i386__) || defined(__x86_64__)

    // Supported on both Mac Intel architectures
    add_platform("i686-apple-darwin");

#endif

    // Supported on all 3 Mac architectures
    add_platform("powerpc-apple-darwin");

#else

    // Any other platform, fall back to the previous method
    add_platform(HOSTTYPE);
#ifdef HOSTTYPEALT
    add_platform(HOSTTYPEALT);
#endif

#endif

    if (config.no_alt_platform) {
        //delete all but first platform (primary)
        platforms.resize(1);
    }

    // add platforms listed in cc_config.xml AFTER the above.
    //
    for (unsigned int i=0; i<config.alt_platforms.size(); i++) {
        add_platform(config.alt_platforms[i].c_str());
    }
}


/// Write XML list of supported platforms.
void CLIENT_STATE::write_platforms(const PROJECT* p, std::ostream& out) {
    out << XmlTag<std::string>("platform_name", p->anonymous_platform ? "anonymous" : get_primary_platform());

    for (size_t i=1; i<platforms.size(); i++) {
        const PLATFORM& platform = platforms[i];
        out << 
        "    <alt_platform>\n"
        "        <name>" << platform.name << "</name>\n"
        "    </alt_platform>\n"
        ;
    }
}

bool CLIENT_STATE::is_supported_platform(const char* p) {
    for (unsigned int i=0; i<platforms.size(); i++) {
        PLATFORM& platform = platforms[i];
        if (platform.name == p) {
            return true;
        }
    }
    return false;
}
