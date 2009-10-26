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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif
#ifdef _WIN32
#include "win_util.h"
#endif

#ifndef M_LN2
#define M_LN2      0.693147180559945309417
#endif

#ifndef _WIN32
#include "config.h"
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/resource.h>
#include <errno.h>
#endif

#include "util.h"

#include <cstring>

#include <list>
#include <string>
#include <sstream>

#include "error_numbers.h"
#include "common_defs.h"
#include "filesys.h"
#include "str_util.h"
#include "parse.h"

#define EPOCHFILETIME_SEC (11644473600.)
#define TEN_MILLION 10000000.

/// Return time of day (seconds since 1970) as a double.
double dtime() {
#ifdef _WIN32
    LARGE_INTEGER time;
    FILETIME sysTime;
    double t;
    GetSystemTimeAsFileTime(&sysTime);
    time.LowPart = sysTime.dwLowDateTime;
    time.HighPart = sysTime.dwHighDateTime;  // Time is in 100 ns units
    t = (double)time.QuadPart;    // Convert to 1 s units
    t /= TEN_MILLION;                /* In seconds */
    t -= EPOCHFILETIME_SEC;     /* Offset to the Epoch time */
    return t;
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + (tv.tv_usec/1.e6);
#endif
}

/// Return time today 0:00 in seconds since 1970 as a double.
double dday() {
    double now = dtime();
    return (now-fmod(now, SECONDS_PER_DAY));
}

/// Sleep for a specified number of seconds.
void boinc_sleep(double seconds) {
#ifdef _WIN32
    ::Sleep((int)(1000*seconds));
#else
    double end_time = dtime() + seconds - 0.01;
    // sleep() and usleep() can be interrupted by SIGALRM,
    // so we may need multiple calls
    //
    while (1) {
        if (seconds >= 1) {
            sleep((unsigned int) seconds);
        } else {
            usleep((int)fmod(seconds*1000000, 1000000));
        }
        seconds = end_time - dtime();
        if (seconds <= 0) break;
    }
#endif
}

void push_unique(std::string s, std::vector<std::string>& v) {
    if (std::find(v.begin(), v.end(), s) == v.end()) {
        v.push_back(s);
    }
}

#ifdef _WIN32

int boinc_thread_cpu_time(HANDLE thread_handle, double& cpu) {
    FILETIME creationTime, exitTime, kernelTime, userTime;

    if (GetThreadTimes(
        thread_handle, &creationTime, &exitTime, &kernelTime, &userTime)
    ) {
        ULARGE_INTEGER tKernel, tUser;
        LONGLONG totTime;

        tKernel.LowPart  = kernelTime.dwLowDateTime;
        tKernel.HighPart = kernelTime.dwHighDateTime;
        tUser.LowPart    = userTime.dwLowDateTime;
        tUser.HighPart   = userTime.dwHighDateTime;
        totTime = tKernel.QuadPart + tUser.QuadPart;

        // Runtimes in 100-nanosecond units
        cpu = totTime / 1.e7;
    } else {
        return -1;
    }
    return 0;
}

int boinc_process_cpu_time(double& cpu) {
    FILETIME creationTime, exitTime, kernelTime, userTime;

    if (GetProcessTimes(
        GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime)
    ) {
        ULARGE_INTEGER tKernel, tUser;
        LONGLONG totTime;

        tKernel.LowPart  = kernelTime.dwLowDateTime;
        tKernel.HighPart = kernelTime.dwHighDateTime;
        tUser.LowPart    = userTime.dwLowDateTime;
        tUser.HighPart   = userTime.dwHighDateTime;
        totTime = tKernel.QuadPart + tUser.QuadPart;

        // Runtimes in 100-nanosecond units
        cpu = totTime / 1.e7;
    } else {
        return -1;
    }
    return 0;
}

static void get_elapsed_time(double& cpu) {
    static double start_time;

    double now = dtime();
    if (start_time) {
        cpu = now - start_time;
    } else {
        cpu = 0;
    }
    start_time = now;
}

int boinc_calling_thread_cpu_time(double& cpu) {
    if (boinc_thread_cpu_time(GetCurrentThread(), cpu)) {
        get_elapsed_time(cpu);
    }
    return 0;
}

#else

// Unix: pthreads doesn't provide an API for getting per-thread CPU time,
// so just get the process's CPU time
//
int boinc_calling_thread_cpu_time(double &cpu_t) {
    struct rusage ru;

    int retval = getrusage(RUSAGE_SELF, &ru);
    if (retval) return ERR_GETRUSAGE;
    cpu_t = (double)ru.ru_utime.tv_sec + ((double)ru.ru_utime.tv_usec) / 1e6;
    cpu_t += (double)ru.ru_stime.tv_sec + ((double)ru.ru_stime.tv_usec) / 1e6;
    return 0;
}

#endif


