## -*- mode: makefile; tab-width: 4 -*-
## $Id: Makefile.am 15375 2008-06-08 22:18:52Z davea $

include $(top_srcdir)/Makefile.incl

# (for a while we used "-static -static-libgcc" on linux, but this is obsolete
# now)
#STATIC_FLAGS=@STATIC_FLAGS@

LIBS += @CLIENTLIBS@

if OS_DARWIN
   LIBS += -framework IOKit -framework Foundation -framework ScreenSaver -framework Cocoa
endif

bin_PROGRAMS = synecd switcher

synecd_SOURCES = \
    acct_mgr.C \
    acct_mgr.h \
    acct_setup.C \
    acct_setup.h \
    app.C \
    app.h \
    app_control.C \
    app_graphics.C \
    app_start.C \
    check_state.C \
    client_msgs.C \
    client_msgs.h \
    client_state.C \
    client_state.h \
    client_types.C \
    client_types.h \
    cpp.h \
    cpu_benchmark.h \
    cpu_sched.C \
    cs_account.C \
    cs_apps.C \
    cs_benchmark.C \
    cs_cmdline.C \
    cs_files.C \
    cs_platforms.C \
    cs_prefs.C \
    cs_scheduler.C \
    cs_statefile.C \
    cs_trickle.C \
    dirs.h \
    dhrystone.C \
    dhrystone.h \
    dhrystone2.C \
    file_names.C \
    file_names.h \
    file_xfer.C \
    file_xfer.h \
    gui_http.C \
    gui_http.h \
    gui_rpc_server.C \
    gui_rpc_server.h \
    gui_rpc_server_ops.C \
    hostinfo_network.C \
    hostinfo_network.h \
    hostinfo_unix.C \
    http_curl.C \
    http_curl.h \
    log_flags.C \
    log_flags.h \
    main.C \
    main.h \
    net_stats.C \
    net_stats.h \
    pers_file_xfer.C \
    pers_file_xfer.h \
    rr_sim.cpp \
    rr_sim.h \
    sandbox.C \
    sandbox.h \
    scheduler_op.C \
    scheduler_op.h \
    time_stats.C \
    time_stats.h \
    whetstone.C \
    work_fetch.C

nodist_synecd_SOURCES = ../svnversion.cpp dirs.cpp

synecd_CPPFLAGS = $(AM_CPPFLAGS) -DHARDCODED_DIRS
synecd_LDADD = $(LIBBOINC) $(PTHREAD_LIBS)

synecddir = $(bindir)

switcher_SOURCES = switcher.C

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(localstatedir)/synecdoche

dirs.cpp: dirs.cpp.in Makefile
	sed -e 's,[@]sysconfdir[@],$(sysconfdir),g' \
		-e 's,[@]localstatedir[@],$(localstatedir),g' \
		< $(srcdir)/dirs.cpp.in > dirs.cpp

EXTRA_DIST = dirs.cpp.in \
    win \
    scripts

CLEANFILES = dirs.cpp
