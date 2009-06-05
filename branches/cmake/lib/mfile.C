// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
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

#ifndef _WIN32
#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <string>
#include <cerrno>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <unistd.h>
#endif

#include "mfile.h"
#include "filesys.h"
#include "error_numbers.h"

MFILE::MFILE() {
    buf = 0;
    len = 0;
}

MFILE::~MFILE() {
    if (buf) free(buf);
}

int MFILE::open(const char* path, const char* mode) {
    f = boinc_fopen(path, mode);
    if (!f) return ERR_FOPEN;
    return 0;
}

namespace {
    #define BUFSIZE 100000
    char mfile_vprintf_buf2[BUFSIZE];
}

int MFILE::vprintf(const char* format, va_list ap) {
    int k = vsnprintf(mfile_vprintf_buf2, BUFSIZE, format, ap);
    if (k<=-1 || k>=BUFSIZE) {
        fprintf(stderr, "ERROR: buffer too small in MFILE::vprintf()\n");
        fprintf(stderr, "ERROR: format: %s\n", format);
        fprintf(stderr, "ERROR: k=%d, BUFSIZE=%d\n", k, BUFSIZE);
        return -1;
    }
    buf = (char*)realloc(buf, len + k + 1);
    if (!buf) {
        errno = ERR_MALLOC;
        return ERR_MALLOC;
    }
    strncpy(buf + len, mfile_vprintf_buf2, k);
    len += k;
    buf[len] = 0;
    return k;
}

int MFILE::printf(const char* format, ...) {
    int n;
    va_list ap;

    va_start(ap, format);
    n = MFILE::vprintf(format, ap);
    va_end(ap);
    return n;
}

size_t MFILE::write(const void* data, size_t size, size_t nitems) {
    buf = (char *)realloc(buf, len+(size*nitems)+1);
    if (!buf) {
        errno = ERR_MALLOC;
        return 0;
    }
    memcpy(buf+len, data, size*nitems);
    len += (int)size*(int)nitems;
    buf[len] = 0;
    return nitems;
}

int MFILE::_putchar(char c) {
    buf = (char*)realloc(buf, len+1+1);
    if (!buf) {
        errno = ERR_MALLOC;
        return EOF;
    }
    buf[len] = c;
    len++;
    buf[len] = 0;
    return c;
}

int MFILE::puts(const char* str) {
    int n = (int)strlen(str);
    buf = (char*)realloc(buf, len+n+1);
    if (!buf) {
        errno = ERR_MALLOC;
        return EOF;
    }
    strncpy(buf+len, str, n);
    len += n;
    buf[len] = 0;
    return n;
}

int MFILE::close() {
    int retval = flush();
    fclose(f);
    free(buf);
    buf = 0;
    f = NULL;
    return retval;
}

int MFILE::flush() {
    int n, old_len = len;

    n = (int)fwrite(buf, 1, len, f);
    len = 0;
    if (n != old_len) return ERR_FWRITE;
    if (fflush(f)) return ERR_FFLUSH;
#ifndef _WIN32
    if (fsync(fileno(f)) < 0) return ERR_FSYNC;
#endif
    return 0;
}

long MFILE::tell() const {
    return f ? ftell(f) : -1;
}

void MFILE::get_buf(char*& buffer, int& length) {
    buffer = buf;
    length = len;
    buf = 0;
    len = 0;
}
