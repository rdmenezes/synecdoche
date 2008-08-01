IF(WIN32)
	SET(WIN_LIB_SOURCES win_util.C)
ELSE(WIN32)
	SET(WIN_LIB_SOURCES)
ENDIF(WIN32)

ADD_LIBRARY(boinc STATIC
	util.C str_util.C
	shmem.C app_ipc.C
	coproc.C
	filesys.C mfile.C miofile.C network.C
	parse.C hostinfo.C prefs.C
	md5.c md5_file.C
	gui_rpc_client.C gui_rpc_client_ops.C gui_rpc_client_print.C
	${WIN_LIB_SOURCES})

IF(WIN32)
	TARGET_LINK_LIBRARIES(boinc wsock32)
ENDIF(WIN32)

ADD_EXECUTABLE(boinc_cmd boinc_cmd.C)
TARGET_LINK_LIBRARIES(boinc_cmd boinc)
