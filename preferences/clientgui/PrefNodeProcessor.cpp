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
#include "PrefNodeBase.h"
#include "PrefNodeProcessor.h"


IMPLEMENT_DYNAMIC_CLASS(PrefNodeProcessor, PrefNodeBase)


PrefNodeProcessor::PrefNodeProcessor(wxWindow* parent, GLOBAL_PREFS* preferences)
: PrefNodeBase(parent, preferences) {

    PrefGroup* limits = AddGroup(_("CPU Limits"));

    limits->AddPreference(new PrefValueText(this,
        _("cpu_usage_limit"),
        _("Use no more than"),
        _("% of processor time"),
        _("BOINC has a built-in throttle that can limit the CPU time used. "
        "This is commonly used to reduce the risk of overheating. The throttle "
        "is coarse-grained; you may see a sawtooth profile if you graph "
        "CPU usage. Default 100%."),
        CValidateNumber<double>(&m_preferences->cpu_usage_limit, 0, 100))
    );

    limits->AddPreference(new PrefValueText(this,
        _("max_cpus"),
        _("On multiprocessor systems, at most use"),
        _("processors"),
        _("This limit specifies the number of processors or individual "
        "processor cores that BOINC will use. Many projects will run one "
        "process on each permitted core. Default 16."),
        CValidateNumber<double>(&m_preferences->max_ncpus_pct, 1, 0xFFFF, false))
    );

    PrefGroup* restrict = AddGroup(_("Processing Restrictions"));

    // WARNING! Prompt is opposite sense.
    PrefValueBase* run_if_user_active = new PrefValueBool(this,
        _("run_if_user_active"),
        _("Suspend while computer is in use"),
        _("Use this option if you want BOINC to behave like a screensaver, "
        "only working when you are away from your computer. Default false."),
        CValidateBoolInverse(&m_preferences->run_if_user_active));

    m_idleTime = new PrefValueText(this,
        _("idle_time_to_run"),
        _("Resume if computer is idle for"),
        _("minutes"),
        _("This option prevents BOINC from starting if you are only "
        "away from your computer briefly. Default 3 minutes."),
        CValidateNumber<double>(&m_preferences->idle_time_to_run));



    run_if_user_active->Connect(
        wxEVT_COMMAND_CHECKBOX_CLICKED,
        wxCommandEventHandler(PrefNodeProcessor::OnRunIdleChanged),
        NULL,
        this
        );

    m_idleTime->Enable(! m_preferences->run_if_user_active);

    restrict->AddPreference(run_if_user_active);
    restrict->AddPreference(m_idleTime);

    // WARNING! Prompt is opposite sense.
    restrict->AddPreference(new PrefValueBool(this,
        _("run_on_batteries"),
        _("Suspend while computer is running on batteries"),
        _("Generally, laptop users don't want BOINC to run while "
        "on battery power. Default true."),
        CValidateBoolInverse(&m_preferences->run_on_batteries))
    );
}


void PrefNodeProcessor::OnRunIdleChanged(wxCommandEvent& event) {

    m_idleTime->Enable(event.IsChecked());
}
