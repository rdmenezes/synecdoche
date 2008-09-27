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

#include "AlreadyExistsPage.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"

IMPLEMENT_DYNAMIC_CLASS(CErrAlreadyExistsPage, wxWizardPage)

BEGIN_EVENT_TABLE(CErrAlreadyExistsPage, wxWizardPage)
    EVT_WIZARD_PAGE_CHANGED(-1, CErrAlreadyExistsPage::OnPageChanged)
    EVT_WIZARD_CANCEL(-1, CErrAlreadyExistsPage::OnCancel)
END_EVENT_TABLE()

/*!
 * CErrAlreadyExistsPage constructors
 */

CErrAlreadyExistsPage::CErrAlreadyExistsPage() {
}

CErrAlreadyExistsPage::CErrAlreadyExistsPage(CBOINCBaseWizard* parent) {
    Create(parent);
}

/*!
 * CErrAccountAlreadyExists creator
 */
 
bool CErrAlreadyExistsPage::Create(CBOINCBaseWizard* parent) {
    m_pTitleStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;

    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create(parent, wizardBitmap);

    CreateControls();
    GetSizer()->Fit(this);
    return TRUE;
}

/*!
 * Control creation for CErrAccountAlreadyExists
 */
 
void CErrAlreadyExistsPage::CreateControls() {    
    CErrAlreadyExistsPage* itemWizardPage96 = this;

    wxBoxSizer* itemBoxSizer97 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage96->SetSizer(itemBoxSizer97);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create(itemWizardPage96, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer97->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer97->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDirectionsStaticCtrl = new wxStaticText;
    m_pDirectionsStaticCtrl->Create(itemWizardPage96, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer97->Add(m_pDirectionsStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CErrAlreadyExistsPage::GetPrev() const {
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */
 
wxWizardPage* CErrAlreadyExistsPage::GetNext() const {
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CErrAlreadyExistsPage::ShowToolTips() {
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrAlreadyExistsPage::GetBitmapResource(const wxString& WXUNUSED(name)) {
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CErrAlreadyExistsPage::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}
 
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRACCOUNTALREADYEXISTSPAGE
 */

void CErrAlreadyExistsPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

    if (((CBOINCBaseWizard*)GetParent())->GetProjectConfig()->uses_username) {
        m_pTitleStaticCtrl->SetLabel(_("Username already in use"));
        m_pDirectionsStaticCtrl->SetLabel(_("An account with that username already exists and has a\ndifferent password than the one you entered.\n\nPlease visit the project's web site and follow the instructions there."));
    } else {
        m_pTitleStaticCtrl->SetLabel(_("Email address already in use"));
        m_pDirectionsStaticCtrl->SetLabel(_("An account with that email address already exists and has a\ndifferent password than the one you entered.\n\nPlease visit the project's web site and follow the instructions there."));
    }

    Fit();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRACCOUNTALREADYEXISTSPAGE
 */

void CErrAlreadyExistsPage::OnCancel(wxWizardEvent& event) {
    PROCESS_CANCELEVENT(event);
}
