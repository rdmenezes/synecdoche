// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2006 University of California
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

//  SecurityUtility.cpp

#include <sys/param.h>  // for MAXPATHLEN
#include <unistd.h>     // for getwd, getlogin

#include <Carbon/Carbon.h>

#include "SetupSecurity.h"

// Standalone utility to set up BOINC security owners, groups, permissions

int main(int argc, char *argv[]) {
    OSStatus            err;
    char boincPath[MAXPATHLEN];
    
    err = CreateBOINCUsersAndGroups();
    if (err != noErr)
        return err;

    err = AddAdminUserToGroups(getlogin());
    if (err != noErr)
        return err;
    
    boincPath[0] = 0;
    getwd(boincPath);
    //ShowSecurityError("Current Working Directory is %s", wd);

    strlcat(boincPath, "/Synecdoche.app", sizeof(boincPath));
    err = SetBOINCAppOwnersGroupsAndPermissions(boincPath);
    if (err != noErr)
        return err;

    err = SetBOINCDataOwnersGroupsAndPermissions();
    return err;
}

