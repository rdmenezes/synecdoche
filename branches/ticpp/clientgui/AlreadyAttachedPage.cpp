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

#include "AlreadyAttachedPage.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"

IMPLEMENT_DYNAMIC_CLASS(CErrAlreadyAttachedPage, wxWizardPage)

BEGIN_EVENT_TABLE(CErrAlreadyAttachedPage, wxWizardPage)
    EVT_WIZARD_PAGE_CHANGED(-1, CErrAlreadyAttachedPage::OnPageChanged)
    EVT_WIZARD_CANCEL(-1, CErrAlreadyAttachedPage::OnCancel)
END_EVENT_TABLE()

/*!
 * CErrAlreadyAttachedPage constructors
 */

CErrAlreadyAttachedPage::CErrAlreadyAttachedPage() {
}

CErrAlreadyAttachedPage::CErrAlreadyAttachedPage(CBOINCBaseWizard* parent) {
    Create(parent);
}

/*!
 * WizardPage creator
 */

bool CErrAlreadyAttachedPage::Create(CBOINCBaseWizard* parent) {
    m_pTitleStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;

    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create(parent, wizardBitmap);

    CreateControls();
    GetSizer()->Fit(this);
    return TRUE;
}

/*!
 * Control creation for WizardPage
 */

void CErrAlreadyAttachedPage::CreateControls() {    
    CErrAlreadyAttachedPage* itemWizardPage96 = this;

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

wxWizardPage* CErrAlreadyAttachedPage::GetPrev() const {
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CErrAlreadyAttachedPage::GetNext() const {
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CErrAlreadyAttachedPage::ShowToolTips() {
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrAlreadyAttachedPage::GetBitmapResource(const wxString& WXUNUSED(name)) {
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CErrAlreadyAttachedPage::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTALREADYATTACHED
 */

void CErrAlreadyAttachedPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

    m_pTitleStaticCtrl->SetLabel(_("Already attached to project"));
    m_pDirectionsStaticCtrl->SetLabel(_("You are already attached to this project."));

    Fit();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTALREADYATTACHED
 */

void CErrAlreadyAttachedPage::OnCancel(wxWizardEvent& event) {
    PROCESS_CANCELEVENT(event);
}