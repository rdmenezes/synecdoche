// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
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
#ifndef _WIZ_ACCOUNTINFOPAGE_H_
#define _WIZ_ACCOUNTINFOPAGE_H_

class wxHyperlinkCtrl;

/*!
 * CAccountInfoPage class declaration
 */

class CAccountInfoPage: public wxWizardPageEx
{    
    DECLARE_DYNAMIC_CLASS( CAccountInfoPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAccountInfoPage( );

    CAccountInfoPage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAccountInfoPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTINFOPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTINFOPAGE
    void OnPageChanging( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTINFOPAGE
    void OnCancel( wxWizardExEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTCREATECTRL
    void OnAccountCreateCtrlSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTUSEEXISTINGCTRL
    void OnAccountUseExistingCtrlSelected( wxCommandEvent& event );

////@end CAccountInfoPage event handler declarations

////@begin CAccountInfoPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    wxString GetAccountEmailAddress() const { return m_strAccountEmailAddress ; }
    void SetAccountEmailAddress(wxString value) { m_strAccountEmailAddress = value ; }

    wxString GetAccountPassword() const { return m_strAccountPassword ; }
    void SetAccountPassword(wxString value) { m_strAccountPassword = value ; }

    wxString GetAccountConfirmPassword() const { return m_strAccountConfirmPassword ; }
    void SetAccountConfirmPassword(wxString value) { m_strAccountConfirmPassword = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAccountInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAccountInfoPage member variables
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
////@end CAccountInfoPage member variables
};

#endif // _WIZ_ACCOUNTINFOPAGE_H_