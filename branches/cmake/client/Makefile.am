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
    acct_setup.C \
    app.C \
    app_control.C \
    app_graphics.C \
    app_start.C \
    check_state.C \
    client_msgs.C \
    client_state.C \
    client_types.C \
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
    dhrystone.C \
    dhrystone2.C \
    file_names.C \
    file_xfer.C \
    gui_http.C \
    gui_rpc_server.C \
    gui_rpc_server_ops.C \
    hostinfo_network.C \
    hostinfo_unix.C \
    http_curl.C \
    log_flags.C \
    main.C \
    net_stats.C \
    pers_file_xfer.C \
    sandbox.C \
    scheduler_op.C \
    time_stats.C \
    whetstone.C \
    work_fetch.C

synecd_DEPENDENCIES =
synecd_CPPFLAGS = -O3 -fomit-frame-pointer -fforce-addr -ffast-math $(AM_CPPFLAGS)
synecd_LDFLAGS = -static-libgcc
synecd_LDADD = $(LIBBOINC) $(PTHREAD_LIBS)

synecddir = $(bindir)
synecd_DATA = ../curl/ca-bundle.crt

switcher_SOURCES = switcher.C

all-local: synecd

## these source files need to be specified because no rule uses them.

EXTRA_DIST = *.h \
    mac \
    translation \
    win
