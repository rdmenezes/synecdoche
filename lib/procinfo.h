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

#ifndef _PROCINFO_
#define _PROCINFO_

#include <vector>

struct PROCINFO {
    int id;
    int parentid;
    double swap_size;
    double working_set_size;
    double working_set_size_smoothed;
    unsigned long page_fault_count;
    double user_time;
    double kernel_time;
    bool is_boinc_app;

    double page_fault_rate;     ///< derived by higher-level code
};

int procinfo_setup(std::vector<PROCINFO>&);
    // call this first to get data structure
void procinfo_app(PROCINFO&, std::vector<PROCINFO>&);
    // call this to get mem usage for a given app
    // (marks process as BOINC)
void procinfo_other(PROCINFO&, std::vector<PROCINFO>&);
    // After getting mem usage for all BOINC apps,
    // call this to get mem usage for everything else

#endif
