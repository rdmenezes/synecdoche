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

#include <wx/wizard.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "CompletionErrorPage.h"

IMPLEMENT_DYNAMIC_CLASS(CCompletionErrorPage, wxWizardPage)

BEGIN_EVENT_TABLE(CCompletionErrorPage, wxWizardPage)
    EVT_WIZARD_PAGE_CHANGED(-1, CCompletionErrorPage::OnPageChanged)
    EVT_WIZARD_CANCEL(-1, CCompletionErrorPage::OnCancel)
END_EVENT_TABLE()

/*!
 * CCompletionErrorPage constructors
 */
 
CCompletionErrorPage::CCompletionErrorPage() {
}

CCompletionErrorPage::CCompletionErrorPage(CBOINCBaseWizard* parent) {
    Create(parent);
}

/*!
 * CAccountResultPage creator
 */
 
bool CCompletionErrorPage::Create(CBOINCBaseWizard* parent) {
    m_pTitleStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;
    m_pServerMessagesDescriptionCtrl = NULL;
    m_pServerMessagesStaticBoxSizerCtrl = NULL;
    m_pServerMessagesCtrl = NULL;

    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create(parent, wizardBitmap);

    CreateControls();
    GetSizer()->Fit(this);
    return TRUE;
}
 
/*!
 * Control creation for CAccountResultPage
 */
 
void CCompletionErrorPage::CreateControls() {    
    CCompletionErrorPage* itemWizardPage85 = this;

    wxBoxSizer* itemBoxSizer86 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage85->SetSizer(itemBoxSizer86);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create(itemWizardPage85, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_pTitleStaticCtrl->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer86->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer86->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDirectionsStaticCtrl = new wxStaticText;
    m_pDirectionsStaticCtrl->Create(itemWizardPage85, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer86->Add(m_pDirectionsStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer86->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pServerMessagesDescriptionCtrl = new wxStaticBox(itemWizardPage85, wxID_ANY, wxEmptyString);
    m_pServerMessagesStaticBoxSizerCtrl = new wxStaticBoxSizer(m_pServerMessagesDescriptionCtrl, wxVERTICAL);
    itemBoxSizer86->Add(m_pServerMessagesStaticBoxSizerCtrl, 0, wxGROW|wxALL, 5);

    m_pServerMessagesCtrl = new wxStaticText;
    m_pServerMessagesCtrl->Create(itemWizardPage85, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_pServerMessagesStaticBoxSizerCtrl->Add(m_pServerMessagesCtrl, 0, wxGROW|wxALL, 5);
}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CCompletionErrorPage::GetPrev() const {
    return NULL;
}

/*!
 * Gets the next page.
 */
 
wxWizardPage* CCompletionErrorPage::GetNext() const {
    return NULL;
}

/*!
 * Should we show tooltips?
 */
 
bool CCompletionErrorPage::ShowToolTips() {
    return TRUE;
}

/// Get the current error message.
///
/// \return The error message currently displayed on this page.
wxString CCompletionErrorPage::GetErrorMessage() const {
    return m_pServerMessagesCtrl->GetLabel();
}

/// Set the current error message.
///
/// \param[in] msg The error message that should be displayed on this page.
void CCompletionErrorPage::SetErrorMessage(const wxString& msg) {
    m_pServerMessagesCtrl->SetLabel(msg);
}

/*!
 * Get bitmap resources
 */
 
wxBitmap CCompletionErrorPage::GetBitmapResource(const wxString& WXUNUSED(name)) {
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */
 
wxIcon CCompletionErrorPage::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONERRORPAGE
 */
 
void CCompletionErrorPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);
    wxASSERT(m_pServerMessagesDescriptionCtrl);
    wxASSERT(m_pServerMessagesStaticBoxSizerCtrl);
    wxASSERT(m_pServerMessagesCtrl);

    if (CheckWizardTypeByPage<CWizardAttachProject>(this)) {
        m_pTitleStaticCtrl->SetLabel(_("Failed to attach to project"));
    } else if (CheckWizardTypeByPage<CWizardAccountManager>(this)) {
        CWizardAccountManager* wiz = dynamic_cast<CWizardAccountManager*>(GetParent());
        if (wiz->IsUpdateWizard()) {
            m_pTitleStaticCtrl->SetLabel(_("Failed to update account manager"));
        } else if (wiz->IsRemoveWizard()) {
            m_pTitleStaticCtrl->SetLabel(_("Failed to remove account manager"));
        } else {
            m_pTitleStaticCtrl->SetLabel(_("Failed to attach to account manager"));
        }
    } else {
        wxASSERT(FALSE);
    }

    if (m_pServerMessagesCtrl->GetLabel().IsEmpty()) {
        m_pDirectionsStaticCtrl->SetLabel(_("An error has occurred;\ncheck Messages for details.\n\nClick Finish to close."));
    } else {
        m_pDirectionsStaticCtrl->SetLabel(_("Click Finish to close."));
    }

    if (CHECK_CLOSINGINPROGRESS() || m_pServerMessagesCtrl->GetLabel().IsEmpty()) {
        m_pServerMessagesDescriptionCtrl->Hide();
        m_pServerMessagesCtrl->Hide();
    } else {
        m_pServerMessagesDescriptionCtrl->SetLabel(_("Messages from server:"));
        m_pServerMessagesDescriptionCtrl->Show();
        m_pServerMessagesCtrl->Show();
    }

    Fit();
}
 
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONERRORPAGE
 */
 
void CCompletionErrorPage::OnCancel(wxWizardEvent& event) {
    PROCESS_CANCELEVENT(event);
}
