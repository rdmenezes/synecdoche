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
#include "PrefNodeGeneral.h"


IMPLEMENT_DYNAMIC_CLASS(PrefNodeGeneral, PrefGridBase)

PrefNodeGeneral::PrefNodeGeneral(wxWindow* parent, GLOBAL_PREFS* preferences)
: PrefGridBase(parent, preferences) {

    PrefGroup* work = AddGroup(_("Work Buffer"));

    work->AddPreference(new PrefValueText(this,
        _("Minimum size of work buffer (days)"),
        _("Try to buffer at least this much work at all times. "
        "This is in addition to the work buffered based on your network "
        "settings. The maximum buffer is 10 days, but some projects may "
        "have a lower limit."),
        _("0.25 days"),
        ValidateNumber<double>(&m_preferences->work_buf_additional_days, 0.0, 10.0))
    );

    PrefGroup* switching = AddGroup(_("Application Switching"));

    switching->AddPreference(new PrefValueText(this,
        _("Interval between switching applications (minutes)"),
        _("If you are attached to more than one project, this value is "
        "used to balance the load between your projects. Tasks will not "
        "switch exactly at this interval, it is only used as a guide. "
        "If you are attached to a single project, this value is ignored."),
        _("60 minutes"),
        ValidateNumber<double>(&m_preferences->cpu_scheduling_period_minutes))
    );
}