/// Update an estimate of "units per day" of something (credit or CPU time).
/// The estimate is exponentially averaged with a given half-life
/// (i.e. if no new work is done, the average will decline by 50% in this time).
/// This function can be called either with new work,
/// or with zero work to decay an existing average.
///
/// NOTE: if you change this, also change update_average in
/// html/inc/credit.inc
void update_average(
    double work_start_time,       // when new work was started
                                    // (or zero if no new work)
    double work,                    // amount of new work
    double half_life,
    double& avg,                    // average work per day (in and out)
    double& avg_time                // when average was last computed
) {
    double now = dtime();

    if (avg_time) {
        // If an average R already exists, imagine that the new work was done
        // entirely between avg_time and now.
        // That gives a rate R'.
        // Replace R with a weighted average of R and R',
        // weighted so that we get the right half-life if R' == 0.
        //
        // But this blows up if avg_time == now; you get 0*(1/0)
        // So consider the limit as diff->0,
        // using the first-order Taylor expansion of
        // exp(x)=1+x+O(x^2).
        // So to the lowest order in diff:
        // weight = 1 - diff ln(2) / half_life
        // so one has
        // avg += (1-weight)*(work/diff_days)
        // avg += [diff*ln(2)/half_life] * (work*SECONDS_PER_DAY/diff)
        // notice that diff cancels out, leaving
        // avg += [ln(2)/half_life] * work*SECONDS_PER_DAY

        double diff, diff_days, weight;

        diff = now - avg_time;
        if (diff<0) diff=0;

        diff_days = diff/SECONDS_PER_DAY;
        weight = exp(-diff*M_LN2/half_life);

        avg *= weight;

        if ((1.0-weight) > 1.e-6) {
            avg += (1-weight)*(work/diff_days);
        } else {
            avg += M_LN2*work*SECONDS_PER_DAY/half_life;
        }
    } else if (work) {
        // If first time, average is just work/duration
        //
        double dd = (now - work_start_time)/SECONDS_PER_DAY;
        avg = work/dd;
    }
    avg_time = now;
}

#ifndef _WIN32
/// (linux) return current CPU time of the given process
double linux_cpu_time(int pid) {
    FILE *file;
    char file_name[24];
    unsigned long utime = 0, stime = 0;
    int n;

    sprintf(file_name,"/proc/%d/stat",pid);
    if ((file = fopen(file_name,"r")) != NULL) {
        n = fscanf(file,"%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%lu%lu",&utime,&stime);
        fclose(file);
        if (n != 2) return 0;
    }
    return (double)(utime + stime)/100;
}
#endif

void boinc_crash() {
#ifdef _WIN32
    DebugBreak();
#else
    *(int*)0 = 0;
#endif
}

/// Read file (at most \a max_len chars, if nonzero) into malloc'd buf.
/// \todo Use std::filebuf instead of FILE.
/// \todo Read into new'd buf? Would need to change callers to use delete
/// instead of free.
int read_file_malloc(const char* path, char*& buf, int max_len, bool tail) {
    FILE* f;
    int retval, isize;
    double size;

    retval = file_size(path, size);
    if (retval) return retval;

    f = fopen(path, "r");
    if (!f) return ERR_FOPEN;

    if (max_len && size > max_len) {
        if (tail) {
            fseek(f, (long)size-max_len, SEEK_SET);
        }
        size = max_len;
    }
    isize = (int) size;
    buf = (char*)malloc(isize+1);
    size_t n = fread(buf, 1, isize, f);
    buf[n] = 0;
    fclose(f);
    return 0;
}

/// Read file (at most \a max_len chars, if nonzero) into string
/// \todo Use std::filebuf directly, and don't delegate to read_file_malloc.
int read_file_string(const char* path, std::string& result, int max_len, bool tail) {
    result.erase();
    int retval;
    char* buf;

    retval = read_file_malloc(path, buf, max_len, tail);
    if (retval) return retval;
    result = buf;
    free(buf);
    return 0;
}

int copy_stream(FILE* in, FILE* out) {
    char buf[1024];
    size_t n, m;
    while (1) {
        n = fread(buf, 1, sizeof(buf), in);
        m = fwrite(buf, 1, n, out);
        if (m != n) return ERR_FWRITE;
        if (n < sizeof(buf)) break;
    }
    return 0;
}

int copy_stream(std::istream& in, std::ostream& out) {
    std::streambuf* s_in = in.rdbuf();
    std::streambuf* s_out= out.rdbuf();
    char buf[1024];
    std::streamsize n, m;
    do {
        n = s_in->sgetn(buf, sizeof(buf));
        m = s_out->sputn(buf, n);
        if (m != n) {
            return ERR_FWRITE;
        }
    } while (n == sizeof(buf));

    return 0;
}

#ifdef _WIN32

