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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <time.h>
#endif

#include <stdexcept>

#include "prefs.h"

#include "parse.h"
#include "util.h"
#include "miofile.h"

#include "error_numbers.h"

GLOBAL_PREFS_MASK::GLOBAL_PREFS_MASK() {
    clear();
}

void GLOBAL_PREFS_MASK::clear() {
    memset(this, 0, sizeof(GLOBAL_PREFS_MASK));
}

void GLOBAL_PREFS_MASK::set_all() {
    run_on_batteries = true;
    run_if_user_active = true;
    idle_time_to_run = true;
    suspend_if_no_recent_input = true;
    start_hour = true;
    end_hour = true;
    net_start_hour = true;
    net_end_hour = true;
    leave_apps_in_memory = true;
    confirm_before_connecting = true;
    hangup_if_dialed = true;
    dont_verify_images = true;
    work_buf_min_days = true;
    work_buf_additional_days = true;
    max_cpus = true;
    max_ncpus_pct = true;
    cpu_scheduling_period_minutes = true;
    disk_interval = true;
    disk_max_used_gb = true;
    disk_max_used_pct = true;
    disk_min_free_gb = true;
    vm_max_used_frac = true;
    ram_max_used_busy_frac = true;
    ram_max_used_idle_frac = true;
    idle_time_to_run = true;
    max_bytes_sec_up = true;
    max_bytes_sec_down = true;
    cpu_usage_limit = true;
}

bool GLOBAL_PREFS_MASK::are_prefs_set() {
    if (run_on_batteries) return true;
    if (run_if_user_active) return true;
    if (idle_time_to_run) return true;
    if (suspend_if_no_recent_input) return true;
    if (start_hour) return true;
    if (end_hour) return true;
    if (net_start_hour) return true;
    if (net_end_hour) return true;
    if (leave_apps_in_memory) return true;
    if (confirm_before_connecting) return true;
    if (hangup_if_dialed) return true;
    if (dont_verify_images) return true;
    if (work_buf_min_days) return true;
    if (work_buf_additional_days) return true;
    if (max_cpus) return true;
    if (max_ncpus_pct) return true;
    if (cpu_scheduling_period_minutes) return true;
    if (disk_interval) return true;
    if (disk_max_used_gb) return true;
    if (disk_max_used_pct) return true;
    if (disk_min_free_gb) return true;
    if (vm_max_used_frac) return true;
    if (ram_max_used_busy_frac) return true;
    if (ram_max_used_idle_frac) return true;
    if (idle_time_to_run) return true;
    if (max_bytes_sec_up) return true;
    if (max_bytes_sec_down) return true;
    if (cpu_usage_limit) return true;
    return false;
}

bool GLOBAL_PREFS_MASK::are_simple_prefs_set() {
    if (start_hour) return true;
    if (end_hour) return true;
    if (net_start_hour) return true;
    if (net_end_hour) return true;
    if (disk_max_used_gb) return true;
    if (cpu_usage_limit) return true;
    if (run_on_batteries) return true;
    if (run_if_user_active) return true;
    if (idle_time_to_run) return true;
    return false;
}


// TIME_SPAN implementation

/// Construct a TIME_SPAN instance.
TIME_SPAN::TIME_SPAN() : m_start(0), m_end(0) {
}

/// Construct a TIME_SPAN instance.
///
/// \param[in] start The beginning of the time span in seconds from midnight.
/// \param[in] end The end of the time span in seconds from midnight.
TIME_SPAN::TIME_SPAN(time_t start, time_t end) : m_start(start), m_end(end) {
    check_range(start);
    check_range(end);
}

TIME_SPAN& TIME_SPAN::operator=(const TIME_SPAN& r) {
    if (this != &r) {
        m_start = r.m_start;
        m_end = r.m_end;
    }
    return *this;
}

bool TIME_SPAN::operator==(const TIME_SPAN& r) {
    return ((m_start == r.m_start) && (m_end == r.m_end));
}

