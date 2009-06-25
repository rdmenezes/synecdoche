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
#ifndef WIZ_ACCOUNTMANAGERINFOPAGE_H
#define WIZ_ACCOUNTMANAGERINFOPAGE_H

#include <wx/wizard.h>

class CBOINCBaseWizard;
class wxHyperlinkCtrl;
class wxStaticText;
class wxTextCtrl;

class CAccountManagerInfoPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS(CAccountManagerInfoPage)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAccountManagerInfoPage();
    CAccountManagerInfoPage(CBOINCBaseWizard* parent);

    /// Creation
    bool Create(CBOINCBaseWizard* parent);

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnPageChanged(wxWizardEvent& event);

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnPageChanging(wxWizardEvent& event);

    /// wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnCancel(wxWizardEvent& event);

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    wxString GetProjectURL() const { return m_strProjectURL; }
    void SetProjectURL(wxString value) { m_strProjectURL = value; }

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
    wxStaticText* m_pDescriptionStaticCtrl;
    wxStaticText* m_pDescription2StaticCtrl;
    wxStaticText* m_pProjectUrlStaticCtrl;
    wxTextCtrl* m_pProjectUrlCtrl;
    wxStaticText* m_pBOINCPromoStaticCtrl;
    wxHyperlinkCtrl* m_pBOINCPromoUrlCtrl;
    wxString m_strProjectURL;
};

#endif // WIZ_ACCOUNTMANAGERINFOPAGE_H
