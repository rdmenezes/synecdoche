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

#ifndef TIME_STATS_H
#define TIME_STATS_H

#include <cstdio>

#include <vector>
#include <iosfwd>

class MIOFILE;

class TIME_STATS {
    bool first;
    int previous_connected_state;
public:
    double last_update;
    // we maintain an exponentially weighted average of these quantities:

    /// The fraction of total time this host runs the core client
    double on_frac;
    /// Of the time this host runs the core client,
    /// the fraction it is connected to the Internet,
    /// or -1 if not known
    double connected_frac;
    /// Of the time this host runs the core client,
    /// the fraction it is enabled to work
    /// (as determined by preferences, manual suspend/resume, etc.)
    double active_frac;
    /// The ratio between CPU time accumulated by BOINC apps
    /// and the wall time those apps are scheduled at the OS level.
    /// May be less than one if
    /// -# apps page or do I/O
    /// -# other CPU-intensive apps run
    double cpu_efficiency;

    double inactive_start;

    void update(int suspend_reason);
    void update_cpu_efficiency(double cpu_wall_time, double cpu_time);

    TIME_STATS();
    void write(std::ostream& out, bool to_server) const;
    int parse(MIOFILE&);
};

#endif
