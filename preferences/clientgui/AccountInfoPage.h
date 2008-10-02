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
#ifndef WIZ_ACCOUNTINFOPAGE_H
#define WIZ_ACCOUNTINFOPAGE_H

#include <wx/wizard.h>

class CBOINCBaseWizard;
class wxHyperlinkCtrl;
class wxTextCtrl;
class wxStaticText;
class wxRadioButton;

class CAccountInfoPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS(CAccountInfoPage)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAccountInfoPage();
    CAccountInfoPage(CBOINCBaseWizard* parent);

    /// Creation
    bool Create(CBOINCBaseWizard* parent);

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTINFOPAGE
    void OnPageChanged(wxWizardEvent& event);

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTINFOPAGE
    void OnPageChanging(wxWizardEvent& event);

    /// wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTINFOPAGE
    void OnCancel(wxWizardEvent& event);

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTCREATECTRL
    void OnAccountCreateCtrlSelected(wxCommandEvent& event);

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTUSEEXISTINGCTRL
    void OnAccountUseExistingCtrlSelected(wxCommandEvent& event);

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    wxString GetAccountEmailAddress() const { return m_strAccountEmailAddress; }
    void SetAccountEmailAddress(wxString value) { m_strAccountEmailAddress = value; }

    wxString GetAccountPassword() const { return m_strAccountPassword; }
    void SetAccountPassword(wxString value) { m_strAccountPassword = value; }

    wxString GetAccountConfirmPassword() const { return m_strAccountConfirmPassword; }
    void SetAccountConfirmPassword(wxString value) { m_strAccountConfirmPassword = value; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource(const wxString& name);

    /// Retrieves icon resources
    wxIcon GetIconResource(const wxString& name);

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Check if a new account should be created.
    bool CreateNewAccount() const;

    /// Disables the validators for all controls on this page.
    void DisableValidators();

private:
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pAccountQuestionStaticCtrl;
    wxRadioButton* m_pAccountCreateCtrl;
    wxRadioButton* m_pAccountUseExistingCtrl;
    wxStaticText* m_pAccountInformationStaticCtrl;
    wxStaticText* m_pAccountEmailAddressStaticCtrl;
    wxTextCtrl* m_pAccountEmailAddressCtrl;
    wxStaticText* m_pAccountPasswordStaticCtrl;
    wxTextCtrl* m_pAccountPasswordCtrl;
    wxStaticText* m_pAccountConfirmPasswordStaticCtrl;
    wxTextCtrl* m_pAccountConfirmPasswordCtrl;
    wxStaticText* m_pAccountPasswordRequirmentsStaticCtrl;
    wxString m_strAccountEmailAddress;
    wxString m_strAccountPassword;
    wxString m_strAccountConfirmPassword;
    wxHyperlinkCtrl* m_pAccountForgotPasswordCtrl;
};

#endif // WIZ_ACCOUNTINFOPAGE_H