/// Check if the client should be suspended for a given point in time.
///
/// Run always if start==end or start==0, end=24.
/// Don't run at all if start=24, end=0.
///
/// \param[in] point_in_time The point in time which should be checked.
/// \return True if the client should suspend at the given point in time.
bool TIME_SPAN::suspended(const time_t point_in_time) const {
    if (m_start == m_end) return false;
    if (m_start == 0 && m_end == (24 * 3600)) return false;
    if (m_start == (24 * 3600) && m_end == 0) return true;
    if (m_start < m_end) {
        return (point_in_time < m_start || point_in_time > m_end);
    } else {
        return (point_in_time >= m_end && point_in_time < m_start);
    }
}

/// Get the run mode determined by this TIME_SPAN instance.
///
/// start == end or start == 0:00 and end == 24:00 => return Always.
/// start == 24:00 and end == 0:00 => return Never.
/// Returns Between in all other cases.
///
/// \return The run mode determined by this TIME_SPAN instance.
TIME_SPAN::TimeMode TIME_SPAN::mode() const {
    if ((m_end == m_start) || ((m_start == 0) && (m_end == (24 * 3600)))) {
        return Always;
    } else if ((m_start == (24 * 3600)) && (m_end == 0)) {
        return Never;
    }
    return Between;
}

/// Reset this instance to its default values (start = end = 0).
void TIME_SPAN::clear() {
    m_start = 0;
    m_end = 0;
}

time_t TIME_SPAN::get_start() const {
    return m_start;
}

time_t TIME_SPAN::get_end() const {
    return m_end;
}

void TIME_SPAN::set_start(const time_t start) {
    check_range(start);
    m_start = start;
}

void TIME_SPAN::set_end(const time_t end) {
    check_range(end);
    m_end = end;
}

void TIME_SPAN::check_range(const time_t point_in_time) const {
    if ((point_in_time < 0) || (point_in_time > 24 * 3600)) {
        std::ostringstream msg;
        msg << "TIME_SPAN::check_range: Value \"" << point_in_time << "\" is not allowed!";
        throw std::invalid_argument(msg.str());
    }
}

// TIME_PREFS implementation

TIME_PREFS::TIME_PREFS() : TIME_SPAN() {
}

TIME_PREFS::TIME_PREFS(time_t start, time_t end) : TIME_SPAN(start, end) {
}

/// Reset this instance to its default values.
void TIME_PREFS::clear() {
    TIME_SPAN::clear();
    week.clear();
}


/// Check if the client should be currently suspended based on this TIME_PREFS instance.
///
/// \return True if the client should be suspended, false otherwise.
bool TIME_PREFS::suspended() const {
    time_t now = time(0);
    struct tm* tmp = localtime(&now);
    time_t point_in_time = tmp->tm_hour * 3600 + tmp->tm_min * 60 + tmp->tm_sec;
    int day = tmp->tm_wday;

    // Use day-specific settings, if they exist:
    const TIME_SPAN* span = week.get(day) ? week.get(day) : this;

    return span->suspended(point_in_time);
}


// WEEK_PREFS implementation

WEEK_PREFS::WEEK_PREFS() {
    std::fill(days, days + 7, static_cast<TIME_SPAN*>(0));
}


WEEK_PREFS::WEEK_PREFS(const WEEK_PREFS& original) {
    std::fill(days, days + 7, static_cast<TIME_SPAN*>(0));
    copy(original);
}

WEEK_PREFS::~WEEK_PREFS() {
    clear();
}