/// chdir into the given directory, and run a program there.
/// If \a nsecs is nonzero, make sure it's still running after that many seconds.
///
/// \a argv is set up Unix-style, i.e. argv[0] is the program name
int run_program(
    const char* dir, const char* file, int argc, const char* const argv[], double nsecs, HANDLE& id
) {
    int retval;
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    char cmdline[1024];
    char error_msg[1024];
    unsigned long status;

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
    if (!retval) {
        windows_error_string(error_msg, sizeof(error_msg));
        fprintf(stderr, "CreateProcess failed: '%s'\n", error_msg);
        return -1; // CreateProcess returns 1 if successful, false if it failed.
    }

    if (nsecs) {
        boinc_sleep(nsecs);
        if (GetExitCodeProcess(process_info.hProcess, &status)) {
            if (status != STILL_ACTIVE) {
                return -1;
            }
        }
    }
    id = process_info.hProcess;
    return 0;
}
#else

/// chdir into the given directory, and run a program there.
/// If nsecs is nonzero, make sure it's still running after that many seconds.
///
/// argv is set up Unix-style, i.e. argv[0] is the program name
int run_program(
    const char* dir, const char* file, int , const char *const argv[], double nsecs, int& id
) {
    int retval;
    int pid = fork();
    if (pid == 0) {
        if (dir) {
            retval = chdir(dir);
            if (retval) return retval;
        }
        // Cast is ugly but necessary as execv takes a char* const* instead of
        // const char* const* in many platforms
        execv(file, const_cast<char**>(argv));
        perror("execv");
        exit(errno);
    }

    if (nsecs) {
        boinc_sleep(3);
        if (waitpid(pid, 0, WNOHANG) == pid) {
            return -1;
        }
    }
    id = pid;
    return 0;
}
#endif

#ifdef _WIN32
void kill_program(HANDLE proc) {
    TerminateProcess(proc, 0);
}
#else
void kill_program(int pid) {
    kill(pid, SIGKILL);
}
#endif

#ifdef _WIN32
int get_exit_status(HANDLE proc) {
    unsigned long status=1;
    while (1) {
        if (GetExitCodeProcess(proc, &status)) {
            if (status == STILL_ACTIVE) {
                boinc_sleep(1);
            }
            else {
                break;
            }
        }
    }
    return (int) status;
}
bool process_exists(HANDLE proc) {
    unsigned long status=1;
    if (GetExitCodeProcess(proc, &status)) {
        if (status == STILL_ACTIVE) return true;
    }
    return false;
}

#else
int get_exit_status(int pid) {
    int status;
    waitpid(pid, &status, 0);
    return status;
}
bool process_exists(int pid) {
    int p = waitpid(pid, 0, WNOHANG);
    if (p == pid) return false;     // process has exited
    if (p == -1) return false;      // PID doesn't exist
    return true;
}

/// Prepare arguments for execv and call that function.
/// This function returns only in case of an error.
///
/// \param[in] path The path name of the executable that should get executed.
/// \param[in] argv A list containing all command line arguments for the
///                 program specified in \a path.
/// \return Only returns -1, if it returns.
int do_execv(const std::string& path, const std::list<std::string>& argv)
{
    const char** argv_p = new const char*[argv.size() + 1];
    int i = 0;
    for (std::list<std::string>::const_iterator it = argv.begin();
                it != argv.end(); ++it) {
        argv_p[i++] = (*it).c_str();
    }
    argv_p[i] = 0;

    // Cast is ugly but necessary as execv takes a char* const* instead of
    // const char* const*
    int ret_val = execv(path.c_str(), const_cast<char* const*>(argv_p));

    delete[] argv_p;
    return ret_val;
}

#endif

#ifdef _WIN32
/// Check if there is a mutex with a special name.
/// Used to prevent more than one running instance of the client.
///
/// \param[in] dir Directory containing the lockfile (not used on Windows).
/// \return ERR_ALREADY_RUNNING if the mutex already exists, zero otherwise.
static int get_client_mutex(const char* SYNEC_UNUSED(dir)) {
    // Global mutex
    std::string buf = "Global\\";
    buf += RUN_MUTEX;

    HANDLE h = CreateMutex(NULL, true, buf.c_str());
    if ((h == 0) || (GetLastError() == ERROR_ALREADY_EXISTS)) {
        return ERR_ALREADY_RUNNING;
    }
#else
static int get_client_mutex(const char* dir) {
    std::ostringstream path;
    static FILE_LOCK file_lock;

    path << dir << '/' << LOCK_FILE_NAME;
    if (file_lock.lock(path.str().c_str())) {
        return ERR_ALREADY_RUNNING;
    }
#endif
    return 0;
}

int wait_client_mutex(const char* dir, double timeout) {
    double start = dtime();
    while (1) {
        int retval = get_client_mutex(dir);
        if (!retval) return 0;
        boinc_sleep(1);
        if (dtime() - start > timeout) break;
    }
    return ERR_ALREADY_RUNNING;
}
