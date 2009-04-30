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

/// \file
/// platform-independent interface to shared memory

#ifndef BOINC_SHMEM_H
#define BOINC_SHMEM_H

#ifndef _WIN32
#include <sys/types.h>
#include <sys/shm.h>
#endif

// create_shmem(): create a shared-memory segment of the given size.
// attach_shmem(): attach to a shared-memory segment
// detach_shmem(): detach from a shared-mem segment.
//    Once all processes have detached, the segment is destroyed
// The above with _mmap: V6 mmap() shared memory for Unix/Linux/Mac

#ifdef _WIN32
HANDLE create_shmem(
    LPCTSTR seg_name, int size, void** pp, bool try_global=true
);
HANDLE attach_shmem(LPCTSTR seg_name, void** pp);
int detach_shmem(HANDLE hSharedMem, void* p);
#else
#define MMAPPED_FILE_NAME    "boinc_mmap_file"
int create_shmem_mmap(const char *path, size_t size, void** pp);
int attach_shmem_mmap(const char *path, void** pp);
int detach_shmem_mmap(void* p, size_t size);
int create_shmem(key_t, int size, gid_t gid, void**);
int attach_shmem(key_t, void**);
int detach_shmem(void*);
int shmem_info(key_t key);
// Destroy a shared-memory segment.
// If there are attachments to it,
// print a message in a loop until the attachments are gone
//
int destroy_shmem(key_t);
#endif      // !defined(_WIN32)

#endif      // BOINC_SHMEM_H