WEEK_PREFS& WEEK_PREFS::operator=(const WEEK_PREFS& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

/// Create a deep copy.
void WEEK_PREFS::copy(const WEEK_PREFS& original) {
    for (int i = 0; i < 7; ++i) {
        TIME_SPAN* time = original.days[i];
        if (time) {
            if (days[i]) {
                *days[i] = *time;
            } else {
                days[i] = new TIME_SPAN(*time);
            }
        } else {
            unset(i);
        }
    }
}

/// Reset this instance to its default values.
void WEEK_PREFS::clear() {
    for (int i = 0; i < 7; ++i) {
        delete days[i];
        days[i] = 0;
    }
}

/// Get the set time span for a given day.
///
/// \return Pointer to a TIME_SPAN instance which is set for the day denoted by \a day.
///         If no time span is set for requested day this function returns 0.
const TIME_SPAN* WEEK_PREFS::get(int day) const {
    if (day < 0 || day > 6) return 0;
    return days[day];
}

/// Set a time span in which the client is allowed to run.
///
/// \param[in] day The for which the time span should be set.
/// \param[in] start The start time on the day denoted by \a day from which on processing will
///                  be allowed in seconds after midnight.
/// \param[in] end The end time on the day denoted by \a day from which on no processing will
///                be allowed in seconds after midnight.
void WEEK_PREFS::set(int day, time_t start, time_t end) {
    if (day < 0 || day > 6) return;
    delete days[day];
    days[day] = new TIME_SPAN(start, end);
}

/// Set a time span in which the client is allowed to run.
///
/// \param[in] day The for which the time span should be set.
/// \param[in] time The time span in which processing is allowed for the day denoted by \a day.
void WEEK_PREFS::set(int day, const TIME_SPAN& time) {
    if (day < 0 || day > 6) return;
    if (*days[day] == time) return;
    delete days[day];
    days[day] = new TIME_SPAN(time);
}

/// Remove the timespan for a given day.
///
/// \param[in] day The day for which the set time span should be removed.
void WEEK_PREFS::unset(int day) {
    if (day < 0 || day > 6) return;
    delete days[day];
    days[day] = 0;
}

// The following values determine how the client behaves
// if there are no global prefs (e.g. on our very first RPC).
// These should impose minimal restrictions,
// so that the client can do the RPC and get the global prefs from the server
//
void GLOBAL_PREFS::defaults() {
    run_on_batteries = true;
    run_if_user_active = true;
    idle_time_to_run = 3;
    suspend_if_no_recent_input = 0;
    cpu_times.clear();
    net_times.clear();
    leave_apps_in_memory = false;
    confirm_before_connecting = true;
    hangup_if_dialed = false;
    dont_verify_images = false;
    work_buf_min_days = 0.1;
    work_buf_additional_days = 0.25;
    max_cpus = 16;
    max_ncpus_pct = 100;
    cpu_scheduling_period_minutes = 60;
    disk_interval = 60;
    disk_max_used_gb = 10;
    disk_max_used_pct = 50;
    disk_min_free_gb = 0.1;
    vm_max_used_frac = 0.75;
    ram_max_used_busy_frac = 0.5;
    ram_max_used_idle_frac = 0.9;
    max_bytes_sec_up = 0;
    max_bytes_sec_down = 0;
    cpu_usage_limit = 100;

    // don't initialize source_project, source_scheduler,
    // mod_time, host_specific here
    // since they are outside of <venue> elements,
    // and this is called when find the right venue.
    // Also, don't memset to 0
}

// before parsing
void GLOBAL_PREFS::clear_bools() {
    run_on_batteries = false;
    run_if_user_active = false;
    leave_apps_in_memory = false;
    confirm_before_connecting = false;
    hangup_if_dialed = false;
    dont_verify_images = false;
}

void GLOBAL_PREFS::init() {
    defaults();
    strcpy(source_project, "");
    strcpy(source_scheduler, "");
    mod_time = 0;
    host_specific = false;
}

GLOBAL_PREFS::GLOBAL_PREFS() {
    init();
}

/// Parse XML global prefs, setting defaults first.
int GLOBAL_PREFS::parse(XML_PARSER& xp, const char* host_venue, bool& found_venue, GLOBAL_PREFS_MASK& mask) {
    init();
    clear_bools();
    return parse_override(xp, host_venue, found_venue, mask);
}

int GLOBAL_PREFS::parse_day(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;

    int day_of_week = -1;
    bool has_cpu = false;
    bool has_net = false;
    double start_hour = 0;
    double end_hour = 0;
    double net_start_hour = 0;
    double net_end_hour = 0;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/day_prefs")) {
            if (day_of_week < 0 || day_of_week > 6) return ERR_XML_PARSE;
            if (has_cpu) {
                cpu_times.week.set(day_of_week, static_cast<time_t>(start_hour * 3600.0), static_cast<time_t>(end_hour * 3600.0));
            }
            if (has_net) {
                net_times.week.set(day_of_week, static_cast<time_t>(net_start_hour * 3600.0), static_cast<time_t>(net_end_hour * 3600.0));
            }
            return 0;
        }
        if (xp.parse_int(tag, "day_of_week", day_of_week)) continue;
        if (xp.parse_double(tag, "start_hour", start_hour)) {
            has_cpu = true;
            continue;
        }
        if (xp.parse_double(tag, "end_hour", end_hour)) {
            has_cpu = true;
            continue;
        }
        if (xp.parse_double(tag, "net_start_hour", net_start_hour)) {
            has_net = true;
            continue;
        }
        if (xp.parse_double(tag, "net_end_hour", net_end_hour)) {
            has_net = true;
            continue;
        }
        xp.skip_unexpected(tag, true, "GLOBAL_PREFS::parse_day");
    }
    return ERR_XML_PARSE;
}


