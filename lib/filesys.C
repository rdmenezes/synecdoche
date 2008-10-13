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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#if !defined(_WIN32) || defined(__CYGWIN32__)
#include "config.h"
#include <cstdio>
#include <fcntl.h>
#include <cerrno>
#include <sys/stat.h>
#include <sys/file.h>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <sstream>

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/mount.h>
#endif

#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#define STATFS statvfs
#elif defined(HAVE_SYS_STATFS_H)
#include <sys/statfs.h>
#define STATFS statfs
#else
#define STATFS statfs
#endif
#endif

#ifdef _WIN32
typedef BOOL (CALLBACK* FreeFn)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
#endif

#include "util.h"
#include "str_util.h"
#include "error_numbers.h"
#include "filesys.h"

char boinc_failed_file[256];

// routines for enumerating the entries in a directory

/// Check if the given path denotes a file.
///
/// \param[in] path The path that should be checked.
/// \return One if the given path denotes a file, zero otherwise.
int is_file(const char* path) {
    struct stat sbuf;
    int retval = stat(path, &sbuf);
    return (!retval && (((sbuf.st_mode) & S_IFMT) == S_IFREG));
}

/// Check if the given path denotes a directory.
///
/// \param[in] path The path that should be checked.
/// \return One if the given path denotes a directory, zero otherwise.
int is_dir(const char* path) {
    struct stat sbuf;
    int retval = stat(path, &sbuf);
    return (!retval && (((sbuf.st_mode) & S_IFMT) == S_IFDIR));
}

#ifndef _WIN32
/// Check if the given path denotes a symbolic link.
///
/// \param[in] path The path that should be checked.
/// \return One if the given path denotes a symbolic link, zero otherwise.
int is_symlink(const char* path) {
    struct stat sbuf;
    int retval = lstat(path, &sbuf);
    return (!retval && S_ISLNK(sbuf.st_mode));
}
#endif

/// Open a directory for scanning with dir_scan.
///
/// \param[in] p Path denoting the directory that should be opened.
/// \return A structure that can be passed to dir_scan if the directory
///         denoted by \a p could be opened, NULL if opening failed.
DIRREF dir_open(const char* p) {
    DIRREF dirp;
#ifdef _WIN32
    if (!is_dir(p)) return NULL;
    dirp = (DIR_DESC*) calloc(sizeof(DIR_DESC), 1);
    if (!dirp) {
        fprintf(stderr, "calloc() failed in dir_open()\n");
        return NULL;
    }
    dirp->first = true;
    safe_strcpy(dirp->path, p);
    strcat(dirp->path, "\\*");
    dirp->handle = INVALID_HANDLE_VALUE;
#else
    dirp = opendir(p);
    if (!dirp) return NULL;
#endif
    return dirp;
}

/// Scan through a directory and return the next file name in it.
///
/// \param[out] p Pointer to a buffer that will receive the name of the
///               next file.
/// \param[in,out] dirp Pointer retrieved from a call to dir_open.
/// \param[in] p_len Size of the buffer pointed to by \a p.
/// \return Zero on success, ERR_BUFFER_OVERFLOW if the buffer
///         pointed to by \a p is too small, ERR_READDIR if the
///         directory-handle of \a dirp is invalid.
int dir_scan(char* p, DIRREF dirp, int p_len) {
    std::string buffer;
    int ret_val = dir_scan(buffer, dirp);
    if (ret_val == 0) {
        // First check if the provided buffer is big enough:
        if ((p) && (buffer.size() >= static_cast<std::string::size_type>(p_len))) {
            return ERR_BUFFER_OVERFLOW;
        }
        strlcpy(p, buffer.c_str(), p_len);
    }
    return ret_val;
}

