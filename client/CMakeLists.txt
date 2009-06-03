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
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/win)
	SET(PLATFORM_CLIENT_SOURCES hostinfo_win.C win/win_service.cpp)
	ADD_DEFINITIONS(-D_CONSOLE)
ELSEIF(UNIX)
	SET(PLATFORM_CLIENT_SOURCES hostinfo_unix.C)
ENDIF(WIN32)

ADD_EXECUTABLE(synecd
	acct_mgr.C
	acct_setup.C
	app.C
	app_control.C
	app_graphics.C
	app_start.C
	check_state.C
	client_msgs.C
	client_state.C
	client_types.C
	cpu_sched.C
	cs_account.C
	cs_apps.C
	cs_benchmark.C
	cs_cmdline.C
	cs_files.C
	cs_platforms.C
	cs_prefs.C
	cs_scheduler.C
	cs_statefile.C
	cs_trickle.C
	dhrystone2.C
	dhrystone.C
	file_names.C
	file_xfer.C
	gui_http.C
	gui_rpc_server.C
	gui_rpc_server_ops.C
	hostinfo_network.C
	http_curl.C
	log_flags.C
	main.C
	net_stats.C
	pers_file_xfer.C
	sandbox.C
	scheduler_op.C
	time_stats.C
	whetstone.C
	work_fetch.C
	${PLATFORM_CLIENT_SOURCES}
)
TARGET_LINK_LIBRARIES(synecd boinc)

TARGET_LINK_LIBRARIES(synecd ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES})

IF(WIN32)
	TARGET_LINK_LIBRARIES(synecd sensapi.lib)
ELSEIF(APPLE)
	FIND_LIBRARY(CF_LIBRARY CoreFoundation)
	FIND_LIBRARY(COCOA_LIBRARY Cocoa)
	FIND_LIBRARY(IOKIT_LIBRARY IOKit)
	TARGET_LINK_LIBRARIES(synecd ${CF_LIBRARY} ${COCOA_LIBRARY} ${IOKIT_LIBRARY})
ENDIF(WIN32)
