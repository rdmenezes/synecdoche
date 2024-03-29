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

#ifndef HOSTINFO_H
#define HOSTINFO_H

/// \file
/// Description of a host's hardware and software.
/// This is used a few places:
/// - it's part of the client's state file, client_state.xml
/// - it's passed in the reply to the get_host_info GUI RPC
/// - it's included in scheduler RPC requests
///
/// Other host-specific info is kept in
/// - TIME_STATS (on/connected/active fractions)
/// - NET_STATS (average network bandwidths)

#include <iosfwd>

class MIOFILE;

class HOST_INFO {
public:
    int timezone;                 ///< local STANDARD time - UTC time (in seconds)
    char domain_name[256];
    char serialnum[256];
    char ip_addr[256];
    char host_cpid[64];

    int p_ncpus;
    char p_vendor[256];
    char p_model[256];
    char p_features[1024];
    double p_fpops;
    double p_iops;
    double p_membw;
    double p_calculated;          ///< when benchmarks were last run, or zero

    double m_nbytes;              ///< Total amount of memory in bytes
    double m_cache;
    double m_swap;                ///< Total amount of swap space in bytes

    double d_total;               ///< Total amount of disk in bytes
    double d_free;                ///< Total amount of free disk in bytes

    char os_name[256];
    char os_version[256];

    HOST_INFO();
    int parse(MIOFILE& in);
    void write(std::ostream& out, bool suppress_net_info) const;
    int parse_cpu_benchmarks(FILE* in);
    void write_cpu_benchmarks(FILE* out);
    void print() const;

    bool host_is_running_on_batteries();
#ifdef __APPLE__
    bool users_idle(bool check_all_logins, double idle_time_to_run, double *actual_idle_time=NULL);
#else
    bool users_idle(bool check_all_logins, double idle_time_to_run);
#endif
    int get_host_info();
    int get_local_network_info();
    void clear_host_info();

    /// Make a random string using host info.
    void make_random_string(const char* salt, char* out);

    /// Make a host cross-project ID.
    void generate_host_cpid();
};

#ifdef __APPLE__
#ifdef __cplusplus
extern "C" {
#endif
#include <mach/port.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>

// Apple has removed NxIdleTime() beginning with OS 10.6, so we must use 
// weak linking to avoid a run-time crash.  For details, please see the 
// comments in the __APPLE__ version of HOST_INFO::users_idle() in 
// client/hostinfo_unix.cpp.
typedef mach_port_t NXEventHandle;
NXEventHandle NXOpenEventStatus(void) __attribute__((weak_import));
double NXIdleTime(NXEventHandle handle) __attribute__((weak_import));
#ifdef __cplusplus
}   // extern "C"
#endif

extern NXEventHandle gEventHandle;
#endif

#endif