/// Parse global prefs, overriding whatever is currently in the structure.
///
/// If host_venue is nonempty and we find an element of the form
/// <venue name="X">
///   ...
/// </venue>
/// where X==host_venue, then parse that and ignore the rest.
/// Otherwise ignore <venue> elements.
///
/// The start tag may or may not have already been parsed
///
/// \return Zero on success, ERR_XML_PARSE on error.
int GLOBAL_PREFS::parse_override(XML_PARSER& xp, const char* host_venue, bool& found_venue, GLOBAL_PREFS_MASK& mask) {
    char tag[256];
    char buf2[256];
    char attrs[256];
    bool in_venue = false, in_correct_venue=false, is_tag;
    double dtemp;

    found_venue = false;
    mask.clear();
    // Reset max_ncpus_pct to zero, because absence of this preference means use max_cpus,
    // and not use the default, which is 100%.
    max_ncpus_pct = 0;

    while (!xp.get(tag, sizeof(tag), is_tag, attrs, sizeof(attrs))) {
        if (!is_tag) continue;
        if (!strcmp(tag, "global_preferences")) continue;
        if (!strcmp(tag, "/global_preferences")) {
            return 0;
        }
        if (in_venue) {
            if (!strcmp(tag, "/venue")) {
                if (in_correct_venue) {
                    return 0;
                } else {
                    in_venue = false;
                    continue;
                }
            } else {
                if (!in_correct_venue) continue;
            }
        } else {
            if (strstr(tag, "venue")) {
                in_venue = true;
                parse_attr(attrs, "name", buf2, sizeof(buf2));
                if (!strcmp(buf2, host_venue)) {
                    defaults();
                    clear_bools();
                    mask.clear();
                    max_ncpus_pct = 0;
                    in_correct_venue = true;
                    found_venue = true;
                } else {
                    in_correct_venue = false;
                }
                continue;
            }
        }
        if (xp.parse_str(tag, "source_project", source_project, sizeof(source_project))) continue;
        if (xp.parse_str(tag, "source_scheduler", source_scheduler, sizeof(source_scheduler))) {
            continue;
        }
        if (xp.parse_double(tag, "mod_time", mod_time)) {
            double now = dtime();
            if (mod_time > now) {
                mod_time = now;
            }
            continue;
        }
        if (xp.parse_bool(tag, "run_on_batteries", run_on_batteries)) {
            mask.run_on_batteries = true;
            continue;
        }
        if (xp.parse_bool(tag, "run_if_user_active", run_if_user_active)) {
            mask.run_if_user_active = true;
            continue;
        }
        if (xp.parse_double(tag, "idle_time_to_run", idle_time_to_run)) {
            mask.idle_time_to_run = true;
            continue;
        }
        if (xp.parse_double(tag, "suspend_if_no_recent_input", suspend_if_no_recent_input)) {
            mask.suspend_if_no_recent_input = true;
            continue;
        }
        double dtmp = 0.0;
        if (xp.parse_double(tag, "start_hour", dtmp)) {
            cpu_times.set_start(static_cast<time_t>(dtmp * 3600.0));
            mask.start_hour = true;
            continue;
        }
        if (xp.parse_double(tag, "end_hour", dtmp)) {
            cpu_times.set_end(static_cast<time_t>(dtmp * 3600.0));
            mask.end_hour = true;
            continue;
        }
        if (xp.parse_double(tag, "net_start_hour", dtmp)) {
            net_times.set_start(static_cast<time_t>(dtmp * 3600.0));
            mask.net_start_hour = true;
            continue;
        }
        if (xp.parse_double(tag, "net_end_hour", dtmp)) {
            net_times.set_end(static_cast<time_t>(dtmp * 3600.0));
            mask.net_end_hour = true;
            continue;
        }
        if (!strcmp(tag, "day_prefs")) {
            parse_day(xp);
            continue;
        }
        if (xp.parse_bool(tag, "leave_apps_in_memory", leave_apps_in_memory)) {
            mask.leave_apps_in_memory = true;
            continue;
        }
        if (xp.parse_bool(tag, "confirm_before_connecting", confirm_before_connecting)) {
            mask.confirm_before_connecting = true;
            continue;
        }
        if (xp.parse_bool(tag, "hangup_if_dialed", hangup_if_dialed)) {
            mask.hangup_if_dialed = true;
            continue;
        }
        if (xp.parse_bool(tag, "dont_verify_images", dont_verify_images)) {
            mask.dont_verify_images = true;
            continue;
        }
        if (xp.parse_double(tag, "work_buf_min_days", work_buf_min_days)) {
            if (work_buf_min_days < 0.00001) work_buf_min_days = 0.00001;
            mask.work_buf_min_days = true;
            continue;
        }
        if (xp.parse_double(tag, "work_buf_additional_days", work_buf_additional_days)) {
            if (work_buf_additional_days < 0) work_buf_additional_days = 0;
            mask.work_buf_additional_days = true;
            continue;
        }
        if (xp.parse_int(tag, "max_cpus", max_cpus)) {
            if (max_cpus < 1) max_cpus = 1;
            mask.max_cpus = true;
            continue;
        }
        if (xp.parse_double(tag, "max_ncpus_pct", max_ncpus_pct)) {
            // If this is zero, max_cpus will take precedence.
            if (max_ncpus_pct < 0) max_ncpus_pct = 0;
            if (max_ncpus_pct > 100) max_ncpus_pct = 100;
            mask.max_ncpus_pct = true;
            continue;
        }
        if (xp.parse_double(tag, "disk_interval", disk_interval)) {
            if (disk_interval<0) disk_interval = 0;
            mask.disk_interval = true;
            continue;
        }
        if (xp.parse_double(tag, "cpu_scheduling_period_minutes", cpu_scheduling_period_minutes)) {
            if (cpu_scheduling_period_minutes < 0.0001) cpu_scheduling_period_minutes = 60;
            mask.cpu_scheduling_period_minutes = true;
            continue;
        }
        if (xp.parse_double(tag, "disk_max_used_gb", disk_max_used_gb)) {
            mask.disk_max_used_gb = true;
            continue;
        }
        if (xp.parse_double(tag, "disk_max_used_pct", disk_max_used_pct)) {
            mask.disk_max_used_pct = true;
            continue;
        }
        if (xp.parse_double(tag, "disk_min_free_gb", disk_min_free_gb)) {
            mask.disk_min_free_gb = true;
            continue;
        }
        if (xp.parse_double(tag, "vm_max_used_pct", dtemp)) {
            vm_max_used_frac = dtemp/100;
            mask.vm_max_used_frac = true;
            continue;
        }
        if (xp.parse_double(tag, "ram_max_used_busy_pct", dtemp)) {
            if (!dtemp) dtemp = 100;
            ram_max_used_busy_frac = dtemp/100;
            mask.ram_max_used_busy_frac = true;
            continue;
        }
        if (xp.parse_double(tag, "ram_max_used_idle_pct", dtemp)) {
            if (!dtemp) dtemp = 100;
            ram_max_used_idle_frac = dtemp/100;
            mask.ram_max_used_idle_frac = true;
            continue;
        }
        if (xp.parse_double(tag, "max_bytes_sec_up", max_bytes_sec_up)) {
            if (max_bytes_sec_up < 0) max_bytes_sec_up = 0;
            mask.max_bytes_sec_up = true;
            continue;
        }
        if (xp.parse_double(tag, "max_bytes_sec_down", max_bytes_sec_down)) {
            if (max_bytes_sec_down < 0) max_bytes_sec_down = 0;
            mask.max_bytes_sec_down = true;
            continue;
        }
        if (xp.parse_double(tag, "cpu_usage_limit", dtemp)) {
            if (dtemp > 0 && dtemp <= 100) {
                cpu_usage_limit = dtemp;
                mask.cpu_usage_limit = true;
            }
            continue;
        }
        if (xp.parse_bool(tag, "host_specific", host_specific)) {
            continue;
        }
        // false means don't print anything
        xp.skip_unexpected(tag, false, "GLOBAL_PREFS::parse_override");
    }
    return ERR_XML_PARSE;
}

