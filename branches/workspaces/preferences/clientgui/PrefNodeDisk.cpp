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
#include "ValidatePercent.h"
#include "PrefGridBase.h"
#include "PrefNodeDisk.h"


IMPLEMENT_DYNAMIC_CLASS(PrefNodeDisk, PrefGridBase)

PrefNodeDisk::PrefNodeDisk(wxWindow* parent, GLOBAL_PREFS* preferences)
: PrefGridBase(parent, preferences) {

    PrefGroup* quotas = AddGroup(_("Disk Quotas"));

    quotas->AddPreference(new PrefValueText(this,
        _("Maximum disk space to use (Gigabytes)"),
        _("This option is combined with the other disk restrictions to "
        "determine the maximum space used for project application data."),
        _("10 Gigabytes"),
        ValidateNumber<double>(&m_preferences->disk_max_used_gb))
    );

    quotas->AddPreference(new PrefValueText(this,
        _("Minimum free disk space (Gigabytes)"),
        _("This option is combined with the other disk restrictions to "
        "determine the maximum space used for project application data."),
        _("0.1 Gigabytes"),
        ValidateNumber<double>(&m_preferences->disk_min_free_gb))
    );

    quotas->AddPreference(new PrefValueText(this,
        _("Maximum percentage of disk space to use"),
        _("This option is combined with the other disk restrictions to "
        "determine the maximum space used for project application data."),
        _("50%"),
        ValidateNumber<double>(&m_preferences->disk_max_used_pct))
    );

    PrefGroup* access = AddGroup(_("Disk Access"));

    access->AddPreference(new PrefValueText(this,
        _("Minimum interval between disk writes (seconds)"),
        _("This setting is not observed by all projects. Use this setting to "
        "prevent the drive spinning up unnecessarily. Also refer to your power "
        "management settings."),
        _("60 seconds"),
        ValidateNumber<double>(&m_preferences->disk_interval))
    );

    PrefGroup* vm = AddGroup(_("Virtual Memory"));

    vm->AddPreference(new PrefValueText(this,
         _("Maximum percentage of page file to use"),
        _("Restrict the amount of virtual memory that may be used by tasks. This "
        "setting should be used with care if your page file size is managed by "
        "the operating system."),
        _("75%"),
        ValidatePercent<double>(&m_preferences->vm_max_used_frac))
    );
}
