// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
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
        _("Allow Synecdoche to do work during these times:"),
        _("Synecdoche will only do work at the specified times. If you set "
        "the start time after the end time, then Synecdoche will work during the "
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