/// Scan through a directory and return the next file name in it.
///
/// \param[out] p Reference to a buffer that will receive the name of the
///               next file.
/// \param[in,out] dirp Pointer retrieved from a call to dir_open.
/// \return Zero on success, ERR_READDIR if the
///         directory-handle of \a dirp is invalid.
int dir_scan(std::string& p, DIRREF dirp) {
#ifdef _WIN32
    WIN32_FIND_DATA data;
    while (1) {
        if (dirp->first) {
            dirp->first = false;
            dirp->handle = FindFirstFile(dirp->path, &data);
            if (dirp->handle == INVALID_HANDLE_VALUE) {
                return ERR_READDIR;
            } else {
                // does Windows have "." and ".."?  well, just in case.
                if (!strcmp(data.cFileName, ".")) continue;
                if (!strcmp(data.cFileName, "..")) continue;
                p = data.cFileName;
                return 0;
            }
        } else {
            if (FindNextFile(dirp->handle, &data)) {
                if (!strcmp(data.cFileName, ".")) continue;
                if (!strcmp(data.cFileName, "..")) continue;
                p = data.cFileName;
                return 0;
            } else {
                FindClose(dirp->handle);
                dirp->handle = INVALID_HANDLE_VALUE;
                return 1;
            }
        }
    }
#else
    while (1) {
        dirent* dp = readdir(dirp);
        if (dp) {
            if (!strcmp(dp->d_name, ".")) continue;
            if (!strcmp(dp->d_name, "..")) continue;
            p = dp->d_name;
            return 0;
        } else {
            return ERR_READDIR;
        }
    }
#endif
}

/// Close a directory previously opened by dir_open.
///
/// \param[in] dirp A DIRREF struct previously obtained by a call to dir_open.
void dir_close(DIRREF dirp) {
#ifdef _WIN32
    if (dirp->handle != INVALID_HANDLE_VALUE) {
        FindClose(dirp->handle);
        dirp->handle = INVALID_HANDLE_VALUE;
    }
    free(dirp);
#else
    if (dirp) {
        closedir(dirp);
    }
#endif
}

DirScanner::DirScanner(std::string const& path) {
#ifdef _WIN32
    first = true;
    handle = INVALID_HANDLE_VALUE;
    if (!is_dir((char*)path.c_str())) {
        return;
    }
    dir = path + "\\*";
#else
    dirp = opendir(path.c_str());
#endif
}

/// Scan through a directory and return the next file name in it
bool DirScanner::scan(std::string& s) {
#ifdef _WIN32
    WIN32_FIND_DATA data;
    while (1) {
        if (first) {
            first = false;
            handle = FindFirstFile(dir.c_str(), &data);
            if (handle == INVALID_HANDLE_VALUE) {
                return false;
            } else {
                if (data.cFileName[0] == '.') continue;
                s = data.cFileName;
                return true;
            }
        } else {
            if (FindNextFile(handle, &data)) {
                if (data.cFileName[0] == '.') continue;
                s = data.cFileName;
                return true;
            } else {
                FindClose(handle);
                handle = INVALID_HANDLE_VALUE;
                return false;
            }
        }
    }
#else
    if (!dirp) return false;

    while (1) {
        dirent* dp = readdir(dirp);
        if (dp) {
            if (dp->d_name[0] == '.') continue;
            s = dp->d_name;
            return true;
        } else {
            return false;
        }
    }
#endif
}

DirScanner::~DirScanner() {
#ifdef _WIN32
    if (handle != INVALID_HANDLE_VALUE) {
        FindClose(handle);
    }
#else
    if (dirp) {
        closedir(dirp);
    }
#endif
}

static int boinc_delete_file_aux(const char* path) {
#ifdef _WIN32
    if (!DeleteFile(path)) {
        return ERR_UNLINK;
    }
#else
    int retval = unlink(path);
    if (retval) return ERR_UNLINK;
#endif
    return 0;
}

// Delete the file located at path
int boinc_delete_file(const char* path) {
    int retval = 0;

    if (!boinc_file_exists(path)) {
        return 0;
    }
    retval = boinc_delete_file_aux(path);
    if (retval) {
        double start = dtime();
        do {
            boinc_sleep(drand()*2);       // avoid lockstep
            retval = boinc_delete_file_aux(path);
            if (!retval) break;
        } while (dtime() < start + FILE_RETRY_INTERVAL);
    }
    if (retval) {
        safe_strcpy(boinc_failed_file, path);
        return ERR_UNLINK;
    }
    return 0;
}

/// get file size
int file_size(const char* path, double& size) {
    struct stat sbuf;
    int retval;

    retval = stat(path, &sbuf);
    if (retval) return ERR_NOT_FOUND;
    size = (double)sbuf.st_size;
    return 0;
}

