ADD_LIBRARY(boinc STATIC
	util.C str_util.C
	filesys.C mfile.C miofile.C network.C
	parse.C hostinfo.C prefs.C
	md5.c md5_file.C
	gui_rpc_client.C gui_rpc_client_ops.C gui_rpc_client_print.C)
ADD_EXECUTABLE(boinc_cmd boinc_cmd.C)
TARGET_LINK_LIBRARIES(boinc_cmd boinc)
