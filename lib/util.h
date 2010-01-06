// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Peter Kortschack
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

#ifndef UTIL_H
#define UTIL_H

#include <cstdlib>
#include <cmath>
#include <cstdio>

#include <string>
#include <vector>
#include <list>
#include <iosfwd>

#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

double dtime();
double dday();
void boinc_sleep(double seconds);
void push_unique(std::string s, std::vector<std::string>& v);

// NOTE: use #include <functional>   to get max,min

#define SECONDS_PER_DAY 86400

inline double drand() {
    return (double)rand()/(double)RAND_MAX;
}

/// Interpolates \a a and \a b linearly, using factor \a f.
/// When \a f is 0, returns \a a; when \a f is 1, returns \a b.
inline double interpolate(double a, double b, double f) {
    return a*(1.0-f) + b*f;
}

#ifdef _WIN32
#include <windows.h>

int boinc_thread_cpu_time(HANDLE thread_handle, double& cpu);
int boinc_process_cpu_time(double& cpu);
#else
// setpriority(2) arg to run in background
// (don't use 20 because
//
static const int PROCESS_IDLE_PRIORITY = 19;
double linux_cpu_time(int pid);
#endif

void update_average(double work_start_time, double work, double half_life, double& avg, double& avg_time);

int boinc_calling_thread_cpu_time(double& cpu);

void boinc_crash();
int read_file_malloc(const char* path, char*&, int max_len=0, bool tail=false);
int read_file_string(const char* path, std::string&, int max_len=0, bool tail=false);
int copy_stream(FILE* in, FILE* out);
int copy_stream(std::istream& in, std::ostream& out);

#ifdef _WIN32

int run_program(
    const char* path, const char* cdir, int argc, const char* const argv[], double, HANDLE& id
);

void kill_program(HANDLE proc);
int get_exit_status(HANDLE proc);
bool process_exists(HANDLE proc);

#else
int run_program(
    const char* path, const char* cdir, int argc, const char* const argv[], double, int&
);
void kill_program(int pid);
int get_exit_status(int pid);
bool process_exists(int pid);

/// Prepare arguments for execv and call that function.
int do_execv(const std::string& path, const std::list<std::string>& argv);
#endif

int wait_client_mutex(const char* dir, double timeout);
#endif
