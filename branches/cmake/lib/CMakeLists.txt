# Synecdoche
# http://synecdoche.googlecode.com/
# Copyright (C) 2008 Nicolas Alvarez
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

IF(WIN32)
	SET(PLATFORM_LIB_SOURCES procinfo_win.C win_util.C proc_control.C diagnostics_win.C stackwalker_win.cpp)
ELSEIF(APPLE)
	SET(PLATFORM_LIB_SOURCES procinfo_mac.C)
ELSEIF(UNIX)
	SET(PLATFORM_LIB_SOURCES procinfo_unix.C)
ENDIF(WIN32)

ADD_LIBRARY(boinc STATIC
	util.C str_util.C
	shmem.C app_ipc.C
	coproc.C diagnostics.C proxy_info.C
	filesys.C mfile.C miofile.C network.C
	parse.C hostinfo.C prefs.C
	md5.c md5_file.C base64.C crypt.C
	gui_rpc_client.C gui_rpc_client_ops.C gui_rpc_client_print.C
	${PLATFORM_LIB_SOURCES})

IF(WIN32)
	TARGET_LINK_LIBRARIES(boinc wsock32)
ENDIF(WIN32)

ADD_EXECUTABLE(boinc_cmd boinc_cmd.C)
TARGET_LINK_LIBRARIES(boinc_cmd boinc)
