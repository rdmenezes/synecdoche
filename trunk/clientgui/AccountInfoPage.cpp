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

#include "AccountInfoPage.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "ValidateEmailAddress.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"
#include "ProjectInfoPage.h"
#include "AccountManagerInfoPage.h"
#include "gui_rpc_client.h"

IMPLEMENT_DYNAMIC_CLASS(CAccountInfoPage, wxWizardPage)

BEGIN_EVENT_TABLE(CAccountInfoPage, wxWizardPage)
    EVT_WIZARD_PAGE_CHANGED(-1, CAccountInfoPage::OnPageChanged)
    EVT_WIZARD_PAGE_CHANGING(-1, CAccountInfoPage::OnPageChanging)
    EVT_WIZARD_CANCEL(-1, CAccountInfoPage::OnCancel)
    EVT_RADIOBUTTON(ID_ACCOUNTCREATECTRL, CAccountInfoPage::OnAccountCreateCtrlSelected)
    EVT_RADIOBUTTON(ID_ACCOUNTUSEEXISTINGCTRL, CAccountInfoPage::OnAccountUseExistingCtrlSelected) 
END_EVENT_TABLE()

/*!
 * CAccountInfoPage constructors
 */

CAccountInfoPage::CAccountInfoPage() {
}

CAccountInfoPage::CAccountInfoPage(CBOINCBaseWizard* parent) {
    Create(parent);
}

/*!
 * AccountInfoPage creator
 */
 
bool CAccountInfoPage::Create(CBOINCBaseWizard* parent) {
    m_pTitleStaticCtrl = NULL;
    m_pAccountQuestionStaticCtrl = NULL;
    m_pAccountInformationStaticCtrl = NULL;
    m_pAccountCreateCtrl = NULL;
    m_pAccountUseExistingCtrl = NULL;
    m_pAccountEmailAddressStaticCtrl = NULL;
    m_pAccountEmailAddressCtrl = NULL;
    m_pAccountPasswordStaticCtrl = NULL;
    m_pAccountPasswordCtrl = NULL;
    m_pAccountConfirmPasswordStaticCtrl = NULL;
    m_pAccountConfirmPasswordCtrl = NULL;
    m_pAccountPasswordRequirmentsStaticCtrl = NULL;
    m_pAccountForgotPasswordCtrl = NULL;

    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create(parent, wizardBitmap);

    CreateControls();
    GetSizer()->Fit(this);
    return TRUE;
}

/*!
 * Control creation for AccountInfoPage
 */

