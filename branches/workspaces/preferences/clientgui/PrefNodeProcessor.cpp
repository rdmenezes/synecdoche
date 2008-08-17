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
#include "ValidateBool.h"
#include "ValidateNumber.h"
#include "PrefGridBase.h"
#include "PrefNodeProcessor.h"


IMPLEMENT_DYNAMIC_CLASS(PrefNodeProcessor, PrefGridBase)


PrefNodeProcessor::PrefNodeProcessor(wxWindow* parent, GLOBAL_PREFS* preferences)
: PrefGridBase(parent, preferences) {

    PrefGroup* limits = AddGroup(_("CPU Limits"));

    limits->AddPreference(new PrefValueText(this,
        _("Maximum percentage of processor time to use"),
        _("The built-in throttle can limit the CPU time used. "
        "This is commonly used to reduce the risk of overheating. The throttle "
        "is coarse-grained; you may see a sawtooth profile if you graph "
        "CPU usage."),
        _("100%"),
        ValidateNumber<double>(&m_preferences->cpu_usage_limit, 0, 100))
    );

    limits->AddPreference(new PrefValueText(this,
        _("Percentage of processors to use"),
        _("This limit specifies the number of processors or individual "
        "processor cores as a percentage of the total. Many projects will run one "
        "process on each permitted core."),
        _("100%"),
        ValidateNumber<double>(&m_preferences->max_ncpus_pct, 0, 100))
    );

    PrefGroup* restrict = AddGroup(_("Processing Restrictions"));

    // Suspend while in use

    // WARNING! Prompt is opposite sense.
    PrefValueBase* run_if_user_active = new PrefValueBool(this,
        _("Suspend while computer is in use"),
        _("Use this option if you want Synecdoche to behave like a screensaver, "
        "only working when you are away from your computer."),
        _("No"),
        ValidateYesNo(&m_preferences->run_if_user_active, true));

    m_idleTimeResume = new PrefValueText(this,
        _("Interval before resuming from idle (minutes)"),
        _("This option prevents computation from starting if you are only "
        "away from your computer briefly."),
        _("3 minutes"),
        ValidateNumber<double>(&m_preferences->idle_time_to_run));

    run_if_user_active->Connect(
        wxEVT_COMMAND_COMBOBOX_SELECTED,
        wxCommandEventHandler(PrefNodeProcessor::OnRunIdleChanged),
        NULL,
        this
        );

    m_idleTimeResume->Enable(! m_preferences->run_if_user_active);

    restrict->AddPreference(run_if_user_active);
    restrict->AddPreference(m_idleTimeResume);

    // Suspend while not in use (power saving option)

    m_suspendIdle = (m_preferences->suspend_if_no_recent_input != 0);

    PrefValueBase* suspend_if_no_recent_input = new PrefValueBool(this,
        _("Use power saving features"),
        _("Use this option if you don't want processing to conflict "
        "with power saving features."),
        _("No"),
        ValidateYesNo(&m_suspendIdle));

    m_idleTimeSuspend = new PrefValueText(this,
        _("Idle interval before suspending (minutes)"),
        _("This is the length of time your computer will continue "
        "processing after it falls idle."),
        _("20 minutes"),
        ValidateNumber<double>(&m_preferences->suspend_if_no_recent_input));

    suspend_if_no_recent_input->Connect(
        wxEVT_COMMAND_COMBOBOX_SELECTED,
        wxCommandEventHandler(PrefNodeProcessor::OnSuspendIdleChanged),
        NULL,
        this
        );

    m_idleTimeSuspend->Enable(m_suspendIdle);

    restrict->AddPreference(suspend_if_no_recent_input);
    restrict->AddPreference(m_idleTimeSuspend);

    // Run on batteries

    // WARNING! Prompt is opposite sense.
    restrict->AddPreference(new PrefValueBool(this,
        _("Suspend while computer is running on batteries"),
        _("Generally, laptop users don't want tasks to run while "
        "on battery power."),
        _("Yes"),
        ValidateYesNo(&m_preferences->run_on_batteries, true))
    );
}


void PrefNodeProcessor::OnRunIdleChanged(wxCommandEvent& event) {

    wxComboCtrl* combo = (wxComboCtrl*) event.GetEventObject();
    m_idleTimeResume->Enable(combo->GetValue().IsSameAs(_("Yes")));
}


void PrefNodeProcessor::OnSuspendIdleChanged(wxCommandEvent& event) {

    wxComboCtrl* combo = (wxComboCtrl*) event.GetEventObject();
    bool enable = combo->GetValue().IsSameAs(_("Yes"));

    if (enable) {
        m_idleTimeSuspend->SetValue(_("20"));
    } else {
        m_idleTimeSuspend->SetValue(_("0"));
    }
    
    m_idleTimeSuspend->Enable(enable);
}