/// Parse global prefs file.
int GLOBAL_PREFS::parse_file(
    const char* filename, const char* host_venue, bool& found_venue
) {
    FILE* f;
    GLOBAL_PREFS_MASK mask;
    int retval;

    f = fopen(filename, "r");
    if (!f) return ERR_FOPEN;
    MIOFILE mf;
    mf.init_file(f);
    XML_PARSER xp(&mf);
    retval = parse(xp, host_venue, found_venue, mask);
    fclose(f);
    return retval;
}

/// Write the global prefs that are actually in force
/// (our particular venue, modified by overwrite file).
/// This is used to write
/// 1) the app init data file
/// 2) GUI RPC get_state reply
/// Not used for scheduler request; there, we just copy the
/// global_prefs.xml file (which includes all venues).
///
/// \param[in] f Reference to a file object that will receive the xml-data.
/// \return Always returns zero.
int GLOBAL_PREFS::write(MIOFILE& f) const {
    f.printf(
        "<global_preferences>\n"
        "   <source_project>%s</source_project>\n"
        "   <mod_time>%f</mod_time>\n"
        "%s%s"
        "   <suspend_if_no_recent_input>%f</suspend_if_no_recent_input>\n"
        "   <start_hour>%f</start_hour>\n"
        "   <end_hour>%f</end_hour>\n"
        "   <net_start_hour>%f</net_start_hour>\n"
        "   <net_end_hour>%f</net_end_hour>\n"
        "%s%s%s%s"
        "   <work_buf_min_days>%f</work_buf_min_days>\n"
        "   <work_buf_additional_days>%f</work_buf_additional_days>\n"
        "   <max_cpus>%d</max_cpus>\n"
        "   <max_ncpus_pct>%f</max_ncpus_pct>\n"
        "   <cpu_scheduling_period_minutes>%f</cpu_scheduling_period_minutes>\n"
        "   <disk_interval>%f</disk_interval>\n"
        "   <disk_max_used_gb>%f</disk_max_used_gb>\n"
        "   <disk_max_used_pct>%f</disk_max_used_pct>\n"
        "   <disk_min_free_gb>%f</disk_min_free_gb>\n"
        "   <vm_max_used_pct>%f</vm_max_used_pct>\n"
        "   <ram_max_used_busy_pct>%f</ram_max_used_busy_pct>\n"
        "   <ram_max_used_idle_pct>%f</ram_max_used_idle_pct>\n"
        "   <idle_time_to_run>%f</idle_time_to_run>\n"
        "   <max_bytes_sec_up>%f</max_bytes_sec_up>\n"
        "   <max_bytes_sec_down>%f</max_bytes_sec_down>\n"
        "   <cpu_usage_limit>%f</cpu_usage_limit>\n",
        source_project,
        mod_time,
        run_on_batteries?"   <run_on_batteries/>\n":"",
        run_if_user_active?"   <run_if_user_active/>\n":"",
        suspend_if_no_recent_input,
        cpu_times.get_start() / 3600.0,
        cpu_times.get_end() / 3600.0,
        net_times.get_start() / 3600.0,
        net_times.get_end() / 3600.0,
        leave_apps_in_memory?"   <leave_apps_in_memory/>\n":"",
        confirm_before_connecting?"   <confirm_before_connecting/>\n":"",
        hangup_if_dialed?"   <hangup_if_dialed/>\n":"",
        dont_verify_images?"   <dont_verify_images/>\n":"",
        work_buf_min_days,
        work_buf_additional_days,
        max_cpus,
        max_ncpus_pct,
        cpu_scheduling_period_minutes,
        disk_interval,
        disk_max_used_gb,
        disk_max_used_pct,
        disk_min_free_gb,
        vm_max_used_frac*100,
        ram_max_used_busy_frac*100,
        ram_max_used_idle_frac*100,
        idle_time_to_run,
        max_bytes_sec_up,
        max_bytes_sec_down,
        cpu_usage_limit
    );

    for (int i = 0; i < 7; i++) {
        const TIME_SPAN* cpu = cpu_times.week.get(i);
        const TIME_SPAN* net = net_times.week.get(i);

        //write only when needed
        if (net || cpu) {    
            f.printf("   <day_prefs>\n");                
            f.printf("      <day_of_week>%d</day_of_week>\n", i);
            if (cpu) {
                f.printf("      <start_hour>%.02f</start_hour>\n", cpu->get_start() / 3600.0);
                f.printf("      <end_hour>%.02f</end_hour>\n", cpu->get_end() / 3600.0);
            }
            if (net) {
                f.printf("      <net_start_hour>%.02f</net_start_hour>\n", net->get_start() / 3600.0);
                f.printf("      <net_end_hour>%.02f</net_end_hour>\n", net->get_end() / 3600.0);
            }
            f.printf("   </day_prefs>\n");
        }
    }
    f.printf("</global_preferences>\n");

    return 0;
}

