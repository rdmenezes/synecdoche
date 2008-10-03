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

#ifndef PREFLOCATIONMANAGER_H
#define PREFLOCATIONMANAGER_H

#include "prefs.h"
#include "PrefTreeBook.h"

#define PREF_DLG_MARGIN 4

class PrefLocationManager : public wxDialog {

    DECLARE_DYNAMIC_CLASS(PrefLocationManager)
    DECLARE_EVENT_TABLE()

public:
    PrefLocationManager(wxWindow* parent=NULL);
    virtual ~PrefLocationManager();

    void OnOK(wxCommandEvent& event);

protected:
    bool SaveState();
    bool RestoreState();

private:
    GLOBAL_PREFS        prefs;

    PrefTreeBook*      m_treeBook;

    wxButton*           m_buttonOkay;
    wxButton*           m_buttonCancel;
    wxButton*           m_buttonHelp;
};

#endif // PREFLOCATIONMANAGER_H

