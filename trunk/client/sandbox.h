// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Peter Kortschack
// Copyright (C) 2007 University of California
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

#ifndef SANDBOX_H
#define SANDBOX_H

extern void kill_via_switcher(int pid);
extern int get_project_gid();
extern int set_to_project_group(const char* path);

/// Run an utility program.
extern int switcher_exec(const char* util_filename, const char* cmdline);

/// Recursively delete everything in the specified directory.
extern int client_clean_out_dir(const char* dirpath);

/// Delete the file located at path.
extern int delete_project_owned_file(const char* path, bool retry);

extern int remove_project_owned_dir(const char* name);
extern int check_security(int use_sandbox, int isManager);

#define BOINC_PROJECT_GROUP_NAME "boinc_project"

extern bool g_use_sandbox;

#endif // SANDBOX_H
