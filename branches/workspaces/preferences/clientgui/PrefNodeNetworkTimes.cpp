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
#include "PrefNodeNetworkTimes.h"


IMPLEMENT_DYNAMIC_CLASS(PrefNodeNetworkTimes, PrefNodeBase)

BEGIN_EVENT_TABLE(PrefNodeNetworkTimes, PrefNodeBase)
    EVT_BUTTON(ID_PREF_COPY_TIMES, PrefNodeNetworkTimes::OnCopyTimes)
END_EVENT_TABLE()

PrefNodeNetworkTimes::PrefNodeNetworkTimes(wxWindow* parent, GLOBAL_PREFS* preferences)
: PrefNodeBase(parent, preferences) {

    PrefGroup* copy = AddGroup(_("Copy Settings"));

    copy->AddPreference(new PrefValueButton(this,
        _("Copy the network time settings from the processor time settings."),
        _("Copy"),
        _("If you want to use the same settings for networking and processing, copy them here."),
        ID_PREF_COPY_TIMES
        ));

    PrefGroup* restrict = AddGroup(_("Time Restrictions"));
    m_time = new PrefValueTime(this,
        _("Allow network activity during these times:"),
        _("Synecdoche will only perform normal network activity at the specified times. "
        "Some user actions override this setting. If you set the start time after "
        "the end time, then Synecdoche will perform network activity during the "
        "night. Times must be specified in HH:MM format. Default: no restrictions."),
        &preferences->net_times);

    restrict->AddPreference(m_time);
        
    PrefGroup* sched = AddGroup(_("Weekly Schedule"));

    m_week = new PrefValueWeek(this,
        _("Override specific days with these times:"),
        _("Permitted values are 'Always', 'Never', or a time period in the "
        "form HH:MM - HH:MM. Default: no restrictions."),
        &preferences->net_times);

    sched->AddPreference(m_week);

    m_time->Connect(
        PREF_EVT_CMD_UPDATE,
        wxCommandEventHandler(PrefNodeBase::PrefValueWeek::OnUpdateUI),
        NULL,
        m_week
        );
}

// Handler for copy button.
void PrefNodeNetworkTimes::OnCopyTimes(wxCommandEvent& WXUNUSED(event)) {

    m_preferences->net_times = m_preferences->cpu_times;

    m_time->Update();
    m_week->Update();
}
