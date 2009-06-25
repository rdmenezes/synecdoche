// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
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

#ifndef RR_SIM_H
#define RR_SIM_H

#include <vector>

/// assume actual CPU utilization will be this multiple
/// of what we've actually measured recently
#define CPU_PESSIMISM_FACTOR 0.9

class RESULT;

class RR_SIM_PROJECT_STATUS {
private:
    std::vector<RESULT*>active;     ///< jobs currently running (in simulation
    std::vector<RESULT*>pending;    ///< jobs runnable but not running yet
    int deadlines_missed;

    /// Fraction of each CPU this project will get
    /// set in CLIENT_STATE::rr_misses_deadline();
    double proc_rate;

    double cpu_shortfall;

public:
    RR_SIM_PROJECT_STATUS();

    void clear();
    int get_deadlines_missed() const;
    void inc_deadlines_missed(int increment = 1);
    double get_proc_rate() const;
    void set_proc_rate(double proc_rate);
    double get_cpu_shortfall() const;
    void set_cpu_shortfall(double cpu_shortfall);
    void add_to_cpu_shortfall(double delta);
    size_t cpus_used() const;
    bool can_run(const RESULT* r, int ncpus) const;
    void activate(RESULT* rp);
    void add_pending(RESULT* rp);
    bool none_active() const;
    void remove_active(const RESULT* r);
    RESULT* get_pending();
};

#endif // RR_SIM_H
