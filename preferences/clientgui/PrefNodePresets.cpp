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
#include "PrefNodeBase.h"
#include "PrefNodePresets.h"


IMPLEMENT_DYNAMIC_CLASS(PrefNodePresets, PrefNodeBase)

PrefNodePresets::PrefNodePresets(wxWindow* parent, GLOBAL_PREFS* preferences)
: PrefNodeBase(parent, preferences) {

    PrefGroup* work = AddGroup(_("Presets"));

    //wxRadioButton* rb1 = new wxRadioButton(this, wxID_ANY, _("Minimum Impact"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    //wxRadioButton* rb2 = new wxRadioButton(this, wxID_ANY, _("Maximum Output"));
    //wxRadioButton* rb3 = new wxRadioButton(this, wxID_ANY, _("Power Saving"));
    //wxRadioButton* rb4 = new wxRadioButton(this, wxID_ANY, _("Custom"));

    //work->Add(rb1, 0, wxALL, 2);
    //work->Add(rb2, 0, wxALL, 2);
    //work->Add(rb3, 0, wxALL, 2);
    //work->Add(rb4, 0, wxALL, 2);

    work->AddPreference(new PrefValueButton(this,
        _("Set my profile so that it will have a minimal amount of impact on my computer."),
        _("Minimum Impact"),
        _("See the documentation to find out which options this will set."),
        wxID_ANY));

    work->AddPreference(new PrefValueButton(this,
        _("Set my profile so that it will do the maximum amount of research possible on my computer."),
        _("Maximum Output"),
        _("See the documentation to find out which options this will set."),
        wxID_ANY));

    work->AddPreference(new PrefValueButton(this,
        _("Set my profile so that my computer will save power while not in use."),
        _("Power Saving"),
        _("See the documentation to find out which options this will set."),
        wxID_ANY));

    PrefGroup* defaults = AddGroup(_("Defaults"));

    defaults->AddPreference(new PrefValueButton(this,
        _("Reset all preferences to their default values."),
        _("Reset"),
        _("See the help text for each preference to see the default value."),
        wxID_ANY));

}
