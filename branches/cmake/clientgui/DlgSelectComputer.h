// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Peter Kortschack
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

/// \file
/// Class implementing the dialog for selecting a different computer in the Manager.

#ifndef _DLGSELECTCOMPUTER_H_
#define _DLGSELECTCOMPUTER_H_

#include "wx/valgen.h"

#define SYMBOL_CDLGSELECTCOMPUTER_STYLE wxDEFAULT_DIALOG_STYLE
#define SYMBOL_CDLGSELECTCOMPUTER_TITLE wxT("")
#define SYMBOL_CDLGSELECTCOMPUTER_SIZE wxSize(400, 300)
#define SYMBOL_CDLGSELECTCOMPUTER_POSITION wxDefaultPosition
#define ID_SELECTCOMPUTERNAME 10001

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

class CDlgSelectComputer: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS(CDlgSelectComputer)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgSelectComputer();
    CDlgSelectComputer(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& caption = SYMBOL_CDLGSELECTCOMPUTER_TITLE, const wxPoint& pos = SYMBOL_CDLGSELECTCOMPUTER_POSITION, const wxSize& size = SYMBOL_CDLGSELECTCOMPUTER_SIZE, long style = SYMBOL_CDLGSELECTCOMPUTER_STYLE);

    wxString GetComputerName() const { return m_strComputerName; }
    void SetComputerName(wxString value) { m_strComputerName = value; }

    /// Set the list of most recently used computers.
    void SetMRUList(const wxArrayString& mru_list);

    wxString GetComputerPassword() const { return m_strComputerPassword; }
    void SetComputerPassword(wxString value) { m_strComputerPassword = value; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource(const wxString& name);

    /// Retrieves icon resources
    wxIcon GetIconResource(const wxString& name);

    /// Should we show tooltips?
    static bool ShowToolTips();

private:
    /// Creation
    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& caption = SYMBOL_CDLGSELECTCOMPUTER_TITLE, const wxPoint& pos = SYMBOL_CDLGSELECTCOMPUTER_POSITION, const wxSize& size = SYMBOL_CDLGSELECTCOMPUTER_SIZE, long style = SYMBOL_CDLGSELECTCOMPUTER_STYLE);

    /// Creates the controls and sizers.
    void CreateControls();

    void OnComputerNameUpdated(wxCommandEvent& event);

private:
    wxComboBox* m_ComputerNameCtrl;
    wxTextCtrl* m_ComputerPasswordCtrl;
    wxString m_strComputerName;
    wxString m_strComputerPassword;
};

#endif // _DLGSELECTCOMPUTER_H_
