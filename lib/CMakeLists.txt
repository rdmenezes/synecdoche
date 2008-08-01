IF(WIN32)
	SET(PLATFORM_LIB_SOURCES win_util.C procinfo_win.C)
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
	md5.c md5_file.C
	gui_rpc_client.C gui_rpc_client_ops.C gui_rpc_client_print.C
	${PLATFORM_LIB_SOURCES})

IF(WIN32)
	TARGET_LINK_LIBRARIES(boinc wsock32)
ENDIF(WIN32)

ADD_EXECUTABLE(boinc_cmd boinc_cmd.C)
TARGET_LINK_LIBRARIES(boinc_cmd boinc)
