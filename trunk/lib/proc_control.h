// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 University of California
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
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

extern HANDLE sandbox_account_interactive_token;
extern HANDLE sandbox_account_service_token;

extern void get_sandbox_account_interactive_token();
extern void get_sandbox_account_service_token();

extern int run_app_windows(
    const char* path, const char* cdir, int argc, char *const argv[], HANDLE&
);
