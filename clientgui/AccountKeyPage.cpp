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

#include "AccountKeyPage.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "ValidateAccountKey.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"

IMPLEMENT_DYNAMIC_CLASS(CAccountKeyPage, wxWizardPage)

BEGIN_EVENT_TABLE(CAccountKeyPage, wxWizardPage)
    EVT_WIZARD_PAGE_CHANGED(-1, CAccountKeyPage::OnPageChanged)
    EVT_WIZARD_CANCEL(-1, CAccountKeyPage::OnCancel)
END_EVENT_TABLE()

/*!
 * CAccountKeyPage constructors
 */
 
CAccountKeyPage::CAccountKeyPage() {
}

CAccountKeyPage::CAccountKeyPage(CBOINCBaseWizard* parent) {
    Create(parent);
}

/*!
 * CAuthenticatorPage creator
 */
 
bool CAccountKeyPage::Create(CBOINCBaseWizard* parent) {
    m_pTitleStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;
    m_pAccountKeyExampleDescriptionStaticCtrl = NULL;
    m_pAccountKeyExampleStaticCtrl = NULL;
    m_pAccountKeyStaticCtrl = NULL;
    m_pAccountKeyCtrl = NULL;

    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create(parent, wizardBitmap);

    CreateControls();
    GetSizer()->Fit(this);
    return TRUE;
}

/*!
 * Control creation for CAuthenticatorPage
 */

void CAccountKeyPage::CreateControls() {    
    CAccountKeyPage* itemWizardPage44 = this;

    wxBoxSizer* itemBoxSizer45 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage44->SetSizer(itemBoxSizer45);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create(itemWizardPage44, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer45->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer45->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDirectionsStaticCtrl = new wxStaticText;
    m_pDirectionsStaticCtrl->Create(itemWizardPage44, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer45->Add(m_pDirectionsStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pAccountKeyExampleDescriptionStaticCtrl = new wxStaticText;
    m_pAccountKeyExampleDescriptionStaticCtrl->Create(itemWizardPage44, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer45->Add(m_pAccountKeyExampleDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pAccountKeyExampleStaticCtrl = new wxStaticText;
    m_pAccountKeyExampleStaticCtrl->Create(itemWizardPage44, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_pAccountKeyExampleStaticCtrl->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxNORMAL, FALSE, _T("Courier New")));
    itemBoxSizer45->Add(m_pAccountKeyExampleStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    itemBoxSizer45->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer53 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer53->AddGrowableCol(1);
    itemBoxSizer45->Add(itemFlexGridSizer53, 0, wxGROW|wxALL, 5);

    m_pAccountKeyStaticCtrl = new wxStaticText;
    m_pAccountKeyStaticCtrl->Create(itemWizardPage44, ID_ACCOUNTKEYSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer53->Add(m_pAccountKeyStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountKeyCtrl = new wxTextCtrl;
    m_pAccountKeyCtrl->Create(itemWizardPage44, ID_ACCOUNTKEYCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer53->Add(m_pAccountKeyCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_pAccountKeyCtrl->SetValidator(CValidateAccountKey(&m_strAccountKey));
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CAccountKeyPage::GetPrev() const {
    return PAGE_TRANSITION_BACK;
}
  
/*!
 * Gets the next page.
 */
 
wxWizardPage* CAccountKeyPage::GetNext() const {
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROCESSINGPAGE);
    }
}
 
/*!
 * Should we show tooltips?
 */
 
bool CAccountKeyPage::ShowToolTips() {
    return TRUE;
}

/// Disables the validators for all controls on this page.
void CAccountKeyPage::DisableValidators() {
    m_pAccountKeyCtrl->SetValidator(wxDefaultValidator);
}

/*!
 * Get bitmap resources
 */
 
wxBitmap CAccountKeyPage::GetBitmapResource(const wxString& WXUNUSED(name)) {
    return wxNullBitmap;
}
  
/*!
 * Get icon resources
 */

wxIcon CAccountKeyPage::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTKEYPAGE
 */
 
void CAccountKeyPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);
    wxASSERT(m_pAccountKeyExampleDescriptionStaticCtrl);
    wxASSERT(m_pAccountKeyExampleStaticCtrl);
    wxASSERT(m_pAccountKeyStaticCtrl);
    wxASSERT(m_pAccountKeyCtrl);

    m_pTitleStaticCtrl->SetLabel(_("Enter account key"));
    m_pDirectionsStaticCtrl->SetLabel(_("This project uses an \"account key\" to identify you.\n\nGo to the project's web site to create an account. Your account\nkey will be emailed to you."));
    m_pAccountKeyExampleDescriptionStaticCtrl->SetLabel(_("An account key looks like:"));
    m_pAccountKeyExampleStaticCtrl->SetLabel(_("82412313ac88e9a3638f66ea82186948"));
    m_pAccountKeyStaticCtrl->SetLabel(_("Account key:"));

    Fit();
    m_pAccountKeyCtrl->SetFocus();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTKEYPAGE
 */
 
void CAccountKeyPage::OnCancel(wxWizardEvent& event) {
    PROCESS_CANCELEVENT(event);
}
