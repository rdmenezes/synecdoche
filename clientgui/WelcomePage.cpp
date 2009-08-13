// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
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

#include "WelcomePage.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"

IMPLEMENT_DYNAMIC_CLASS(CWelcomePage, wxWizardPage)

BEGIN_EVENT_TABLE(CWelcomePage, wxWizardPage)
    EVT_WIZARD_PAGE_CHANGED(-1, CWelcomePage::OnPageChanged)
    EVT_WIZARD_CANCEL(-1, CWelcomePage::OnCancel)
    EVT_SET_FOCUS(CWelcomePage::OnSetFocus)
    EVT_SHOW(CWelcomePage::OnShow)
END_EVENT_TABLE()

/*!
 * CWelcomePage constructors
 */

CWelcomePage::CWelcomePage() {
}

CWelcomePage::CWelcomePage(CBOINCBaseWizard* parent) {
    Create(parent);
}

/*!
 * WizardPage creator
 */

bool CWelcomePage::Create(CBOINCBaseWizard* parent) {
    m_pTitleStaticCtrl = NULL;
    m_pDescriptionStaticCtrl = NULL;
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

void CWelcomePage::CreateControls() {    
    CWelcomePage* itemWizardPage2 = this;

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage2->SetSizer(itemBoxSizer3);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create(itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(355,24), 0);
    m_pTitleStaticCtrl->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer3->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDescriptionStaticCtrl = new wxStaticText;
    m_pDescriptionStaticCtrl->Create(itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer3->Add(m_pDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDirectionsStaticCtrl = new wxStaticText;
    m_pDirectionsStaticCtrl->Create(itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer3->Add(m_pDirectionsStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemWizardPage2->SetSizer(itemBoxSizer3);
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CWelcomePage::GetPrev() const {
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CWelcomePage::GetNext() const {
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (CheckWizardTypeByPage<CWizardAttachProject>(this)) {
        return PAGE_TRANSITION_NEXT(ID_PROJECTINFOPAGE);
    } else if (CheckWizardTypeByPage<CWizardAccountManager>(this)) {
        CWizardAccountManager* wiz = dynamic_cast<CWizardAccountManager*>(GetParent());
        if (wiz->IsUpdateWizard()) {
            return PAGE_TRANSITION_NEXT(ID_ACCOUNTMANAGERPROCESSINGPAGE);
        } else {
            return PAGE_TRANSITION_NEXT(ID_ACCOUNTMANAGERINFOPAGE);
        }
    }
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CWelcomePage::ShowToolTips() {
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CWelcomePage::GetBitmapResource(const wxString& WXUNUSED(name)) {
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CWelcomePage::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_WELCOMEPAGE
 */

void CWelcomePage::OnPageChanged(wxWizardEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CWelcomePage::OnPageChanged - Function Begin"));
    if (event.GetDirection() == false) return;

    CMainDocument*         pDoc = wxGetApp().GetDocument();
    ACCT_MGR_INFO          ami;
    bool                   is_acct_mgr_detected = false;
    wxString               strBuffer = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDescriptionStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

    if (CheckWizardTypeByPage<CWizardAttachProject>(this)) {
        pDoc->rpc.acct_mgr_info(ami);
        is_acct_mgr_detected = !ami.acct_mgr_url.empty();

        if (is_acct_mgr_detected) {
            m_pTitleStaticCtrl->SetLabel(_("Attach to project"));

            strBuffer.Printf(
                _("If possible, add projects at the\n"
                  "%s web site.\n"
                  "\n"
                  "Projects added via this wizard will not be\n"
                  "listed on or managed via %s."), 
                wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str(),
                wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str()
            );

            m_pDescriptionStaticCtrl->SetLabel(strBuffer);
        } else {
            m_pTitleStaticCtrl->SetLabel(_("Attach to project"));
            m_pDescriptionStaticCtrl->SetLabel(
                _("We'll now guide you through the process of attaching\n"
                  "to a project.")
            );
        }
    } else if (CheckWizardTypeByPage<CWizardAccountManager>(this)) {
        m_pTitleStaticCtrl->SetLabel(_("Account manager"));
        m_pDescriptionStaticCtrl->SetLabel(
            _("We'll now guide you through the process of attaching\n"
              "to an account manager.\n\n"
              "If you want to attach to a single project, click Cancel,\n"
              "then select the 'Attach to project' menu item instead."
            )
        );
    } else {
        wxASSERT(FALSE);
    }

    m_pDirectionsStaticCtrl->SetLabel(_("To continue, click Next."));

    Fit();
    wxLogTrace(wxT("Function Start/End"), wxT("CWelcomePage::OnPageChanged - Function End"));
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_WELCOMEPAGE
 */

void CWelcomePage::OnCancel(wxWizardEvent& event) {
    PROCESS_CANCELEVENT(event);
}


/*!
 * wxEVT_SET_FOCUS event handler for ID_WELCOMEPAGE
 */

void CWelcomePage::OnSetFocus(wxFocusEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CWelcomePage::OnSetFocus - Function Begin"));
    event.Skip();    
    wxLogTrace(wxT("Function Start/End"), wxT("CWelcomePage::OnSetFocus - Function End"));
}

/*!
 * wxEVT_SHOW event handler for ID_WELCOMEPAGE
 */

void CWelcomePage::OnShow(wxShowEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CWelcomePage::OnShow - Function Begin"));
    event.Skip();    
    wxLogTrace(wxT("Function Start/End"), wxT("CWelcomePage::OnShow - Function End"));
}
