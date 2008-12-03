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

// test program for message queue functions

// -d       destroy
// -c       create
// -s [msg] send message [msg]
// -r       receive message
// -rw      wait for message

#define KEY 0xb01fcafe

#include "config.h"
#include <cstring>
#include <cstdio>

#include "msg_queue.h"

struct my_msg {
    long msg_type;
    char msg_text[256];
};

int main(int argc, char** argv) {
    my_msg the_msg;
    int retval;

    if (!strcmp(argv[1], "-d")) {
        destroy_message_queue(KEY);
    } else if (!strcmp(argv[1], "-c")) {
        create_message_queue(KEY);
    } else if (!strcmp(argv[1], "-rw")) {
        retval = receive_message(KEY, &the_msg, sizeof(my_msg),true);
        printf("Received message: %s\n", the_msg.msg_text);
    } else if (!strcmp(argv[1], "-r")) {
        retval = receive_message(KEY, &the_msg, sizeof(my_msg),false);
        if (!retval) printf("Received message: %s\n", the_msg.msg_text);
    } else if (!strcmp(argv[1], "-s")) {
        the_msg.msg_type = 1;
        strcpy(the_msg.msg_text, argv[2]);
        send_message(KEY, &the_msg, sizeof(my_msg),true);
    }

    return 0;
}

const char *BOINC_RCSID_7ce3817f4c = "$Id: msg_test.C 13804 2007-10-09 11:35:47Z fthomas $";
