// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
// Copyright (C) 2009 University of California
//
// Synecdoche is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Synecdoche is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

/// \file
/// boinccmd: command-line interface to a BOINC core client,
/// using GUI RPCs.
///
/// usage: boinccmd [--host hostname] [--passwd passwd] command

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifdef _WIN32
#include "win_util.h"
#else
#include "config.h"
#include <cstring>
#include <unistd.h>
#endif

#include <iostream>
#include <string>
#include <vector>

#include "gui_rpc_client.h"
#include "error_numbers.h"
#include "util.h"
#include "str_util.h"
#include "version.h"
#include "common_defs.h"
#include "hostinfo.h"

void version(){
    std::cout << "syneccmd, built from Synecdoche " << SYNEC_VERSION_STRING << std::endl;
#if defined(_WIN32) && defined(USE_WINSOCK)
    WSACleanup();
#endif
    exit(0);
}

void usage() {
    std::cerr << "\n\
usage: syneccmd [--host hostname] [--passwd passwd] command\n\n\
Commands:\n\
 --lookup_account URL email passwd\n\
 --create_account URL email passwd name\n\
 --project_attach URL auth          attach to project\n\
 --join_acct_mgr URL name passwd    attach account manager\n\
 --quit_acct_mgr                    quit current account manager\n\
 --get_state                        show entire state\n\
 --get_results                      show results\n\
 --get_simple_gui_info              show status of projects and active results\n\
 --get_file_transfers               show file transfers\n\
 --get_project_status               show status of all attached projects\n\
 --get_disk_usage                   show disk usage\n\
 --get_proxy_settings\n\
 --get_messages [seqno]             show messages > seqno\n\
 --get_message_count                show number of messages in the queue\n\
 --get_host_info\n\
 --version, -V                      show core client version\n\
 --result url result_name op        job operation\n\
   op = suspend | resume | abort | graphics_window | graphics_fullscreen\n\
 --project URL op                   project operation\n\
   op = reset | detach | update | suspend | resume | nomorework | allowmorework\n\
 --file_transfer URL filename op    file transfer operation\n\
   op = retry | abort\n\
 --set_run_mode mode duration       set run mode for given duration\n\
   mode = always | auto | never\n\
 --set_network_mode mode duration\n\
 --set_proxy_settings\n\
 --run_benchmarks\n\
 --read_global_prefs_override\n\
 --quit\n\
 --read_cc_config\n\
 --set_debts URL1 std1 ltd1 [URL2 std2 ltd2 ...]\n\
 --get_project_config URL\n\
 --get_project_config_poll\n\
 --network_available\n\
 --get_cc_status\n\
";
#if defined(_WIN32) && defined(USE_WINSOCK)
    WSACleanup();
#endif
    exit(1);
}

void parse_display_args(const char** argv, int& i, DISPLAY_INFO& di) {
    di.window_station = "winsta0";
    di.desktop = "default";
    di.display = "";
    while (argv[i]) {
        if (!strcmp(argv[i], "--window_station")) {
            di.window_station = argv[++i];
        } else if (!strcmp(argv[i], "--desktop")) {
            di.desktop = argv[++i];
        } else if (!strcmp(argv[i], "--display")) {
            di.display = argv[++i];
        }
        ++i;
    }
}

void show_error(int retval) {
    std::cerr << "Error " << retval << ": " << boincerror(retval) << std::endl;
}

const char* next_arg(int argc, const char** argv, int& i) {
    if (i >= argc) {
        std::cerr << "Missing command-line argument" << std::endl;
        usage();
    }
    return argv[i++];
}

const char* prio_name(int prio) {
    switch (prio) {
    case 1: 
        return "low";
    case 2:
        return "medium";
    case 3:
        return "high";
    default:
        return "unknown";
    }
}