void CAccountInfoPage::CreateControls() {    
    CAccountInfoPage* itemWizardPage56 = this;

    wxBoxSizer* itemBoxSizer57 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage56->SetSizer(itemBoxSizer57);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create(itemWizardPage56, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer57->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxGROW|wxALL, 5);

    m_pAccountQuestionStaticCtrl = new wxStaticText;
    m_pAccountQuestionStaticCtrl->Create(itemWizardPage56, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer57->Add(m_pAccountQuestionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer61 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer61->AddGrowableCol(1);
    itemBoxSizer57->Add(itemFlexGridSizer61, 0, wxGROW|wxALL, 5);

    m_pAccountCreateCtrl = new wxRadioButton;
    m_pAccountCreateCtrl->Create(itemWizardPage56, ID_ACCOUNTCREATECTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_pAccountCreateCtrl->SetValue(TRUE);
    itemFlexGridSizer61->Add(m_pAccountCreateCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountUseExistingCtrl = new wxRadioButton;
    m_pAccountUseExistingCtrl->Create(itemWizardPage56, ID_ACCOUNTUSEEXISTINGCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_pAccountUseExistingCtrl->SetValue(FALSE);
    itemFlexGridSizer61->Add(m_pAccountUseExistingCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountInformationStaticCtrl = new wxStaticText;
    m_pAccountInformationStaticCtrl->Create(itemWizardPage56, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer57->Add(m_pAccountInformationStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer64 = new wxFlexGridSizer(4, 2, 0, 0);
    itemFlexGridSizer64->AddGrowableCol(1);
    itemBoxSizer57->Add(itemFlexGridSizer64, 0, wxEXPAND|wxALL, 0);

    m_pAccountEmailAddressStaticCtrl = new wxStaticText;
    m_pAccountEmailAddressStaticCtrl->Create(itemWizardPage56, ID_ACCOUNTEMAILADDRESSSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer64->Add(m_pAccountEmailAddressStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountEmailAddressCtrl = new wxTextCtrl;
    m_pAccountEmailAddressCtrl->Create(itemWizardPage56, ID_ACCOUNTEMAILADDRESSCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, 22), 0);
    itemFlexGridSizer64->Add(m_pAccountEmailAddressCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountPasswordStaticCtrl = new wxStaticText;
    m_pAccountPasswordStaticCtrl->Create(itemWizardPage56, ID_ACCOUNTPASSWORDSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer64->Add(m_pAccountPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountPasswordCtrl = new wxTextCtrl;
    m_pAccountPasswordCtrl->Create(itemWizardPage56, ID_ACCOUNTPASSWORDCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    itemFlexGridSizer64->Add(m_pAccountPasswordCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountConfirmPasswordStaticCtrl = new wxStaticText;
    m_pAccountConfirmPasswordStaticCtrl->Create(itemWizardPage56, ID_ACCOUNTCONFIRMPASSWORDSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer64->Add(m_pAccountConfirmPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountConfirmPasswordCtrl = new wxTextCtrl;
    m_pAccountConfirmPasswordCtrl->Create(itemWizardPage56, ID_ACCOUNTCONFIRMPASSWORDCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    itemFlexGridSizer64->Add(m_pAccountConfirmPasswordCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer64->Add( 0, 0 );

    m_pAccountPasswordRequirmentsStaticCtrl = new wxStaticText;
    m_pAccountPasswordRequirmentsStaticCtrl->Create(itemWizardPage56, ID_ACCOUNTREQUIREMENTSSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_pAccountPasswordRequirmentsStaticCtrl->SetFont(wxFont(7, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE));
    itemFlexGridSizer64->Add(m_pAccountPasswordRequirmentsStaticCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountForgotPasswordCtrl = new wxHyperlinkCtrl;
    m_pAccountForgotPasswordCtrl->Create(itemWizardPage56, ID_ACCOUNTFORGOTPASSWORDCTRL, _("Forgot your password?"), wxEmptyString);
    itemFlexGridSizer64->Add(m_pAccountForgotPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    // m_pAccountEmailAddressCtrl is setup when the OnPageChange event is fired since
    //   it can be a username or an email address.
    m_pAccountPasswordCtrl->SetValidator(wxTextValidator(wxFILTER_ASCII, &m_strAccountPassword));
    m_pAccountConfirmPasswordCtrl->SetValidator(wxTextValidator(wxFILTER_ASCII, &m_strAccountConfirmPassword));
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAccountInfoPage::GetPrev() const {
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAccountInfoPage::GetNext() const {
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (CheckWizardTypeByPage<CWizardAttachProject>(this)) {
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROCESSINGPAGE);
    } else if (CheckWizardTypeByPage<CWizardAccountManager>(this)) {
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTMANAGERPROCESSINGPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */

bool CAccountInfoPage::ShowToolTips() {
    return TRUE;
}

/// Check if a new account should be created.
///
/// \return True is a new account should be created, false if an
///         existing account should be used.
bool CAccountInfoPage::CreateNewAccount() const {
    return m_pAccountCreateCtrl->GetValue();
}

/// Disables the validators for all controls on this page.
void CAccountInfoPage::DisableValidators() {
    m_pAccountEmailAddressCtrl->SetValidator(wxDefaultValidator);
    m_pAccountPasswordCtrl->SetValidator(wxDefaultValidator);
    m_pAccountConfirmPasswordCtrl->SetValidator(wxDefaultValidator);
}

/*!
 * Get bitmap resources
 */
 
wxBitmap CAccountInfoPage::GetBitmapResource(const wxString& WXUNUSED(name)) {
    return wxNullBitmap;
}
 
/*!
 * Get icon resources
 */
 
wxIcon CAccountInfoPage::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTINFOPAGE
 */
 
void CAccountInfoPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    CBOINCBaseWizard*      bw = dynamic_cast<CBOINCBaseWizard*>(GetParent());
    wxASSERT(bw);

	PROJECT_CONFIG*        pc = bw->GetProjectConfig();
    CSkinAdvanced*         pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CSkinWizardATAM*       pSkinWizardATAM = wxGetApp().GetSkinManager()->GetWizards()->GetWizardATAM();

    wxASSERT(pSkinAdvanced);
    wxASSERT(pSkinWizardATAM);
    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pAccountQuestionStaticCtrl);
    wxASSERT(m_pAccountInformationStaticCtrl);
    wxASSERT(m_pAccountCreateCtrl);
    wxASSERT(m_pAccountUseExistingCtrl);
    wxASSERT(m_pAccountEmailAddressStaticCtrl);
    wxASSERT(m_pAccountEmailAddressCtrl);
    wxASSERT(m_pAccountPasswordStaticCtrl);
    wxASSERT(m_pAccountPasswordCtrl);
    wxASSERT(m_pAccountConfirmPasswordStaticCtrl);
    wxASSERT(m_pAccountConfirmPasswordCtrl);
    wxASSERT(m_pAccountPasswordRequirmentsStaticCtrl);
    wxASSERT(m_pAccountForgotPasswordCtrl);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));
    wxASSERT(wxDynamicCast(pSkinWizardATAM, CSkinWizardATAM));

    static bool bRunOnce = true;
    if (bRunOnce) {
        bRunOnce = false;
        if (!CheckWizardTypeByPage<CWizardAccountManager>(this)) {
            m_pAccountCreateCtrl->SetValue(true);
            m_pAccountUseExistingCtrl->SetValue(false);
        }
    }

    if (CheckWizardTypeByPage<CWizardAccountManager>(this)) {
        m_pAccountQuestionStaticCtrl->Hide();
        m_pAccountCreateCtrl->SetValue(false);
        m_pAccountCreateCtrl->Hide();
        m_pAccountUseExistingCtrl->SetValue(true);
        m_pAccountUseExistingCtrl->Hide();
        m_pAccountConfirmPasswordStaticCtrl->Hide();
        m_pAccountConfirmPasswordCtrl->Hide();
        m_pAccountPasswordRequirmentsStaticCtrl->Hide();
    }

    if (!CheckWizardTypeByPage<CWizardAccountManager>(this)) {
        if (pc->account_creation_disabled || pc->client_account_creation_disabled) {
            m_pAccountCreateCtrl->SetValue(false);
            m_pAccountCreateCtrl->Hide();
            m_pAccountUseExistingCtrl->SetValue(true);
            m_pAccountUseExistingCtrl->Hide();
        } else {
            m_pAccountCreateCtrl->Show();
            m_pAccountCreateCtrl->Enable();
            m_pAccountUseExistingCtrl->Show();
        }
    }

    m_pTitleStaticCtrl->SetLabel(_("User information"));

    if (!CheckWizardTypeByPage<CWizardAccountManager>(this)) {
        if (pc->client_account_creation_disabled) {
            m_pAccountQuestionStaticCtrl->SetLabel(_("Please enter your account information\n(to create an account, visit the project's web site)"));
        } else if (pc->account_creation_disabled) {
            m_pAccountQuestionStaticCtrl->SetLabel(_("This project is not currently accepting new accounts.\nYou can attach only if you already have an account."));
        } else {
            m_pAccountQuestionStaticCtrl->SetLabel(_("Are you already running this project?"));
        }
        m_pAccountCreateCtrl->SetLabel(_("&No, new user"));
        m_pAccountUseExistingCtrl->SetLabel(_("&Yes, existing user"));
    } else {
        if (pSkinAdvanced->IsBranded() && 
            !pSkinWizardATAM->GetAccountInfoMessage().IsEmpty()) {
            m_pAccountInformationStaticCtrl->SetLabel(pSkinWizardATAM->GetAccountInfoMessage());
        }
    }

    if (m_pAccountUseExistingCtrl->GetValue()) {
        m_pAccountConfirmPasswordStaticCtrl->Hide();
        m_pAccountConfirmPasswordCtrl->Hide();
        m_pAccountPasswordRequirmentsStaticCtrl->Hide();
        m_pAccountPasswordStaticCtrl->SetLabel(_("&Password:"));
    } else {
        m_pAccountConfirmPasswordStaticCtrl->Show();
        m_pAccountConfirmPasswordCtrl->Show();
        m_pAccountPasswordRequirmentsStaticCtrl->Show();
        m_pAccountPasswordStaticCtrl->SetLabel(_("Choose a &password:"));
        m_pAccountConfirmPasswordStaticCtrl->SetLabel(_("C&onfirm password:"));
    }

    if (!bw->GetProjectName().IsEmpty()) {
        wxString strQuestion;
        strQuestion.Printf(_("Are you already running %s?"), bw->GetProjectName().c_str());
        m_pAccountQuestionStaticCtrl->SetLabel(strQuestion);
    }

    if (pc->uses_username) {
        if (CheckWizardTypeByPage<CWizardAccountManager>(this)) {
            if (pSkinAdvanced->IsBranded() && 
                !pSkinWizardATAM->GetAccountInfoMessage().IsEmpty()) {
                m_pAccountInformationStaticCtrl->SetLabel(pSkinWizardATAM->GetAccountInfoMessage());
            }
        }

        m_pAccountEmailAddressStaticCtrl->SetLabel(_("&Username:"));
        m_pAccountEmailAddressCtrl->SetValidator(wxTextValidator(wxFILTER_ASCII, &m_strAccountEmailAddress));
    } else {
        if (CheckWizardTypeByPage<CWizardAccountManager>(this)) {
            if (pSkinAdvanced->IsBranded() && 
                !pSkinWizardATAM->GetAccountInfoMessage().IsEmpty()) {
                m_pAccountInformationStaticCtrl->SetLabel(pSkinWizardATAM->GetAccountInfoMessage());
            }
        }

        m_pAccountEmailAddressStaticCtrl->SetLabel(_("&Email address:"));
        m_pAccountEmailAddressCtrl->SetValidator(CValidateEmailAddress(&m_strAccountEmailAddress));
    }

    if (pc->min_passwd_length) {
        wxString str;
        str.Printf(_("minimum length %d"), pc->min_passwd_length);
        m_pAccountPasswordRequirmentsStaticCtrl->SetLabel(str);
    }

    m_pAccountForgotPasswordCtrl->SetLabel(_("Forgot your password?"));

    if (!CheckWizardTypeByPage<CWizardAccountManager>(this)) {
        CWizardAttachProject* pWAP = dynamic_cast<CWizardAttachProject*>(GetParent());
        wxASSERT(pWAP);
        m_pAccountForgotPasswordCtrl->SetURL(
            wxString(pWAP->GetProjectInfoPage()->GetProjectURL() + _T("get_passwd.php"))
        );
    } else {
        CWizardAccountManager* pWAM = dynamic_cast<CWizardAccountManager*>(GetParent());
        wxASSERT(pWAM);
        m_pAccountForgotPasswordCtrl->SetURL(
            wxString(pWAM->GetAccountManagerInfoPage()->GetProjectURL() + _T("get_passwd.php"))
        );
    }

    Fit();
    m_pAccountEmailAddressCtrl->SetFocus();
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTINFOPAGE
 */
 
void CAccountInfoPage::OnPageChanging(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;
 
    if (!CHECK_CLOSINGINPROGRESS()) {
        wxString strTitle;
        if (CheckWizardTypeByPage<CWizardAttachProject>(this)) {
            strTitle = _("Attach to project");
        } else if (CheckWizardTypeByPage<CWizardAccountManager>(this)) {
            CWizardAccountManager* wiz = dynamic_cast<CWizardAccountManager*>(GetParent());
            if (wiz->IsUpdateWizard()) {
                strTitle = _("Update account manager");
            } else {
                strTitle = _("Attach to account manager");
            }
        }
        wxString strMessage = wxT("");
        bool     bDisplayError = false;
 
        // Verify minimum password length
        unsigned int iMinLength = ((CBOINCBaseWizard*)GetParent())->GetProjectConfig()->min_passwd_length;
        wxString strPassword = m_pAccountPasswordCtrl->GetValue();
        if (strPassword.Length() < iMinLength) {
            if (CheckWizardTypeByPage<CWizardAttachProject>(this)) {
                strMessage.Printf(_("The minimum password length for this project is %d. Please enter a different password."),
                    iMinLength);
            } else if (CheckWizardTypeByPage<CWizardAccountManager>(this)) {
                strMessage.Printf(_("The minimum password length for this account manager is %d. Please enter a different password."),
                    iMinLength);
            }

            bDisplayError = true;
        }

        if ((!CheckWizardTypeByPage<CWizardAccountManager>(this)) && (m_pAccountCreateCtrl->GetValue())) {
            // Verify that the password and confirmation password math.
            if (m_pAccountPasswordCtrl->GetValue() != m_pAccountConfirmPasswordCtrl->GetValue()) {
                strMessage = _("The password and confirmation password do not match. Please type them again.");
                bDisplayError = true;
            }
        }
 
        if (bDisplayError) {
            ::wxMessageBox(strMessage, strTitle, wxICON_ERROR | wxOK, this);
            event.Veto();
        }
    }
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTINFOPAGE
 */
 
void CAccountInfoPage::OnCancel(wxWizardEvent& event) {
    PROCESS_CANCELEVENT(event);
}
 
/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTUSEXISTINGBUTTON
 */
 
void CAccountInfoPage::OnAccountUseExistingCtrlSelected(wxCommandEvent& WXUNUSED(event)) {
    m_pAccountPasswordStaticCtrl->SetLabel(_("&Password:"));
    m_pAccountConfirmPasswordStaticCtrl->Hide();
    m_pAccountConfirmPasswordCtrl->Hide();
    m_pAccountPasswordRequirmentsStaticCtrl->Hide();
    m_pAccountEmailAddressCtrl->SetFocus();
    Fit();
}
  
/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTCREATEBUTTON
 */
 
void CAccountInfoPage::OnAccountCreateCtrlSelected(wxCommandEvent& WXUNUSED(event)) {
    m_pAccountPasswordStaticCtrl->SetLabel(_("Choose a &password:"));
    m_pAccountConfirmPasswordStaticCtrl->Show();
    m_pAccountConfirmPasswordCtrl->Show();
    m_pAccountPasswordRequirmentsStaticCtrl->Show();
    m_pAccountEmailAddressCtrl->SetFocus();
    Fit();
}
