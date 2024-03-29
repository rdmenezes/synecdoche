// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Peter Kortschack
// Copyright (C) 2009 University of California
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

#include "CompletionPage.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"
#include "AccountInfoPage.h"

IMPLEMENT_DYNAMIC_CLASS(CCompletionPage, wxWizardPage)

BEGIN_EVENT_TABLE(CCompletionPage, wxWizardPage)
    EVT_WIZARD_PAGE_CHANGED(-1, CCompletionPage::OnPageChanged)
    EVT_WIZARD_CANCEL(-1, CCompletionPage::OnCancel)
    EVT_WIZARD_FINISHED(ID_COMPLETIONPAGE, CCompletionPage::OnFinished)
END_EVENT_TABLE()

/*!
 * CCompletionPage constructors
 */
 
CCompletionPage::CCompletionPage() {
}

CCompletionPage::CCompletionPage(CBOINCBaseWizard* parent) {
    Create(parent);
}

/*!
 * CCompletionPage creator
 */
 
bool CCompletionPage::Create(CBOINCBaseWizard* parent) {
    m_pCompletionTitle = NULL;
    m_pCompletionWelcome = NULL;
    m_pCompletionBrandedMessage = NULL;
    m_pCompletionMessage = NULL;

    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create(parent, wizardBitmap);

    CreateControls();
    GetSizer()->Fit(this);
    return TRUE;
}
  
/*!
 * Control creation for CCompletionPage
 */
 
void CCompletionPage::CreateControls()
{    
    CCompletionPage* itemWizardPage79 = this;

    wxBoxSizer* itemBoxSizer80 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage79->SetSizer(itemBoxSizer80);

    m_pCompletionTitle = new wxStaticText;
    m_pCompletionTitle->Create(itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_pCompletionTitle->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Verdana")));
    itemBoxSizer80->Add(m_pCompletionTitle, 0, wxALIGN_LEFT|wxALL, 5);

    m_pCompletionWelcome = new wxStaticText;
    m_pCompletionWelcome->Create(itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_pCompletionWelcome->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE));
    itemBoxSizer80->Add(m_pCompletionWelcome, 0, wxALIGN_LEFT|wxALL, 5);

    m_pCompletionBrandedMessage = new wxStaticText;
    m_pCompletionBrandedMessage->Create(itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer80->Add(m_pCompletionBrandedMessage, 0, wxALIGN_LEFT|wxALL, 5);

    m_pCompletionMessage = new wxStaticText;
    m_pCompletionMessage->Create(itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer80->Add(m_pCompletionMessage, 0, wxALIGN_LEFT|wxALL, 5);
}
  
/*!
 * Gets the previous page.
 */

wxWizardPage* CCompletionPage::GetPrev() const {
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CCompletionPage::GetNext() const {
    return NULL;
}

/*!
 * Should we show tooltips?
 */
 
bool CCompletionPage::ShowToolTips() {
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCompletionPage::GetBitmapResource(const wxString& WXUNUSED(name)) {
    return wxNullBitmap;
}
  
/*!
 * Get icon resources
 */
 
wxIcon CCompletionPage::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONPAGE
 */
 
void CCompletionPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    CWizardAttachProject* pWAP = ((CWizardAttachProject*)GetParent());
    CSkinAdvanced*        pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();

    wxASSERT(pSkinAdvanced);
    wxASSERT(m_pCompletionTitle);
    wxASSERT(m_pCompletionWelcome);
    wxASSERT(m_pCompletionBrandedMessage);
    wxASSERT(m_pCompletionMessage);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    if (CheckWizardTypeByPage<CWizardAttachProject>(this)) {
        m_pCompletionTitle->SetLabel(_("Attached to project"));

        m_pCompletionWelcome->Hide();

        m_pCompletionBrandedMessage->SetLabel(_("You are now successfully attached to this project."));

        if (pWAP->GetAccountInfoPage()->CreateNewAccount()) {
            m_pCompletionMessage->SetLabel(_("When you click Finish, your web browser will go to a page where\nyou can set your account name and preferences."));
        } else {
            m_pCompletionMessage->SetLabel(_("Click Finish to close."));
        }
    } else if (CheckWizardTypeByPage<CWizardAccountManager>(this)) {
        CWizardAccountManager* wiz = dynamic_cast<CWizardAccountManager*>(GetParent());
        if (wiz->IsUpdateWizard()) {
            // Update completed
            wxString strTitle;
            if (pSkinAdvanced->IsBranded()) {
                // %s is the project name
                //    i.e. 'GridRepublic'
                strTitle.Printf(_("Update from %s completed."),
                    pSkinAdvanced->GetApplicationShortName().c_str());
            } else {
                strTitle = _("Update completed.");
            }
            m_pCompletionTitle->SetLabel(strTitle);
            m_pCompletionMessage->SetLabel(_("Click Finish to close."));
        } else {
            // Attach Completed

            wxString strTitle;
            if (pSkinAdvanced->IsBranded()) {
                // %s is the project name
                //    i.e. 'GridRepublic'
                strTitle.Printf(_("Attached to %s"),
                    pSkinAdvanced->GetApplicationShortName().c_str());
            } else {
                strTitle = _("Attached to account manager");
            }

            m_pCompletionTitle->SetLabel(strTitle);

            if (pSkinAdvanced->IsBranded()) {
                // %s is the project name
                //    i.e. 'GridRepublic'
                wxString strWelcome;
                strWelcome.Printf(_("Welcome to %s!"),
                    pSkinAdvanced->GetApplicationShortName().c_str());

                m_pCompletionWelcome->Show();
                m_pCompletionWelcome->SetLabel(strWelcome);
            }

            wxString strBrandedMessage;
            if (pSkinAdvanced->IsBranded()) {
                // 1st %s is the project name
                //    i.e. 'GridRepublic'
                // 2nd %s is the account manager success message
                strBrandedMessage.Printf(_("You are now successfully attached to the %s system."),
                    pSkinAdvanced->GetApplicationShortName().c_str());
            } else {
                strBrandedMessage = _("You are now successfully attached to this account manager.");
            }
            m_pCompletionBrandedMessage->SetLabel(strBrandedMessage);
            m_pCompletionMessage->SetLabel(_("Click Finish to close."));
        }
    }

    Fit();

    // Is this supposed to be completely automated?
    // If so, then go ahead and close the wizard down now.
  if (pWAP->GetCloseWhenCompleted()) {
        pWAP->SimulateNextButton();
    }
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONPAGE
 */
 
void CCompletionPage::OnCancel(wxWizardEvent& event) {
    PROCESS_CANCELEVENT(event);
}
 
/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_COMPLETIONPAGE
 */
 
void CCompletionPage::OnFinished(wxWizardEvent& event) {
    event.Skip();
}