int main_impl(int argc, const char** argv) {
    RPC_CLIENT rpc;
    int retval, port=GUI_RPC_PORT;
    MESSAGES messages;
    char hostname_buf[256], *hostname=0;
    char *p;

#ifdef _WIN32
    chdir_to_data_dir();
#endif

#if defined(_WIN32) && defined(USE_WINSOCK)
    WSADATA wsdata;
    retval = WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
    if (retval) {
        std::cerr << "WinsockInitialize: " << retval << std::endl;
        exit(1);
    }
#endif
    if (argc < 2) usage();
    int i = 1;
    if (!strcmp(argv[i], "--help")) usage();
    if (!strcmp(argv[i], "-h"))     usage();
    if (!strcmp(argv[i], "--version")) version();
    if (!strcmp(argv[i], "-V"))     version();

    if (!strcmp(argv[i], "--host")) {
        if (++i == argc) usage();
        strlcpy(hostname_buf, argv[i], sizeof(hostname_buf));
        hostname = hostname_buf;
        p = strchr(hostname, ':');
        if (p) {
            port = atoi(p+1);
            *p=0;
        }
        i++;
    }

    std::string passwd;
    if ((i<argc)&& !strcmp(argv[i], "--passwd")) {
        if (++i == argc) usage();
        passwd = argv[i];
        i++;
    }

    if (passwd.empty()) {
        // No password given via command line, try to read the GUI-RPC-password-file:
        passwd = read_gui_rpc_password();
    }

    // change the following to debug GUI RPC's asynchronous connection mechanism
    //
#if 1
    retval = rpc.init(hostname, port);
    if (retval) {
        std::cerr << "can't connect to " << (hostname?hostname:"local host") << std::endl;
#if defined(_WIN32) && defined(USE_WINSOCK)
        WSACleanup();
#endif
        exit(1);
    }
#else
    retval = rpc.init_asynch(hostname, 60., false);
    while (1) {
        retval = rpc.init_poll();
        if (!retval) break;
        if (retval == ERR_RETRY) {
            std::cout << "sleeping" << std::endl;
            sleep(1);
            continue;
        }
        std::cerr << "can't connect: " << retval << std::endl;
#if defined(_WIN32) && defined(USE_WINSOCK)
        WSACleanup();
#endif
        exit(1);
    }
    std::cout << "connected" << std::endl;
#endif

    if (!passwd.empty()) {
        retval = rpc.authorize(passwd.c_str());
        if (retval) {
            std::cerr << "Authorization failure: " << retval << std::endl;
#if defined(_WIN32) && defined(USE_WINSOCK)
            WSACleanup();
#endif
            exit(1);
        }
    }

    const char* cmd = next_arg(argc, argv, i);
    if (!strcmp(cmd, "--get_state")) {
        CC_STATE state;
        retval = rpc.get_state(state);
        if (!retval) state.print();
    } else if (!strcmp(cmd, "--get_results")) {
        RESULTS results;
        retval = rpc.get_results(results);
        if (!retval) results.print();
    } else if (!strcmp(cmd, "--get_file_transfers")) {
        FILE_TRANSFERS ft;
        retval = rpc.get_file_transfers(ft);
        if (!retval) ft.print();
    } else if (!strcmp(cmd, "--get_project_status")) {
        PROJECTS ps;
        retval = rpc.get_project_status(ps);
        if (!retval) ps.print();
    } else if (!strcmp(cmd, "--get_simple_gui_info")) {
        SIMPLE_GUI_INFO sgi;
        retval = rpc.get_simple_gui_info(sgi);
        if (!retval) sgi.print();
    } else if (!strcmp(cmd, "--get_disk_usage")) {
        DISK_USAGE du;
        retval = rpc.get_disk_usage(du);
        if (!retval) du.print();
    } else if (!strcmp(cmd, "--result")) {
        RESULT result;
        const char* project_url = next_arg(argc, argv, i);
        result.project_url = project_url;
        const char* name = next_arg(argc, argv, i);
        result.name = name;
        const char* op = next_arg(argc, argv, i);
        if (!strcmp(op, "suspend")) {
            retval = rpc.result_op(result, "suspend");
        } else if (!strcmp(op, "resume")) {
            retval = rpc.result_op(result, "resume");
        } else if (!strcmp(op, "abort")) {
            retval = rpc.result_op(result, "abort");
        } else if (!strcmp(op, "graphics_window")) {
            DISPLAY_INFO di;
            parse_display_args(argv, i, di);
            retval = rpc.show_graphics(project_url, name, MODE_WINDOW, di);
        } else if (!strcmp(op, "graphics_fullscreen")) {
            DISPLAY_INFO di;
            parse_display_args(argv, i, di);
            retval = rpc.show_graphics(project_url, name, MODE_FULLSCREEN, di);
        } else {
            std::cerr << "Unknown op " << op << std::endl;
        }
    } else if (!strcmp(cmd, "--project")) {
        PROJECT project;
        project.master_url =  next_arg(argc, argv, i);
        canonicalize_master_url(project.master_url);
        const char* op = next_arg(argc, argv, i);
        if (!strcmp(op, "reset")) {
            retval = rpc.project_op(project, "reset");
        } else if (!strcmp(op, "suspend")) {
            retval = rpc.project_op(project, "suspend");
        } else if (!strcmp(op, "resume")) {
            retval = rpc.project_op(project, "resume");
        } else if (!strcmp(op, "detach")) {
            retval = rpc.project_op(project, "detach");
        } else if (!strcmp(op, "update")) {
            retval = rpc.project_op(project, "update");
        } else if (!strcmp(op, "suspend")) {
            retval = rpc.project_op(project, "suspend");
        } else if (!strcmp(op, "resume")) {
            retval = rpc.project_op(project, "resume");
        } else if (!strcmp(op, "nomorework")) {
            retval = rpc.project_op(project, "nomorework");
        } else if (!strcmp(op, "allowmorework")) {
            retval = rpc.project_op(project, "allowmorework");
        } else if (!strcmp(op, "detach_when_done")) {
            retval = rpc.project_op(project, "detach_when_done");
        } else if (!strcmp(op, "dont_detach_when_done")) {
            retval = rpc.project_op(project, "dont_detach_when_done");
        } else {
            std::cerr << "Unknown op " << op << std::endl;
        }
    } else if (!strcmp(cmd, "--project_attach")) {
        std::string url(next_arg(argc, argv, i));
        canonicalize_master_url(url);
        const char* auth = next_arg(argc, argv, i);
        retval = rpc.project_attach(url.c_str(), auth, "");
    } else if (!strcmp(cmd, "--file_transfer")) {
        FILE_TRANSFER ft;

        ft.project_url = next_arg(argc, argv, i);
        ft.name = next_arg(argc, argv, i);
        const char* op = next_arg(argc, argv, i);
        if (!strcmp(op, "retry")) {
            retval = rpc.file_transfer_op(ft, "retry");
        } else if (!strcmp(op, "abort")) {
            retval = rpc.file_transfer_op(ft, "abort");
        } else {
            std::cerr << "Unknown op " << op << std::endl;
        }
    } else if (!strcmp(cmd, "--set_run_mode")) {
        const char* op = next_arg(argc, argv, i);
        double duration;
        if (i >= argc || (argv[i][0] == '-')) {
            duration = 0;
        } else {
            duration = atof(next_arg(argc, argv, i));
        }
        if (!strcmp(op, "always")) {
            retval = rpc.set_run_mode(RUN_MODE_ALWAYS, duration);
        } else if (!strcmp(op, "auto")) {
            retval = rpc.set_run_mode(RUN_MODE_AUTO, duration);
        } else if (!strcmp(op, "never")) {
            retval = rpc.set_run_mode(RUN_MODE_NEVER, duration);
        } else {
            std::cerr << "Unknown op " << op << std::endl;
        }
    } else if (!strcmp(cmd, "--set_network_mode")) {
        const char* op = next_arg(argc, argv, i);
        double duration;
        if (i >= argc || (argv[i][0] == '-')) {
            duration = 0;
        } else {
            duration = atof(next_arg(argc, argv, i));
        }
        if (!strcmp(op, "always")) {
            retval = rpc.set_network_mode(RUN_MODE_ALWAYS, duration);
        } else if (!strcmp(op, "auto")) {
            retval = rpc.set_network_mode(RUN_MODE_AUTO, duration);
        } else if (!strcmp(op, "never")) {
            retval = rpc.set_network_mode(RUN_MODE_NEVER, duration);
        } else {
            std::cerr << "Unknown op " << op << std::endl;
        }
    } else if (!strcmp(cmd, "--get_proxy_settings")) {
        GR_PROXY_INFO pi;
        retval = rpc.get_proxy_settings(pi);
        if (!retval) pi.print();
    } else if (!strcmp(cmd, "--set_proxy_settings")) {
        GR_PROXY_INFO pi;
        pi.http_server_name = next_arg(argc, argv, i);
        pi.http_server_port = atoi(next_arg(argc, argv, i));
        pi.http_user_name = next_arg(argc, argv, i);
        pi.http_user_passwd = next_arg(argc, argv, i);
        pi.socks_server_name = next_arg(argc, argv, i);
        pi.socks_server_port = atoi(next_arg(argc, argv, i));
        pi.socks_version = atoi(next_arg(argc, argv, i));
        pi.socks5_user_name = next_arg(argc, argv, i);
        pi.socks5_user_passwd = next_arg(argc, argv, i);
        pi.use_http_proxy = !pi.http_server_name.empty();
        pi.use_http_authentication = !pi.http_user_name.empty();
        pi.use_socks_proxy = !pi.socks_server_name.empty();
        retval = rpc.set_proxy_settings(pi);
    } else if (!strcmp(cmd, "--get_messages")) {
        int seqno = 0;
        if (i != argc) {
            seqno = atoi(next_arg(argc, argv, i));
        }
        retval = rpc.get_messages(seqno, messages);
        if (!retval) {
            for (std::vector<MESSAGE*>::const_iterator m = messages.messages.begin();
                            m != messages.messages.end(); ++m) {
                MESSAGE& md = **m;
                strip_whitespace(md.body);
                std::cout << md.seqno << ":" << time_to_string(md.timestamp)
                          << " (" << prio_name(md.priority) << ") ";
                if (!md.project.empty()) {
                    std::cout << "[" << md.project << "] ";
                }
                std::cout << md.body << "\n";
            }
            std::cout << std::flush;
        }
    } else if (!strcmp(cmd, "--get_message_count")) {
        int msg_count;
        retval = rpc.get_message_count(msg_count);
        if (!retval) {
            std::cout << "Number of messages in the queue: " << msg_count << std::endl;
        }
    } else if (!strcmp(cmd, "--get_host_info")) {
        HOST_INFO hi;
        retval = rpc.get_host_info(hi);
        if (!retval) hi.print();
    } else if (!strcmp(cmd, "--join_acct_mgr")) {
        const char* am_url = next_arg(argc, argv, i);
        const char* am_name = next_arg(argc, argv, i);
        const char* am_passwd = next_arg(argc, argv, i);
        retval = rpc.acct_mgr_rpc(am_url, am_name, am_passwd);
        if (!retval) {
            while (1) {
                ACCT_MGR_RPC_REPLY amrr;
                retval = rpc.acct_mgr_rpc_poll(amrr);
                if (retval) {
                    std::cout << "poll status: " << boincerror(retval) << std::endl;
                } else {
                    if (amrr.error_num) {
                        std::cout << "poll status: " << boincerror(amrr.error_num) << std::endl;
                        if (amrr.error_num != ERR_IN_PROGRESS) break;
                        boinc_sleep(1);
                    } else {
                        size_t n = amrr.messages.size();
                        if (n) {
                            std::cout << "Messages from account manager:\n";
                            for (size_t j=0; j<n; j++) {
                                std::cout << amrr.messages[j] << '\n';
                            }
                            std::cout << std::flush;
                        }
                        break;
                    }
                }
            }
        }
    } else if (!strcmp(cmd, "--quit_acct_mgr")) {
        retval = rpc.acct_mgr_rpc("", "", "");
    } else if (!strcmp(cmd, "--run_benchmarks")) {
        retval = rpc.run_benchmarks();
    } else if (!strcmp(cmd, "--get_project_config")) {
        const char* gpc_url = next_arg(argc, argv,i);
        retval = rpc.get_project_config(std::string(gpc_url));
    } else if (!strcmp(cmd, "--get_project_config_poll")) {
        PROJECT_CONFIG pc;
        retval = rpc.get_project_config_poll(pc);
        if (retval) {
            std::cout << "retval: " << retval << std::endl;
        } else {
            pc.print();
        }
    } else if (!strcmp(cmd, "--lookup_account")) {
        ACCOUNT_IN lai;
        lai.url = next_arg(argc, argv, i);
        lai.email_addr = next_arg(argc, argv, i);
        lai.passwd = next_arg(argc, argv, i);
        retval = rpc.lookup_account(lai);
        std::cout << "status: " << boincerror(retval) << std::endl;
        if (!retval) {
            ACCOUNT_OUT lao;
            while (1) {
                retval = rpc.lookup_account_poll(lao);
                if (retval) {
                    std::cout << "poll status: " << boincerror(retval) << std::endl;
                } else {
                    if (lao.error_num) {
                        std::cout << "poll status: " << boincerror(lao.error_num) << std::endl;
                        if (lao.error_num != ERR_IN_PROGRESS) break;
                        boinc_sleep(1);
                    } else {
                        lao.print();
                        break;
                    }
                }
            }
        }
    } else if (!strcmp(cmd, "--create_account")) {
        ACCOUNT_IN cai;
        cai.url = next_arg(argc, argv, i);
        cai.email_addr = next_arg(argc, argv, i);
        cai.passwd = next_arg(argc, argv, i);
        cai.user_name = next_arg(argc, argv, i);
        retval = rpc.create_account(cai);
        std::cout << "status: " << boincerror(retval) << std::endl;
        if (!retval) {
            ACCOUNT_OUT lao;
            while (1) {
                retval = rpc.create_account_poll(lao);
                if (retval) {
                    std::cout << "poll status: " << boincerror(retval) << std::endl;
                } else {
                    if (lao.error_num) {
                        std::cout << "poll status: " << boincerror(lao.error_num) << std::endl;
                        if (lao.error_num != ERR_IN_PROGRESS) break;
                        boinc_sleep(1);
                    } else {
                        lao.print();
                        break;
                    }
                }
            }
        }
    } else if (!strcmp(cmd, "--read_global_prefs_override")) {
        retval = rpc.read_global_prefs_override();
    } else if (!strcmp(cmd, "--read_cc_config")) {
        retval = rpc.read_cc_config();
        std::cout << "retval: " << retval << std::endl;
    } else if (!strcmp(cmd, "--network_available")) {
        retval = rpc.network_available();
    } else if (!strcmp(cmd, "--get_cc_status")) {
        CC_STATUS cs;
        retval = rpc.get_cc_status(cs);
        if (!retval) {
            retval = cs.network_status;
        }
    } else if (!strcmp(cmd, "--set_debts")) {
        std::vector<PROJECT>projects;
        while (i < argc) {
            PROJECT proj;
            proj.master_url = std::string(next_arg(argc, argv, i));
            proj.short_term_debt = atoi(next_arg(argc, argv, i));
            proj.long_term_debt = atoi(next_arg(argc, argv, i));
            projects.push_back(proj);
        }
        retval = rpc.set_debts(projects);
    } else if (!strcmp(cmd, "--quit")) {
        retval = rpc.quit();
    } else {
        std::cerr << "unrecognized command " << cmd << std::endl;
    }
    if (retval < 0) {
        show_error(retval);
    }

#if defined(_WIN32) && defined(USE_WINSOCK)
    WSACleanup();
#endif
    return retval;
}

int main(int argc, const char** argv) {
    try {
        return main_impl(argc, argv);
    } catch (std::exception& ex) {
        std::cerr << "An error occured: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "An unspecified error occured. Aborting." << std::endl;
    }
    return 1;
}
