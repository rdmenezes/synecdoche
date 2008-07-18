// Synecdoche
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "stdwx.h"
#include "prefs.h"
#include "ValidateNumber.h"
#include "PrefNodeBase.h"
#include "PrefNodeProcessorTimes.h"


IMPLEMENT_DYNAMIC_CLASS(PrefNodeProcessorTimes, PrefNodeBase)

PrefNodeProcessorTimes::PrefNodeProcessorTimes(wxWindow* parent, GLOBAL_PREFS* preferences)
: PrefNodeBase(parent, preferences) {

    PrefGroup* restrict = AddGroup(_("Time Restrictions"));
    PrefValueTime* time = new PrefValueTime(this,
        _("Allow BOINC to do work during these times:"),
        _("BOINC will only do work at the specified times. If you set "
        "the start time after the end time, then BOINC will work during the "
        "night. Times must be specified in HH:MM format. Default: no restrictions."),
        &preferences->cpu_times);

    restrict->AddPreference(time);
        
    PrefGroup* sched = AddGroup(_("Weekly Schedule"));

    PrefValueWeek* week = new PrefValueWeek(this,
        _("Override specific days with these times:"),
        _("Permitted values are 'Always', 'Never', or a time period in the "
        "form HH:MM - HH:MM. Default: no restrictions."),
        &preferences->cpu_times);

    sched->AddPreference(week);

    time->Connect(
        PREF_EVT_CMD_UPDATE,
        wxCommandEventHandler(PrefNodeBase::PrefValueWeek::OnUpdateUI),
        NULL,
        week
        );


}