int boinc_truncate(const char* path, double size) {
    int retval;
#if defined(_WIN32) &&  !defined(__CYGWIN32__)
    // the usual Windows nightmare.
    // There's another function, SetEndOfFile(),
    // that supposedly works with files over 2GB,
    // but it uses HANDLES
    //
    int fd = _open(path, _O_RDWR, 0);
    if (fd == -1) return ERR_TRUNCATE;
    retval = _chsize(fd, (long)size);
    _close(fd);
#else
    retval = truncate(path, (off_t)size);
#endif
    if (retval) return ERR_TRUNCATE;
    return 0;
}

/// Remove everything from specified directory.
///
/// \param[in] dirpath Path to the directory that should be cleaned.
/// \return Zero on success, nonzero otherwise.
int clean_out_dir(const char* dirpath) {
    DIRREF dirp = dir_open(dirpath);
    if (!dirp) {
        return 0;    // If dir doesn't exist, it's empty.
    }
    while (true) {
        std::string filename;
        if (dir_scan(filename, dirp)) {
            // The directory is empty - we're done.
            break;
        }
        std::string path = std::string(dirpath).append("/").append(filename);
        clean_out_dir(path.c_str());
        boinc_rmdir(path.c_str());
        int retval = boinc_delete_file(path.c_str());
        if (retval) {
            // Deleting of one file failed - abort the
            // operation and return the error code.
            dir_close(dirp);
            return retval;
        }
    }
    dir_close(dirp);
    return 0;
}

/// Return total size of files in directory and optionally its subdirectories.
/// Win: use special version because stat() is slow, can be avoided
/// Unix: follow symbolic links
///
/// \param[in] dirpath Path to the directory which size should be calculated.
/// \param[out] size Referenze to a variable that will receive the size of the
///                  directory specified by \a dirpath.
/// \param[in] recurse If true the size of all sub-directories will be
///                    considered, too.
int dir_size(const char* dirpath, double& size, bool recurse) {
#ifdef WIN32
    std::string path2(dirpath);
    path2.append("/*");
    size = 0.0;
    WIN32_FIND_DATA findData;
    HANDLE hFind = ::FindFirstFile(path2.c_str(), &findData);
    if (INVALID_HANDLE_VALUE != hFind) {
        do {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!recurse) continue;
                if (!strcmp(findData.cFileName, ".")) continue;
                if (!strcmp(findData.cFileName, "..")) continue;

                double dsize = 0;
                std::string buf(dirpath);
                buf.append("/").append(findData.cFileName);
                dir_size(buf.c_str(), dsize, recurse);
                size += dsize;
            } else {
                size += findData.nFileSizeLow + ((__int64)(findData.nFileSizeHigh) << 32);
            }
        } while (FindNextFile(hFind, &findData));
		::FindClose(hFind);
    }  else {
        return ERR_OPENDIR;
    }
#else
    int retval=0;
    DIRREF dirp;
    double x;

    size = 0;
    dirp = dir_open(dirpath);
    if (!dirp) return ERR_OPENDIR;
    while (1) {
        std::string filename;
        retval = dir_scan(filename, dirp);
        if (retval) break;
        std::string subdir(dirpath);
        subdir.append("/").append(filename);

        if (is_dir(subdir.c_str())) {
            if (recurse) {
                retval = dir_size(subdir.c_str(), x);
                if (retval) continue;
                size += x;
            }
        } else {
            retval = file_size(subdir.c_str(), x);
            if (retval) continue;
            size += x;
        }
    }
    dir_close(dirp);
#endif
    return 0;
}

FILE* boinc_fopen(const char* path, const char* mode) {
    FILE* f;

    // if opening for read, and file isn't there,
    // leave now (avoid 5-second delay!!)
    //
    if (strchr(mode, 'r')) {
        if (!boinc_file_exists(path)) {
            return 0;
        }
    }
    f = fopen(path, mode);
#ifdef _WIN32
    // on Windows: if fopen fails, try again for 5 seconds
    // (since the file might be open by FastFind, Diskeeper etc.)
    //
    if (!f) {
        double start = dtime();
        do {
            boinc_sleep(drand()*2);
            f = _fsopen(path, mode, _SH_DENYNO);
                // _SH_DENYNO makes the file sharable while open
            if (f) break;
        } while (dtime() < start + FILE_RETRY_INTERVAL);
    }
#else
    // Unix - if call was interrupted, retry a few times
    //
    if (!f) {
        for (int i=0; i<5; i++) {
            boinc_sleep(drand());
            if (errno != EINTR) break;
            f = fopen(path, mode);
            if (f) break;
        }
    }
    if (f) {
        fcntl(fileno(f), F_SETFD, FD_CLOEXEC);
    }
#endif
    return f;
}


