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

// setprojectgrp.C
//
// When run as
// setprojectgrp Path
// sets group of file at Path to boinc_project
//
// setprojectgrp runs setuid boinc_master and setgid boinc_project

#include <unistd.h>
#include <grp.h>
#include <stdio.h>
#include <cerrno>
#include <sys/stat.h>

int main(int argc, char** argv) {
    gid_t       project_gid;
    int         retval = 0;
    struct stat sbuf;
    
    project_gid = getegid();

#if 0           // For debugging
    fprintf(stderr, "setprojectgrp argc=%d, arg[1]= %s, boinc_project gid = %d\n", argc, argv[1], project_gid);
    fflush(stderr);
#endif

    // chown() doesn't change ownershp of symbolic links; it follows the link and 
    // changes the file is not available in OS 10.3.9.
    //
    // But we don't really need to worry about this, because the system ignores 
    // ownership & permissions of symbolic links anyway.
    //
    // Also, the target of a symbolic link may not be present if the slot containing 
    // the link is no longer in use.
    //
    if (lstat(argv[1], &sbuf) == 0) {
        if (!S_ISLNK(sbuf.st_mode)) {
            retval = chown(argv[1], (uid_t)-1, project_gid);
            if (retval)
                fprintf(stderr, "chown(%s, -1, %d) failed: errno=%d\n", argv[1], project_gid, errno);
        }
    }
    return retval;
}
