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

#ifndef _PREFFRAME_H_
#define _PREFFRAME_H_

#include "prefs.h"
#include "PrefTreeBook.h"

#define PREF_DLG_MARGIN 4
#define ID_LOCATIONMANAGER 7000

class PrefFrame : public wxDialog {

    DECLARE_DYNAMIC_CLASS(PrefFrame)
    DECLARE_EVENT_TABLE()

public:
    PrefFrame(wxWindow* parent=NULL);
    virtual ~PrefFrame();

    void OnOK(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);
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

#endif // _PREFFRAME_H_
