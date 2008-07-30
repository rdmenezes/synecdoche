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
#include "PrefGridBase.h"
#include "PrefNodeMemory.h"


IMPLEMENT_DYNAMIC_CLASS(PrefNodeMemory, PrefGridBase)

PrefNodeMemory::PrefNodeMemory(wxWindow* parent, GLOBAL_PREFS* preferences)
: PrefGridBase(parent, preferences) {

    PrefGroup* work = AddGroup(_("Memory Limits"));

    work->AddPreference(new PrefValueText(this,
        _("Maximum memory to use when computer is in use"),
        _("Limits the amount of RAM used by project applications when your computer "
        "is in use (i.e. when there has been recent mouse or keyboard activity). "
        "Setting this to a low value lets you process all the time without "
        "impacting your computer's performance."),
        _("50%"),
        CValidatePercent<double>(&m_preferences->ram_max_used_busy_frac))
    );

    work->AddPreference(new PrefValueText(this,
        _("Maximum memory to use when computer is idle"),
        _("Limits the amount of RAM used by project applications when your computer "
        "is not in use."),
        _("90%"),
        CValidatePercent<double>(&m_preferences->ram_max_used_idle_frac))
    );

    PrefGroup* switching = AddGroup(_("Application Switching"));

    switching->AddPreference(new PrefValueBool(this,
        _("Leave applications in memory while suspended"),
        _("This improves CPU effectiveness at the cost of virtual memory. This "
        "option is not relevant if you are only attached to one project."),
        _("No"),
        ValidateYesNo(&m_preferences->leave_apps_in_memory))
    );

}
