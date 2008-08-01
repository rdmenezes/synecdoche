INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib)
ADD_EXECUTABLE(boinc_client main.C
	client_state.C acct_setup.C app_control.C check_state.C cpu_sched.C cs_account.C cs_apps.C cs_benchmark.C cs_cmdline.C cs_files.C cs_platforms.C cs_prefs.C cs_scheduler.C cs_statefile.C cs_trickle.C scheduler_op.C work_fetch.C
	client_types.C app.C file_names.C
	gui_rpc_server.C gui_rpc_server_ops.C
	client_msgs.C log_flags.C
	file_xfer.C pers_file_xfer.C http_curl.C gui_http.C 
	sandbox.C net_stats.C app_start.C auto_update.C
	whetstone.C dhrystone.C dhrystone2.C
	)
TARGET_LINK_LIBRARIES(boinc_client boinc)
