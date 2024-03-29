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

#ifdef _WIN32
#include <wincrypt.h>
#else
#include "config.h"
#include <cstdio>
#endif

#include "md5_file.h"
#include "md5.h"
#include "error_numbers.h"

int md5_file(const char* path, char* output, double& nbytes) {
    unsigned char buf[4096];
    unsigned char binout[16];
    FILE* f;
    md5_state_t state;
    int i, n;

    nbytes = 0;
    f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "md5_file: can't open %s\n", path);
        perror("md5_file");
        return ERR_FOPEN;
    }
    md5_init(&state);
    while (1) {
        n = (int)fread(buf, 1, 4096, f);
        if (n<=0) break;
        nbytes += n;
        md5_append(&state, buf, n);
    }
    md5_finish(&state, binout);
    for (i=0; i<16; i++) {
        sprintf(output+2*i, "%02x", binout[i]);
    }
    output[32] = 0;
    fclose(f);
    return 0;
}

int md5_block(const unsigned char* data, int nbytes, char* output) {
    unsigned char binout[16];
    int i;

    md5_state_t state;
    md5_init(&state);
    md5_append(&state, data, nbytes);
    md5_finish(&state, binout);
    for (i=0; i<16; i++) {
        sprintf(output+2*i, "%02x", binout[i]);
    }
    output[32] = 0;
    return 0;
}

std::string md5_string(const unsigned char* data, int nbytes) {
    char output[MD5_LEN];
    md5_block(data, nbytes, output);
    return std::string(output);
}

int make_random_string(char* out) {
    char buf[256];
#ifdef _WIN32
    HCRYPTPROV hCryptProv;
        
    if(! CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0)) {
        return -1;
    }
    
    if(! CryptGenRandom(hCryptProv, (DWORD) 32, (BYTE *) buf)) {
        CryptReleaseContext(hCryptProv, 0);
        return -1;
    }
        
    CryptReleaseContext(hCryptProv, 0);
#else
    FILE* f = fopen("/dev/random", "r");
    if (!f) {
        return -1;
    }
    size_t n = fread(buf, 32, 1, f);
    fclose(f);
    if (n != 1) return -1;
#endif
    md5_block((const unsigned char*)buf, 32, out);
    return 0;
}
