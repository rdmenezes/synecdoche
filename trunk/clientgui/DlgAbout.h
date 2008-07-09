// Synecdoche
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
// Copyright (C) 2005 University of California
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
//
#ifndef _DLGABOUT_H_
#define _DLGABOUT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgAbout.cpp"
#endif

#include "hyperlink.h"
#include "wx/statline.h"

class wxHyperLink;


class CDlgAbout: public wxDialog {

    DECLARE_DYNAMIC_CLASS(CDlgAbout)

public:
    /// Constructors
    CDlgAbout() {}
    CDlgAbout(wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE
        );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& caption = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );

private:
    /// Creates the controls and sizers
    void CreateControls(CSkinAdvanced* pSkinAdvanced);
};

#endif
