INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib)
ADD_EXECUTABLE(boinc_client main.C
	client_state.C acct_setup.C app_control.C check_state.C cpu_sched.C cs_account.C cs_apps.C cs_benchmark.C cs_cmdline.C cs_files.C cs_platforms.C cs_prefs.C cs_scheduler.C cs_statefile.C cs_trickle.C rrsim_test.C scheduler_op.C sim.C sim_util.C work_fetch.C
	)
#TARGET_LINK_LIBRARIES(boinc_api boinc)
