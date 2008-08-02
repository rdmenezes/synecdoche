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
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/win)
	SET(PLATFORM_CLIENT_SOURCES hostinfo_win.C)
ELSEIF(UNIX)
	SET(PLATFORM_CLIENT_SOURCES hostinfo_unix.C)
ENDIF(WIN32)

ADD_EXECUTABLE(boinc_client main.C
	client_state.C acct_setup.C app_control.C check_state.C cpu_sched.C cs_account.C cs_apps.C cs_benchmark.C cs_cmdline.C cs_files.C cs_platforms.C cs_prefs.C cs_scheduler.C cs_statefile.C cs_trickle.C scheduler_op.C work_fetch.C
	client_types.C app.C file_names.C app_start.C acct_mgr.C
	gui_rpc_server.C gui_rpc_server_ops.C
	client_msgs.C log_flags.C
	file_xfer.C pers_file_xfer.C http_curl.C gui_http.C 
	sandbox.C auto_update.C
	net_stats.C time_stats.C hostinfo_network.C
	whetstone.C dhrystone.C dhrystone2.C
	app_graphics.C
	${PLATFORM_CLIENT_SOURCES}
	)
TARGET_LINK_LIBRARIES(boinc_client boinc)

TARGET_LINK_LIBRARIES(boinc_client ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES})
