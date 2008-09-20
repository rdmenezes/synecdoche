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
#ifndef WIZ_ACCOUNTKEYPAGE_H
#define WIZ_ACCOUNTKEYPAGE_H

#include <wx/wizard.h>

class CBOINCBaseWizard;
class wxStaticText;
class wxTextCtrl;

class CAccountKeyPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS(CAccountKeyPage)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAccountKeyPage();
    CAccountKeyPage(CBOINCBaseWizard* parent);

    /// Creation
    bool Create(CBOINCBaseWizard* parent);

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTKEYPAGE
    void OnPageChanged(wxWizardEvent& event);

    /// wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTKEYPAGE
    void OnCancel(wxWizardEvent& event);

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    wxString GetAccountKey() const { return m_strAccountKey; }
    void SetAccountKey(wxString value) { m_strAccountKey = value; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource(const wxString& name);

    /// Retrieves icon resources
    wxIcon GetIconResource(const wxString& name);

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Disables the validators for all controls on this page.
    void DisableValidators();

private:
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDirectionsStaticCtrl;
    wxStaticText* m_pAccountKeyExampleDescriptionStaticCtrl;
    wxStaticText* m_pAccountKeyExampleStaticCtrl;
    wxStaticText* m_pAccountKeyStaticCtrl;
    wxTextCtrl* m_pAccountKeyCtrl;
    wxString m_strAccountKey;
};

#endif // WIZ_ACCOUNTKEYPAGE_H
