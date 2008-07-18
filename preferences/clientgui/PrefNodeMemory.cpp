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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "PrefNodeMemory.h"
#endif

#include "stdwx.h"
#include "prefs.h"
#include "PrefNodeBase.h"
#include "PrefNodeMemory.h"


IMPLEMENT_DYNAMIC_CLASS(PrefNodeMemory, PrefNodeBase)

PrefNodeMemory::PrefNodeMemory(wxWindow* parent, GLOBAL_PREFS* preferences)
: PrefNodeBase(parent, preferences) {

    PrefGroup* work = AddGroup(_("Memory Limits"));

    work->AddPreference(new PrefValueText(this,
        _("ram_max_used_busy_frac"),
        _("When computer is in use, use no more than"),
        _("% total memory"),
        _("Limits the amount of RAM used by BOINC applications when your computer "
        "is in use (i.e. when there has been recent mouse or keyboard activity). "
        "Setting this to a low value lets you run BOINC all the time without "
        "impacting your computer's performance. Default 50%."),
        CValidatePercent<double>(&m_preferences->ram_max_used_busy_frac))
    );

    work->AddPreference(new PrefValueText(this,
        _("ram_max_used_idle_frac"),
        _("When computer is idle, use no more than"),
        _("% total memory"),
        _("Limits the amount of RAM used by BOINC applications when your computer "
        "is not in use. Default 90%."),
        CValidatePercent<double>(&m_preferences->ram_max_used_idle_frac))
    );

    PrefGroup* switching = AddGroup(_("Application Switching"));

    switching->AddPreference(new PrefValueBool(this,
        _("leave_apps_in_memory"),
        _("Leave applications in memory while suspended"),
        _("This improves CPU effectiveness at the cost of virtual memory. This "
        "option is not relevant if you are only attached to one project. "
        "Default false."),
        CValidateBool(&m_preferences->leave_apps_in_memory))
    );

}
