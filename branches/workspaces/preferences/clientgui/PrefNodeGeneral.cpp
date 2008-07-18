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
#include "PrefNodeGeneral.h"


IMPLEMENT_DYNAMIC_CLASS(PrefNodeGeneral, PrefNodeBase)

PrefNodeGeneral::PrefNodeGeneral(wxWindow* parent, GLOBAL_PREFS* preferences)
: PrefNodeBase(parent, preferences) {

    PrefGroup* work = AddGroup(_("Work Buffer"));

    work->AddPreference(new PrefValueText(this,
        _("work_buf_additional_days"),
        _("Buffer at least"),
        _("days (Max 10)"),
        _("BOINC will try to buffer at least this much work at all times. "
        "This is in addition to the work buffered based on your network "
        "settings. The maximum buffer is 10 days, but some projects may "
        "have a lower limit. Default 0.25 days."),
        CValidateNumber<double>(&m_preferences->work_buf_additional_days, 0.0, 10.0))
    );

    PrefGroup* switching = AddGroup(_("Application Switching"));

    switching->AddPreference(new PrefValueText(this,
        _("cpu_scheduling_period_minutes"),
        _("Switch between applications every"),
        _("minutes"),
        _("If you are attached to more than one project, BOINC uses this "
        "value to balance the load between your projects. BOINC will not "
        "switch exactly at this interval, it is only used as a guide. "
        "Default 60 minutes."),
        CValidateNumber<double>(&m_preferences->cpu_scheduling_period_minutes))
    );
}
