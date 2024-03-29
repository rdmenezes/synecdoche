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


// utility program for encryption.
//
// -genkey n private_keyfile public_keyfile
//                  create a key pair with n bits (512 <= n <= 1024)
//                  write it in hex notation
// -sign file private_keyfile
//                  create a signature for a given file
//                  write it in hex notation
// -sign_string string private_keyfile
//                  create a signature for a given string
//                  write it in hex notation
// -verify file signature_file public_keyfile
//                  verify a signature
// -test_crypt private_keyfile public_keyfile
//                  test encrypt/decrypt

#if defined(_WIN32)
#include <windows.h>
#else
#include "config.h"
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "crypt.h"

void die(const char* p) {
    fprintf(stderr, "Error: %s\n", p);
    exit(2);
}

unsigned int random_int() {
    unsigned int n;
#if defined(_WIN32)
#if defined(__CYGWIN32__)
    HMODULE hLib=LoadLibrary((const char *)"ADVAPI32.DLL");
#else
    HMODULE hLib=LoadLibrary("ADVAPI32.DLL");
#endif
    if (!hLib) {
        die("Can't load ADVAPI32.DLL");
    }
    BOOLEAN (APIENTRY *pfn)(void*, ULONG) =
    (BOOLEAN (APIENTRY *)(void*,ULONG))GetProcAddress(hLib,"SystemFunction036");
    if (pfn) {
        char buff[32];
        ULONG ulCbBuff = sizeof(buff);
        if(pfn(buff,ulCbBuff)) {
            // use buff full of random goop
            memcpy(&n,buff,sizeof(n));
        }
    }
    FreeLibrary(hLib);
#else
    FILE* f = fopen("/dev/random", "r");
    if (!f) {
        die("can't open /dev/random\n");
    }
    fread(&n, sizeof(n), 1, f);
    fclose(f);
#endif
    return n;
}

int main(int argc, char** argv) {
    R_RSA_PUBLIC_KEY public_key;
    R_RSA_PRIVATE_KEY private_key;
    int n, retval;
    bool is_valid;
    DATA_BLOCK signature, in, out;
    unsigned char signature_buf[256], buf[256], buf2[256];
    FILE *f, *fpriv, *fpub;
    char cbuf[256];

    if (argc == 1) {
        printf("missing command\n");
        exit(1);
    }
    if (!strcmp(argv[1], "-genkey")) {
        if (argc < 5) {
            fprintf(stderr, "missing cmdline args\n");
            exit(1);
        }
        printf("creating keys in %s and %s\n", argv[3], argv[4]);
        n = atoi(argv[2]);

        srand(random_int());
        RSA* rp = RSA_generate_key(n,  65537, 0, 0);
        openssl_to_keys(rp, n, private_key, public_key);
        fpriv = fopen(argv[3], "w");
        if (!fpriv) die("fopen");
        fpub = fopen(argv[4], "w");
        if (!fpub) die("fopen");
        print_key_hex(fpriv, (KEY*)&private_key, sizeof(private_key));
        print_key_hex(fpub, (KEY*)&public_key, sizeof(public_key));

    } else if (!strcmp(argv[1], "-sign")) {
        fpriv = fopen(argv[3], "r");
        if (!fpriv) die("fopen");
        retval = scan_key_hex(fpriv, (KEY*)&private_key, sizeof(private_key));
        if (retval) die("scan_key_hex\n");
        signature.data = signature_buf;
        signature.len = 256;
        retval = sign_file(argv[2], private_key, signature);
        print_hex_data(stdout, signature);
    } else if (!strcmp(argv[1], "-sign_string")) {
        fpriv = fopen(argv[3], "r");
        if (!fpriv) die("fopen");
        retval = scan_key_hex(fpriv, (KEY*)&private_key, sizeof(private_key));
        if (retval) die("scan_key_hex\n");
        generate_signature(argv[2], cbuf, private_key);
        puts(cbuf);
    } else if (!strcmp(argv[1], "-verify")) {
        fpub = fopen(argv[4], "r");
        if (!fpub) die("fopen");
        retval = scan_key_hex(fpub, (KEY*)&public_key, sizeof(public_key));
        if (retval) die("read_public_key");
        f = fopen(argv[3], "r");
        signature.data = signature_buf;
        signature.len = 256;
        retval = scan_hex_data(f, signature);
        if (retval) die("scan_hex_data");
        retval = verify_file(argv[2], public_key, signature, is_valid);
        if (retval) die("verify_file");
        if (is_valid) {
            printf("file is valid\n");
        } else {
            printf("file is invalid\n");
            return 1;
        }
    } else if (!strcmp(argv[1], "-test_crypt")) {
        fpriv = fopen(argv[2], "r");
        if (!fpriv) die("fopen");
        retval = scan_key_hex(fpriv, (KEY*)&private_key, sizeof(private_key));
        if (retval) die("scan_key_hex\n");
        fpub = fopen(argv[3], "r");
        if (!fpub) die("fopen");
        retval = scan_key_hex(fpub, (KEY*)&public_key, sizeof(public_key));
        if (retval) die("read_public_key");
        strcpy((char*)buf2, "encryption test successful");
        in.data = buf2;
        in.len = strlen((char*)in.data);
        out.data = buf;
        encrypt_private(private_key, in, out);
        in = out;
        out.data = buf2;
        decrypt_public(public_key, in, out);
        printf("out: %s\n", out.data);
    } else {
        printf("unrecognized command\n");
        return 1;
    }
    return 0;
}

const char *BOINC_RCSID_6633b596b9 = "$Id: crypt_prog.C 15170 2008-05-09 21:27:22Z davea $";