/// Write a subset of the global preferences,
/// as selected by the mask of bools.
int GLOBAL_PREFS::write_subset(MIOFILE& f, GLOBAL_PREFS_MASK& mask) {
    if (!mask.are_prefs_set()) return 0;
    
    f.printf("<global_preferences>\n");
    if (mask.run_on_batteries) {
        f.printf("   <run_on_batteries>%d</run_on_batteries>\n",
            run_on_batteries?1:0
        );
    }
    if (mask.run_if_user_active) {
        f.printf("   <run_if_user_active>%d</run_if_user_active>\n",
            run_if_user_active?1:0
        );
    }
    if (mask.idle_time_to_run) {
        f.printf("   <idle_time_to_run>%f</idle_time_to_run>\n", idle_time_to_run);
    }
    if (mask.suspend_if_no_recent_input) {

        f.printf("   <suspend_if_no_recent_input>%f</suspend_if_no_recent_input>\n",
            suspend_if_no_recent_input
        );
    }
    if (mask.start_hour) {
        f.printf("   <start_hour>%f</start_hour>\n", cpu_times.get_start() / 3600.0);
    }
    if (mask.end_hour) {
        f.printf("   <end_hour>%f</end_hour>\n", cpu_times.get_end() / 3600.0);
    }
    if (mask.net_start_hour) {
        f.printf("   <net_start_hour>%f</net_start_hour>\n", net_times.get_start() / 3600.0);
    }
    if (mask.net_end_hour) {
        f.printf("   <net_end_hour>%f</net_end_hour>\n", net_times.get_end() / 3600.0);
    }
    if (mask.leave_apps_in_memory) {
        f.printf("   <leave_apps_in_memory>%d</leave_apps_in_memory>\n",
            leave_apps_in_memory?1:0
        );
    }
    if (mask.confirm_before_connecting) {
        f.printf("   <confirm_before_connecting>%d</confirm_before_connecting>\n",
            confirm_before_connecting?1:0
        );
    }
    if (mask.hangup_if_dialed) {
        f.printf("   <hangup_if_dialed>%d</hangup_if_dialed>\n",
            hangup_if_dialed?1:0
        );
    }
    if (mask.dont_verify_images) {
        f.printf("   <dont_verify_images>%d</dont_verify_images>\n",
            dont_verify_images?1:0
        );
    }
    if (mask.work_buf_min_days) {
        f.printf("   <work_buf_min_days>%f</work_buf_min_days>\n", work_buf_min_days);
    }
    if (mask.work_buf_additional_days) {
        f.printf("   <work_buf_additional_days>%f</work_buf_additional_days>\n", work_buf_additional_days);
    }
    if (mask.max_cpus) {
        f.printf("   <max_cpus>%d</max_cpus>\n", max_cpus);
    }
    if (mask.max_ncpus_pct) {
        f.printf("   <max_ncpus_pct>%f</max_ncpus_pct>\n", max_ncpus_pct);
    }
    if (mask.cpu_scheduling_period_minutes) {
        f.printf("   <cpu_scheduling_period_minutes>%f</cpu_scheduling_period_minutes>\n", cpu_scheduling_period_minutes);
    }
    if (mask.disk_interval) {
        f.printf("   <disk_interval>%f</disk_interval>\n", disk_interval);
    }
    if (mask.disk_max_used_gb) {
        f.printf("   <disk_max_used_gb>%f</disk_max_used_gb>\n", disk_max_used_gb);
    }
    if (mask.disk_max_used_pct) {
        f.printf("   <disk_max_used_pct>%f</disk_max_used_pct>\n", disk_max_used_pct);
    }
    if (mask.disk_min_free_gb) {
        f.printf("   <disk_min_free_gb>%f</disk_min_free_gb>\n", disk_min_free_gb);
    }
    if (mask.vm_max_used_frac) {
        f.printf("   <vm_max_used_pct>%f</vm_max_used_pct>\n", vm_max_used_frac*100);
    }
    if (mask.ram_max_used_busy_frac) {
        f.printf("   <ram_max_used_busy_pct>%f</ram_max_used_busy_pct>\n", ram_max_used_busy_frac*100);
    }
    if (mask.ram_max_used_idle_frac) {
        f.printf("   <ram_max_used_idle_pct>%f</ram_max_used_idle_pct>\n", ram_max_used_idle_frac*100);
    }
    if (mask.max_bytes_sec_up) {
        f.printf("   <max_bytes_sec_up>%f</max_bytes_sec_up>\n", max_bytes_sec_up);
    }
    if (mask.max_bytes_sec_down) {
        f.printf("   <max_bytes_sec_down>%f</max_bytes_sec_down>\n", max_bytes_sec_down);
    }
    if (mask.cpu_usage_limit) {
        f.printf("   <cpu_usage_limit>%f</cpu_usage_limit>\n", cpu_usage_limit);
    }

    for (int i=0; i<7; i++) {
        const TIME_SPAN* cpu = cpu_times.week.get(i);
        const TIME_SPAN* net = net_times.week.get(i);
        //write only when needed
        if (net || cpu) {
            f.printf("   <day_prefs>\n");                
            f.printf("      <day_of_week>%d</day_of_week>\n", i);
            if (cpu) {
                f.printf("      <start_hour>%.02f</start_hour>\n", cpu->get_start() / 3600.0);
                f.printf("      <end_hour>%.02f</end_hour>\n", cpu->get_end() / 3600.0);
            }
            if (net) {
                f.printf("      <net_start_hour>%.02f</net_start_hour>\n", net->get_start() / 3600.0);
                f.printf("      <net_end_hour>%.02f</net_end_hour>\n", net->get_end() / 3600.0);
            }
            f.printf("   </day_prefs>\n");
        }
    }
    f.printf("</global_preferences>\n");
    return 0;
}


/// The new percentage preference for CPUs is given precedence, but if that 
/// isn't specified then the exact limit is used. The maximum is never less
/// than 1.
/// \param[in] availableCPUs Total number of CPUs available (need not represent real CPUs).
/// \return The maximum number of CPUs that may be used.
int GLOBAL_PREFS::GetMaxCPUs(int availableCPUs) const {

    if (max_ncpus_pct > 0) {
        availableCPUs = static_cast<int>((availableCPUs * max_ncpus_pct) / 100);
    } else if (max_cpus < availableCPUs) {
        availableCPUs = max_cpus;
    }
    if (availableCPUs < 1) {
        availableCPUs = 1;
    }
    return availableCPUs;
}
