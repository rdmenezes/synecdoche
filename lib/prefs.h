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

#ifndef _PREFS_
#define _PREFS_

#include <cstdio>

class MIOFILE;
class XML_PARSER;

// global prefs are maintained as follows:
// 1) a "global_prefs.xml" file, which stores the "network" prefs;
//      it's maintained by communication with scheduling servers
//      or project managers
// 2) a "global_prefs_override.xml" file, which can be edited manually
//      or via a GUI.
//      For the prefs that it specifies, it overrides the network prefs.

// A struct with one bool per pref.
// This is passed in GUI RPCs (get/set_global_prefs_override_struct)
// to indicate which prefs are (or should be) specified in the override file
//
struct GLOBAL_PREFS_MASK {
    bool run_on_batteries;
    bool run_if_user_active;
    bool idle_time_to_run;
    bool suspend_if_no_recent_input;
    bool start_hour;
    bool end_hour;
    bool net_start_hour;
    bool net_end_hour;
    bool leave_apps_in_memory;
    bool confirm_before_connecting;
    bool hangup_if_dialed;
    bool dont_verify_images;
    bool work_buf_min_days;
    bool work_buf_additional_days;
    bool max_cpus;
    bool max_ncpus_pct;
    bool cpu_scheduling_period_minutes;
    bool disk_interval;
    bool disk_max_used_gb;
    bool disk_max_used_pct;
    bool disk_min_free_gb;
    bool vm_max_used_frac;
    bool ram_max_used_busy_frac;
    bool ram_max_used_idle_frac;
    bool max_bytes_sec_up;
    bool max_bytes_sec_down;
    bool cpu_usage_limit;

    GLOBAL_PREFS_MASK();
    void clear();
    bool are_prefs_set();
    bool are_simple_prefs_set();
    void set_all();
};

/// This class represents a time span on a single day, e.g. from 0:00 to 24:00.
class TIME_SPAN {
public:
    enum TimeMode {
        Always = 7000,
        Never,
        Between
    };
    
    /// Construct a TIME_SPAN instance.
    TIME_SPAN();
    
    /// Construct a TIME_SPAN instance.
    TIME_SPAN(time_t start, time_t end);
    
    TIME_SPAN& operator=(const TIME_SPAN& r);
    bool operator==(const TIME_SPAN& r);

    /// Check if the client should be suspended for a given point in time.
    bool suspended(const time_t point_in_time) const;
    
    /// Get the run mode determined by this TIME_SPAN instance.
    TimeMode mode() const;
    
    /// Reset this instance to its default values.
    void clear();
    
    time_t get_start() const;
    time_t get_end() const;
    
    void set_start(const time_t start);
    void set_end(const time_t end);

private:
    void check_range(const time_t point_in_time) const;

private:
    time_t m_start; ///< Start of the time span in seconds after midnight.
    time_t m_end; ///< End of the time span in seconds after midnight.
    
};

/// This class maintains time spans for a whole week with one time span for each day.
class WEEK_PREFS {
public:
    WEEK_PREFS();
    WEEK_PREFS(const WEEK_PREFS& original);
    ~WEEK_PREFS();

    /// Get the set time span for a given day.
    const TIME_SPAN* get(int day) const;
    
    /// Set a time span in which the client is allowed to run.
    void set(int day, time_t start, time_t end);
    
    /// Set a time span in which the client is allowed to run.
    void set(int day, const TIME_SPAN& time);
    
    /// Remove the timespan for a given day.
    void unset(int day);
    
    /// Reset this instance to its default values.
    void clear();
    
    WEEK_PREFS& operator=(const WEEK_PREFS& rhs);

private:
    /// Create a deep copy.
    void copy(const WEEK_PREFS& original);
    
    TIME_SPAN* days[7];

};

/// This maintains time spans for scheduling the computing and networking activity.
/// It handles one general time span which applies to every day and additional time spans, one
/// for each day of a week.
class TIME_PREFS : public TIME_SPAN {
public:
    TIME_PREFS();
    TIME_PREFS(time_t start, time_t end);
    
    /// Reset this instance to its default values.
    void        clear();
    
    /// Check if the client should be currently suspended based on this TIME_PREFS instance.
    bool        suspended() const;
    
    WEEK_PREFS  week;
};


struct GLOBAL_PREFS {
    double mod_time;
    bool run_on_batteries;
        // poorly named; what it really means is:
        // if false, suspend while on batteries
    bool run_if_user_active;
    double idle_time_to_run;
    double suspend_if_no_recent_input;
    bool leave_apps_in_memory;
    bool confirm_before_connecting;
    bool hangup_if_dialed;
    bool dont_verify_images;
    TIME_PREFS cpu_times;
    TIME_PREFS net_times;
    double work_buf_min_days;
    double work_buf_additional_days;
    int max_cpus;
    double max_ncpus_pct;
    double cpu_scheduling_period_minutes;
    double disk_interval;
    double disk_max_used_gb;
    double disk_max_used_pct;
    double disk_min_free_gb;
    double vm_max_used_frac;
    double ram_max_used_busy_frac;
    double ram_max_used_idle_frac;
    double max_bytes_sec_up;
    double max_bytes_sec_down;
    double cpu_usage_limit;
    char source_project[256];
    char source_scheduler[256];
    bool host_specific;

    GLOBAL_PREFS();
    void defaults();
    void init();
    void clear_bools();
    int parse(XML_PARSER&, const char* venue, bool& found_venue, GLOBAL_PREFS_MASK& mask);
    int parse_day(XML_PARSER&);
    int parse_override(XML_PARSER&, const char* venue, bool& found_venue, GLOBAL_PREFS_MASK& mask);
    int parse_file(const char* filename, const char* venue, bool& found_venue);
    int write(MIOFILE&);
    int write_subset(MIOFILE&, GLOBAL_PREFS_MASK&);
    inline double cpu_scheduling_period() {
        return cpu_scheduling_period_minutes*60;
    }
    /// Gets the maximum number of CPUs that may be used.
    int GetMaxCPUs(int availableCPUs) const;
};

#endif
