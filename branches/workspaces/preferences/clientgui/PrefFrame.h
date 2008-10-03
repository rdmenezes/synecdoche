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

#ifndef PREFFRAME_H
#define PREFFRAME_H

#include "prefs.h"
#include "PrefTreeBook.h"

#define PREF_DLG_MARGIN 4
#define ID_LOCATIONMANAGER 7000

/// Preferences dialog class.
/// The preferences dialog is the user interface for all local and global
/// preferences.
class PrefFrame : public wxDialog {

    DECLARE_DYNAMIC_CLASS(PrefFrame)
    DECLARE_EVENT_TABLE()

public:
    PrefFrame(wxWindow* parent=NULL);
    virtual ~PrefFrame();

    /// Handler for OK button.
    void OnOK(wxCommandEvent& event);

    /// Handler for Help button.
    void OnHelp(wxCommandEvent& event);

    /// Handler for Location Manager button.
    void OnLocationManager(wxCommandEvent& ev);

protected:
    bool SaveState();
    bool RestoreState();

private:
    GLOBAL_PREFS        prefs;

    PrefTreeBook*      m_treeBook;

    wxButton*           m_buttonOkay;
    wxButton*           m_buttonCancel;
    wxButton*           m_buttonHelp;

//    std::vector<VENUE>       m_venues;
};

#endif // PREFFRAME_H
