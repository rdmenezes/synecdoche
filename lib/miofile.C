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

#ifndef _WIN32
#include "config.h"
#include <string>
#include <cstring>
#include <cstdarg>
#endif

#include "miofile.h"
#include "mfile.h"
#include "error_numbers.h"

using std::string;

MIOFILE::MIOFILE() {
    mf = 0;
    f = 0;
    buf = 0;
}

MIOFILE::~MIOFILE() {
}

void MIOFILE::init_mfile(MFILE* mfile) {
    mf = mfile;
}

void MIOFILE::init_file(FILE* file) {
    f = file;
}

void MIOFILE::init_buf_read(const char* buf) {
    this->buf = buf;
}

void MIOFILE::init_buf_write(char* buf, int len) {
    this->wbuf = buf;
    this->len = len;
    this->wbuf[0] = 0;
}

int MIOFILE::printf(const char* format, ...) {
    int retval;

    va_list ap;
    va_start(ap, format);
    if (mf) {
        retval = mf->vprintf(format, ap);
    } else if (f) {
        retval = vfprintf(f, format, ap);
    } else {
        size_t cursize = strlen(wbuf);
        size_t remaining_len = len - cursize;
        retval = vsnprintf(wbuf+cursize, remaining_len, format, ap);
    }
    va_end(ap);
    return retval;
}

char* MIOFILE::fgets(char* dst, int dst_len) {
    if (f) {
        return ::fgets(dst, dst_len, f);
    }
    const char* q = strchr(buf, '\n');
    if (!q) return 0;

    q++;
    int n = (int)(q - buf);
    if (n > dst_len-1) n = dst_len-1;
    memcpy(dst, buf, n);
    dst[n] = 0;

    buf = q;
    return dst;
}

int MIOFILE::_ungetc(int c) {
    if (f) {
        return ungetc(c, f);
    } else {
        buf--;
        // NOTE: we assume that the char being pushed
        // is what's already there
        //*buf = c;
    }
    return c;
}

/// Copy from a file to static buffer.
int copy_element_contents(MIOFILE& in, const char* end_tag, char* p, int len) {
    char buf[256];
    int n;

    strcpy(p, "");
    while (in.fgets(buf, 256)) {
        if (strstr(buf, end_tag)) {
            return 0;
        }
        n = (int)strlen(buf);
        if (n >= len-1) return ERR_XML_PARSE;
        strcat(p, buf);
        len -= n;
    }
    return ERR_XML_PARSE;
}

int copy_element_contents(MIOFILE& in, const char* end_tag, string& str) {
    char buf[256];

    str.clear();
    while (in.fgets(buf, 256)) {
        if (strstr(buf, end_tag)) {
            return 0;
        }
        str += buf;
    }
    fprintf(stderr, "copy_element_contents(): no end tag\n");
    return ERR_XML_PARSE;
}
