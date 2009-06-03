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

#ifndef _FILESYS_
#define _FILESYS_

/// On Windows, retry for this period of time, since some other program
/// (virus scan, defrag, index) may have the file open.
#define FILE_RETRY_INTERVAL 5


#if defined(_WIN32) && !defined(__CYGWIN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <stdio.h>
#include <dirent.h>
#include <grp.h>
#endif /* !WIN32 */

#ifdef __cplusplus
#include <string>
extern "C" {
#endif
  extern int boinc_delete_file(const char*);
  extern int boinc_touch_file(const char *path);
  extern FILE* boinc_fopen(const char* path, const char* mode);
  extern int boinc_copy(const char* orig, const char* newf);
  extern int boinc_rename(const char* old, const char* newf);
  extern int boinc_mkdir(const char*);
#ifndef _WIN32
  extern int boinc_chown(const char*, gid_t);
#endif
  extern int boinc_rmdir(const char*);
  extern int remove_project_owned_file_or_dir(const char* path);
  extern void boinc_getcwd(char*);
  extern void relative_to_absolute(const char* relname, char* path);
  extern int boinc_make_dirs(char*, char*);
  extern char boinc_failed_file[256];
  extern int is_file(const char* path);
  extern int is_dir(const char* path);
  extern int is_symlink(const char* path);
  extern int boinc_truncate(const char*, double);
  extern int boinc_file_exists(const std::string& path);
  extern int boinc_file_or_symlink_exists(const std::string& path);

#ifdef __cplusplus
}
#endif

/* C++ specific prototypes/defines follow here */
#ifdef __cplusplus

extern int file_size(const char*, double&);
extern int clean_out_dir(const char*);
extern int dir_size(const char* dirpath, double&, bool recurse=true);
extern int get_filesystem_info(double& total, double& free, const char* path=".");

// TODO TODO TODO
// remove this code - the DirScanner class does the same thing.
// But need to rewrite a couple of places that use it
//
#if defined(_WIN32) && !defined(__CYGWIN32__)
typedef struct _DIR_DESC {
    char path[256];
    bool first;
    void* handle;
} DIR_DESC;
typedef DIR_DESC *DIRREF;
#else
typedef DIR *DIRREF;
#endif

extern DIRREF dir_open(const char*);
extern int dir_scan(char*, DIRREF, int);
int dir_scan(std::string&, DIRREF);
extern void dir_close(DIRREF);


class DirScanner {
#if defined(_WIN32) && !defined(__CYGWIN32__)
    std::string dir;
    bool first;
    void* handle;
#else
    DIR* dirp;
#endif
public:
    DirScanner(std::string const& path);
    ~DirScanner();
    bool scan(std::string& name);    // return true if file returned
};

struct FILE_LOCK {
#if defined(_WIN32) && !defined(__CYGWIN32__)
    HANDLE handle;
#else
    int fd;
#endif
    FILE_LOCK();
    ~FILE_LOCK();
    int lock(const char* filename);
    int unlock(const char* filename);
};

#ifndef _WIN32

/// search PATH, find the directory that a program is in, if any
extern int get_file_dir(char* filename, char* dir);

#endif


#endif /* c++ */

#endif /* double-inclusion protection */

