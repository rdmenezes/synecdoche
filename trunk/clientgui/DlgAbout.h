// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
// Copyright (C) 2005 University of California
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
//
#ifndef _DLGABOUT_H_
#define _DLGABOUT_H_

#include <wx/dialog.h>

class CSkinAdvanced;

#define ID_DIALOG 10000

class CDlgAbout: public wxDialog {

    DECLARE_DYNAMIC_CLASS(CDlgAbout)

public:
    /// Constructors
    CDlgAbout() {}
    CDlgAbout(wxWindow* parent,
        wxWindowID id = ID_DIALOG,
        const wxString& caption = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE
        );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = ID_DIALOG, const wxString& caption = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );

private:
    /// Creates the controls and sizers
    void CreateControls(CSkinAdvanced* pSkinAdvanced);
};

#endif
