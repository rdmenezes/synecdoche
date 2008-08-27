// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2005 University of California
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

#include "boinc_api.h"

#ifdef __cplusplus
extern "C" {
    extern int boinc_init_graphics_lib(WORKER_FUNC_PTR worker, char* argv0);
    extern void* graphics_lib_handle;
}

extern int boinc_init_options_graphics_lib(
    BOINC_OPTIONS&, WORKER_FUNC_PTR worker, char* argv0
);

#else
    extern int boinc_init_graphics_lib(WORKER_FUNC_PTR worker, char* argv0);
    extern void* graphics_lib_handle;
#endif
