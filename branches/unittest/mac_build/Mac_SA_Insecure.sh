#! /bin/sh

# Berkeley Open Infrastructure for Network Computing
# http://boinc.berkeley.edu
# Copyright (C) 2006 University of California
#
# This is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation;
# either version 2.1 of the License, or (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# To view the GNU Lesser General Public License visit
# http://www.gnu.org/copyleft/lesser.html
# or write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

# Mac_SA_Insecure.sh user group
#
# Undo making a Macintosh BOINC installation secure.  
# - Set file/dir ownership to the specified user and group
# - Remove BOINC groups and users
#
# IMPORTANT NOTE: earlier versions of the Mac_SA_Insecure.sh and 
# Mac_SA_Secure.sh scripts had serious problems when run under OS 10.3.x.
# They sometimes created bad users and groups with IDs that were duplicates 
# of other users and groups.  They ran correctly under OS 10.4.x
#
# If you ran an older version of either script under OS 10.3.x, you should 
# first run the current version of Mac_SA_Insecure.sh to delete the bad 
# entries and then run Mac_SA_Secure.sh to create new good entries.
#
#
# Execute this as root in the BOINC directory:
# cd {path_to_boinc_directory}
# sudo sh {path}/Mac_SA_Insecure.sh user group
#
# After running this script, the boinc client must be run with 
# the --insecure option.
# NOTE: running BOINC with security disabled is not recommended.
#
# Last updated 10/2/07

function remove_boinc_users() {
    name=$(dscl . search /users RecordName boinc_master | cut -f1 -s)
    if [ "$name" = "boinc_master" ] ; then
        sudo dscl . -delete /users/boinc_master
    fi

    name=$(dscl . search /groups RecordName boinc_master | cut -f1 -s)
    if [ "$name" = "boinc_master" ] ; then
        sudo dscl . -delete /groups/boinc_master
    fi
    
    name=$(dscl . search /users RecordName boinc_project | cut -f1 -s)
    if [ "$name" = "boinc_project" ] ; then
        sudo dscl . -delete /users/boinc_project
    fi

    name=$(dscl . search /groups RecordName boinc_project | cut -f1 -s)
    if [ "$name" = "boinc_project" ] ; then
        sudo dscl . -delete /groups/boinc_project
    fi
}

function check_login() {
    if [ `whoami` != 'root' ]
    then
        echo 'This script must be run as root'
        exit
    fi
}

check_login

if [ $# -eq 2 ]
then
    user=$1
    group=$2
else
    echo "usage: $0 user group"
    exit
fi

echo "Changing directory $(pwd) file ownership to user $user and group $group - OK? (y/n)"
read line
if [ "$line" != "y" ]
then
    exit
fi

if [ ! -f "synecd" ]
then
    echo "Can't find Synecdoche daemon in directory $(pwd); exiting"
    exit
fi

chown -R ${user}:${group} .
chmod -R u+rw-s,g+r-w-s,o+r-w .
chmod 600 gui_rpc_auth.cfg

if [ -f switcher/AppStats ] ; then 
# AppStats application must run setuid root (used in BOINC 5.7 through 5.8.14 only)
chown root:${group} switcher/AppStats
chmod 4550 switcher/AppStats
fi

if [ -x /Applications/Synecdoche.app/Contents/MacOS/Synecdoche ] ; then 
    chown ${user}:${group} /Applications/Synecdoche.app/Contents/MacOS/Synecdoche
    chmod -R u+r-w+s,g+r-ws,o+r-ws /Applications/Synecdoche.app/Contents/MacOS/Synecdoche
fi

if [ -x /Applications/Synecdoche.app/Contents/Resources/synecd ] ; then 
    chown ${user}:${group} /Applications/Synecdoche.app/Contents/Resources/synecd
    chmod -R u+r-ws,g+r-ws,o+r-ws /Applications/Synecdoche.app/Contents/Resources/synecd
fi

# Version 6 screensaver has its own embedded switcher application, but older versions don't.
if [ -x "/Library/Screen Savers/Synecdoche.saver/Contents/Resources/gfx_switcher" ] ; then 
    chown ${user}:${group} "/Library/Screen Savers/Synecdoche.saver/Contents/Resources/gfx_switcher"
    chmod -R u+r-ws,g+r-ws,o+r-ws "/Library/Screen Savers/Synecdoche.saver/Contents/Resources/gfx_switcher"
fi

remove_boinc_users