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
#include "ValidateMultiplied.h"
#include "ValidateYesNo.h"
#include "PrefGridBase.h"
#include "PrefNodeNetwork.h"


IMPLEMENT_DYNAMIC_CLASS(PrefNodeNetwork, PrefGridBase)

PrefNodeNetwork::PrefNodeNetwork(wxWindow* parent, GLOBAL_PREFS* preferences)
: PrefGridBase(parent, preferences) {

    PrefGroup* connect = AddGroup(_("Connection"));

    connect->AddPreference(new PrefValueBool(this,
        _("Confirm before connecting to Internet"),
        _("BOINC will only try and get an Internet connection when it needs one."),
        _("Yes"),
        ValidateYesNo(&m_preferences->confirm_before_connecting))
    );

    connect->AddPreference(new PrefValueBool(this,
        _("Disconnect when done"),
        _("BOINC will only disconnect if it initiated the Internet connection."),
        _("No"),
        ValidateYesNo(&m_preferences->hangup_if_dialed))
    );

    connect->AddPreference(new PrefValueText(this,
        _("Approximate connection interval (days)"),
        _("BOINC will use this as a hint for buffering work between connections. "
        "BOINC will still use the Internet more frequently if a connection "
        "is available."),
        _("0.1 days"),
        ValidateNumber<double>(&m_preferences->work_buf_min_days))
    );

    PrefGroup* limits = AddGroup(_("Bandwidth Limits"));

    limits->AddPreference(new PrefValueText(this,
        _("Maximum upload rate (Kbytes/sec)"),
        _("Zero means upload rate is unrestricted."),
        _("Unrestricted"),
        ValidateMultiplied<double>(&m_preferences->max_bytes_sec_up, 0.001))
    );

    limits->AddPreference(new PrefValueText(this,
        _("Maximum download rate (Kbytes/sec)"),
        _("Zero means download rate is unrestricted."),
        _("Unrestricted"),
        ValidateMultiplied<double>(&m_preferences->max_bytes_sec_down, 0.001))
    );

    PrefGroup* errors = AddGroup(_("Error Checking"));

    errors->AddPreference(new PrefValueBool(this,
        _("Skip image file verification"),
        _("Some dialup Internet Service Providers compress image downloads on the fly. "
        "If you can't use a better ISP, use this option to ignore the modified images "
        "until you can switch to a better ISP."),
        _("No"),
        ValidateYesNo(&m_preferences->dont_verify_images))
    );
}
