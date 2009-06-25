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

#ifndef FILESYS_H
#define FILESYS_H

/// On Windows, retry for this period of time, since some other program
/// (virus scan, defrag, index) may have the file open.
#define FILE_RETRY_INTERVAL 5

#include "attributes.h"

#if defined(_WIN32) && !defined(__CYGWIN32__)
#else
#include <stdio.h>
#include <dirent.h>
#include <grp.h>
#endif // !WIN32

#ifdef __cplusplus
#include <string>
extern "C" {
#endif // __cplusplus
  /// Delete a file.
  int boinc_delete_file(const std::string& path);

  /// Create an empty file.
  int boinc_touch_file(const char* path);

  /// Open a file for reading or writing.
  FILE* boinc_fopen(const char* path, const char* mode);

  /// Copy a file.
  int boinc_copy(const char* orig, const char* newf);

  /// Rename a file.
  int boinc_rename(const char* old, const char* newf);

  /// Create a directory for which the owner and the group have full access.
  int boinc_mkdir(const char* path);

#ifndef _WIN32
  /// Change the group of a file or directory.
  int boinc_chown(const char* path, gid_t gid);
#endif

  /// Remove a directory.
  int boinc_rmdir(const char* name);

  int remove_project_owned_file_or_dir(const char* path);

  /// Create directories with parent directories if necessary.
  int boinc_make_dirs(const char* dirpath, const char* filepath);

  /// A buffer that will store a file name in case of failed operations.
  extern char boinc_failed_file[256];

  /// Check if the given path denotes a file.
  int is_file(const char* path);

  /// Check if the given path denotes a directory.
  int is_dir(const char* path);

  /// Check if the given path denotes a symbolic link.
  int is_symlink(const char* path);

  /// Truncate the size of a file.
  int boinc_truncate(const char* path, double size);

  /// Check if a file exists.
  bool boinc_file_exists(const std::string& path);

  /// Check if a file exists.
  int boinc_file_or_symlink_exists(const std::string& path);

#ifdef __cplusplus
}
#endif // __cplusplus

/* C++ specific prototypes/defines follow here */
#ifdef __cplusplus

/// Return the current working directory.
std::string boinc_getcwd();

/// Turn a relative path into an absolute one.
std::string relative_to_absolute(const std::string& relname);
//std::string relative_to_absolute(const char* relname);

/// Get the size of a file.
int file_size(const char* path, double& size);

/// Remove everything from specified directory.
int clean_out_dir(const char* dirpath);

/// Return total size of files in directory and optionally its subdirectories.
int dir_size(const char* dirpath, double& size, bool recurse = true);

/// Get total and free space on current filesystem (in bytes).
int get_filesystem_info(double& total, double& free, const char* path=".");

// TODO TODO TODO
// remove this code - the DirScanner class does the same thing.
// But need to rewrite a couple of places that use it
#if defined(_WIN32) && !defined(__CYGWIN32__)
typedef struct _DIR_DESC {
    char path[256];
    bool first;
    void* handle;
} DIR_DESC __attribute__ ((deprecated));
typedef DIR_DESC *DIRREF  __attribute__ ((deprecated));
#else
typedef DIR *DIRREF  __attribute__ ((deprecated));
#endif

/// Open a directory for scanning with dir_scan.
DIRREF dir_open(const char* p) __attribute__ ((deprecated));

/// Scan through a directory and return the next file name in it.
int dir_scan(char* p, DIRREF dirp, int p_len) __attribute__ ((deprecated));

/// Scan through a directory and return the next file name in it.
int dir_scan(std::string& p, DIRREF dirp) __attribute__ ((deprecated));

/// Close a directory previously opened by dir_open.
void dir_close(DIRREF dirp) __attribute__ ((deprecated));


class DirScanner {
private:
#if defined(_WIN32) && !defined(__CYGWIN32__)
    std::string dir;
    bool first;
    void* handle;
#else
    DIR* dirp;
#endif

public:
    /// Create a DirScanner instance and try to open the specified directory.
    DirScanner(const std::string& path);

    /// Destroy the DirScanner instance and frees the contained handle, if necessary.
    ~DirScanner();

    /// Scan through a directory and return the next file name in it.
    bool scan(std::string& name);
};

class FILE_LOCK {
private:
#if defined(_WIN32) && !defined(__CYGWIN32__)
    HANDLE handle;
#else
    int fd;
#endif

public:
    /// Create a FILE_LOCK instance.
    FILE_LOCK();

    /// Destroy the FILE_LOCK instance.
    ~FILE_LOCK();

    /// Lock a file.
    int lock(const char* filename);

    /// Release and delete a previously locked file.
    int unlock(const char* filename);
};

#ifndef _WIN32
/// Search PATH and find the directory that a program is in, if any.
int get_file_dir(const char* filename, std::string& dir);
#endif // _WIN32

#endif // __cplusplus

#endif // FILESYS_H

