# This file is part of Synecdoche.
# http://synecdoche.googlecode.com/
# Copyright (C) 2008 Nicolas Alvarez
#
# Synecdoche is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Synecdoche is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

IF(WIN32)
    SET(PLATFORM_LIB_SOURCES
        diagnostics_win.C
        proc_control.C
        procinfo_win.C
        stackwalker_win.cpp
        win_util.C
    )
ELSEIF(APPLE)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/mac)
    SET(PLATFORM_LIB_SOURCES
        procinfo_mac.C
    )
ELSEIF(UNIX)
    SET(PLATFORM_LIB_SOURCES procinfo_unix.C)
ENDIF(WIN32)

ADD_LIBRARY(boinc STATIC
    app_ipc.C
    base64.C
    crypt.C
    diagnostics.C
    filesys.C
    gui_rpc_client.C
    gui_rpc_client_ops.C
    gui_rpc_client_print.C
    hostinfo.C
    md5.c
    md5_file.C
    mfile.C
    miofile.C
    network.C
    parse.C
    prefs.C
    proxy_info.C
    shmem.C
    str_util.C
    util.C
    ${PLATFORM_LIB_SOURCES}
)

IF(WIN32)
    TARGET_LINK_LIBRARIES(boinc wsock32)
ENDIF(WIN32)

ADD_EXECUTABLE(syneccmd boinc_cmd.C)
TARGET_LINK_LIBRARIES(syneccmd boinc)

ADD_SUBDIRECTORY(tests)