int boinc_file_exists(const std::string& path) {
   struct stat buf;
   if (stat(path.c_str(), &buf)) {
       return false;     // stat() returns zero on success
   }
   return true;
}

/// same, but doesn't traverse symlinks
int boinc_file_or_symlink_exists(const std::string& path) {
   struct stat buf;
#ifdef _WIN32
   if (stat(path.c_str(), &buf)) {
#else
   if (lstat(path.c_str(), &buf)) {
#endif
       return false;     // stat() returns zero on success
   }
   return true;
}

// returns zero on success, nonzero if didn't touch file
int boinc_touch_file(const char *path) {
    FILE *fp;

    if (boinc_file_exists(path)) {
        return 0;
    }
    fp = fopen(path, "w");
    if (fp) {
        fclose(fp);
        return 0;
    }
    return -1;
}

/// Copy a file.
///
/// \param[in] orig Path of the source file.
/// \param[in] newf Path of the destination.
/// \return Zero on success, nonzero otherwise.
int boinc_copy(const char* orig, const char* newf) {
#ifdef _WIN32
    if (!CopyFile(orig, newf, FALSE)) {     // FALSE means overwrite OK
        return GetLastError();
    }
    return 0;
#elif defined(__EMX__)
    std::ostringstream cmd;
    cmd << "copy " << orig << ' ' << newf;
    return system(cmd.str().c_str());
#else
    std::ostringstream cmd;
    cmd << "cp " << orig << ' ' << newf;
    return system(cmd.str().c_str());
#endif
}

static int boinc_rename_aux(const char* old, const char* newf) {
#ifdef _WIN32
    boinc_delete_file(newf);
    if (MoveFile(old, newf)) return 0;
    return GetLastError();
#else
    return rename(old, newf);
#endif
}

int boinc_rename(const char* old, const char* newf) {
    int retval=0;

    retval = boinc_rename_aux(old, newf);
    if (retval) {
        double start = dtime();
        do {
            boinc_sleep(drand()*2);       // avoid lockstep
            retval = boinc_rename_aux(old, newf);
            if (!retval) break;
        } while (dtime() < start + FILE_RETRY_INTERVAL);
    }
    return retval;
}

/// make a dir that's owner and group RWX
int boinc_mkdir(const char* path) {
    if (is_dir(path)) return 0;
#ifdef _WIN32
    if (!CreateDirectory(path, NULL)) {
        return GetLastError();
    }
    return 0;
#else
    mode_t old_mask = umask(0);
    int retval = mkdir(path, 0771);
    umask(old_mask);
    return retval;
#endif
}

int boinc_rmdir(const char* name) {
#ifdef _WIN32
    if (!RemoveDirectory(name)) {
        return ERR_RMDIR;
    }
#else
    int retval = rmdir(name);
    if (retval) return ERR_RMDIR;
#endif
    return 0;
}

#ifndef _WIN32
int boinc_chown(const char* path, gid_t gid) {
    if (gid) {
        if (chown(path, (uid_t)-1, gid)) {
            return ERR_CHOWN;
        }
    }
    return 0;
}
#endif

/// Create directories with parent directories if necessary.
/// If \a filepath is of the form "a/b/c", this function
/// creates the directories "dirpath/a", "dirpath/a/b".
///
/// \param[in] dirpath The base directory in which the directory
///                    creation should start.
/// \param[in] filepath The path that should be created.
/// \return Zero on success, nonzero otherwise.
int boinc_make_dirs(const char* dirpath, const char* filepath) {
    std::string buf(filepath);
    std::string oldpath(dirpath);
    std::string::size_type pos;
    while ((pos = buf.find('/')) != std::string::npos) {
        std::ostringstream newpath;
        newpath << oldpath << '/' << buf.substr(0, pos);
        int retval = boinc_mkdir(newpath.str().c_str());
        if (retval) {
            return retval;
        }
        oldpath = newpath.str();
        buf.erase(0, pos + 1);
    }
    return 0;
}


FILE_LOCK::FILE_LOCK() {
#ifndef _WIN32
    fd = -1;
#endif
}
FILE_LOCK::~FILE_LOCK() {
#ifndef _WIN32
    if (fd >= 0) close(fd);
#endif
}

int FILE_LOCK::lock(const char* filename) {
#if defined(_WIN32) && !defined(__CYGWIN32__)
    handle = CreateFile(
        filename, GENERIC_WRITE,
        0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
    );
    if (handle == INVALID_HANDLE_VALUE) {
        return -1;
    }
    return 0;

#else
    if (fd<0) {
        fd = open(
            filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
        );
    }
    if (fd<0) {
        return -1;
    }

    struct flock fl;
    fl.l_type=F_WRLCK;
    fl.l_whence=SEEK_SET;
    fl.l_start=0;
    fl.l_len=0;
    if (-1 != fcntl(fd, F_SETLK, &fl)) return 0;
    return -1;
#endif
}

int FILE_LOCK::unlock(const char* filename) {
#if defined(_WIN32) && !defined(__CYGWIN32__)
    if (!CloseHandle(handle)) {
        perror("FILE_LOCK::unlock(): close failed.");
    }
#else
    if (close(fd)) {
        perror("FILE_LOCK::unlock(): close failed.");
    }
#endif
    boinc_delete_file(filename);
    return 0;
}

void boinc_getcwd(char* path) {
    // TODO: This is unsafe.
    getcwd(path, 256);
}

void relative_to_absolute(const char* relname, char* path) {
    // TODO: Still unsafe here.
    getcwd(path, 256);
    if (strlen(relname)) {
        strcat(path, "/");
        strcat(path, relname);
    }
}

/// get total and free space on current filesystem (in bytes)
int get_filesystem_info(double &total_space, double &free_space, const char* path) {
#ifdef _WIN32
    char buf[256];
    boinc_getcwd(buf);
    FreeFn pGetDiskFreeSpaceEx;
    pGetDiskFreeSpaceEx = (FreeFn)GetProcAddress(
        GetModuleHandle("kernel32.dll"), "GetDiskFreeSpaceExA"
    );
    if (pGetDiskFreeSpaceEx) {
        ULARGE_INTEGER TotalNumberOfFreeBytes;
        ULARGE_INTEGER TotalNumberOfBytes;
        ULARGE_INTEGER TotalNumberOfBytesFreeToCaller;
        pGetDiskFreeSpaceEx(
            buf, &TotalNumberOfBytesFreeToCaller, &TotalNumberOfBytes,
            &TotalNumberOfFreeBytes
        );
        signed __int64 uMB;
        uMB = TotalNumberOfFreeBytes.QuadPart / (1024 * 1024);
        free_space = uMB * 1024.0 * 1024.0;
        uMB = TotalNumberOfBytes.QuadPart / (1024 * 1024);
        total_space = uMB * 1024.0 * 1024.0;
    } else {
        DWORD dwSectPerClust;
        DWORD dwBytesPerSect;
        DWORD dwFreeClusters;
        DWORD dwTotalClusters;
        GetDiskFreeSpace(
            buf, &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters,
            &dwTotalClusters
        );
        free_space = (double)dwFreeClusters * dwSectPerClust * dwBytesPerSect;
        total_space = (double)dwTotalClusters * dwSectPerClust * dwBytesPerSect;
    }
#else
#ifdef STATFS
    struct STATFS fs_info;

    STATFS(path, &fs_info);
#ifdef HAVE_SYS_STATVFS_H
    total_space = (double)fs_info.f_frsize * (double)fs_info.f_blocks;
    free_space = (double)fs_info.f_frsize * (double)fs_info.f_bavail;
#else
    total_space = (double)fs_info.f_bsize * (double)fs_info.f_blocks;
    free_space = (double)fs_info.f_bsize * (double)fs_info.f_bavail;
#endif
#else
#error Need to specify a method to obtain free/total disk space
#endif
#endif
    return 0;
}

#ifndef _WIN32

int get_file_dir(char* filename, char* dir) {
    char buf[8192], *p, path[256];
    struct stat sbuf;
    int retval;

    p = getenv("PATH");
    if (!p) return ERR_NOT_FOUND;
    strcpy(buf, p);

    p = strtok(buf, ":");
    while (p) {
        sprintf(path, "%s/%s", p, filename);
        retval = stat(path, &sbuf);
        if (!retval && (sbuf.st_mode & 0111)) {
            strcpy(dir, p);
            return 0;
        }
        p = strtok(0, ":");
    }
    return ERR_NOT_FOUND;
}


#endif
