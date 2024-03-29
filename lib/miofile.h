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

#ifndef _MIOFILE_
#define _MIOFILE_

#include <cstdio>

#include <string>

class MFILE;

/// MIOFILE lets you do formatted I/O to either a FILE or a memory buffer,
/// depending on how you initialize it.
///
/// - output:
///   - init_file(): output goes to the FILE* that you specify
///   - init_mfile(): output goes to an MFILE (i.e. a memory buffer);
///     you can call MFILE::get_buf() to get the results
/// - input:
///   - init_file(): input comes from the FILE* that you specify
///   - init_buf(): input comes from the buffer you specify.
///     This string is not modified.
///
/// Why is this here?  Because on Windows (9x, maybe all)
/// you can't do fdopen() on a socket.

class MIOFILE {
    MFILE* mf;
    FILE* f;
    char* wbuf;
    int len;
    const char* buf;
public:
    MIOFILE();
    ~MIOFILE();
    void init_mfile(MFILE* mfile);
    void init_file(FILE* file);
    void init_buf_read(const char* buf);
    void init_buf_write(char* buf, int len);
    int printf(const char* format, ...);
    char* fgets(char* dst, int dst_len);
    int _ungetc(int c);
    inline int _getc() {
        if (f) {
            return getc(f);
        }
        return (*buf)?(*buf++):EOF;
    }
};

int copy_element_contents(MIOFILE& in, const char* end_tag, char* p, int len);
int copy_element_contents(MIOFILE& in, const char* end_tag, std::string& str);

#endif
