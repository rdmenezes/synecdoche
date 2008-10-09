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
//#define __WIZ_DEBUG__

#include <wx/wizard.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/log.h>
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"
#include "WelcomePage.h"
 
IMPLEMENT_DYNAMIC_CLASS(CWelcomePage, wxWizardPage)
 
BEGIN_EVENT_TABLE(CWelcomePage, wxWizardPage)
    EVT_WIZARD_PAGE_CHANGED(-1, CWelcomePage::OnPageChanged)
    EVT_WIZARD_PAGE_CHANGING(-1, CWelcomePage::OnPageChanging)
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
#if defined(__WIZ_DEBUG__)
    m_pErrDescriptionCtrl = NULL;
    m_pErrProjectPropertiesCtrl = NULL;
    m_pErrProjectCommCtrl = NULL;
    m_pErrProjectPropertiesURLCtrl = NULL;
    m_pErrAccountCreationDisabledCtrl = NULL;
    m_pErrClientAccountCreationDisabledCtrl = NULL;
    m_pErrAccountAlreadyExistsCtrl = NULL;
    m_pErrProjectAlreadyAttachedCtrl = NULL;
    m_pErrProjectAttachFailureCtrl = NULL;
    m_pErrGoogleCommCtrl = NULL;
    m_pErrNetDetectionCtrl = NULL;
#endif
 
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

#if defined(__WIZ_DEBUG__)
    m_pErrDescriptionCtrl = new wxStaticBox(itemWizardPage2, wxID_ANY, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(m_pErrDescriptionCtrl, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer7, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(-1, 2, 0, 0);
    itemFlexGridSizer8->AddGrowableCol(0);
    itemFlexGridSizer8->AddGrowableCol(1);
    itemStaticBoxSizer7->Add(itemFlexGridSizer8, 0, wxGROW|wxALL, 5);

    m_pErrProjectPropertiesCtrl = new wxCheckBox;
    m_pErrProjectPropertiesCtrl->Create(itemWizardPage2, ID_ERRPROJECTPROPERTIES, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_pErrProjectPropertiesCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_pErrProjectPropertiesCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pErrProjectCommCtrl = new wxCheckBox;
    m_pErrProjectCommCtrl->Create(itemWizardPage2, ID_ERRPROJECTCOMM, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_pErrProjectCommCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_pErrProjectCommCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pErrProjectPropertiesURLCtrl = new wxCheckBox;
    m_pErrProjectPropertiesURLCtrl->Create(itemWizardPage2, ID_ERRPROJECTPROPERTIESURL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_pErrProjectPropertiesURLCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_pErrProjectPropertiesURLCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pErrAccountCreationDisabledCtrl = new wxCheckBox;
    m_pErrAccountCreationDisabledCtrl->Create(itemWizardPage2, ID_ERRACCOUNTCREATIONDISABLED, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_pErrAccountCreationDisabledCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_pErrAccountCreationDisabledCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pErrClientAccountCreationDisabledCtrl = new wxCheckBox;
    m_pErrClientAccountCreationDisabledCtrl->Create(itemWizardPage2, ID_ERRCLIENTACCOUNTCREATIONDISABLED, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_pErrClientAccountCreationDisabledCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_pErrClientAccountCreationDisabledCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pErrAccountAlreadyExistsCtrl = new wxCheckBox;
    m_pErrAccountAlreadyExistsCtrl->Create(itemWizardPage2, ID_ERRACCOUNTALREADYEXISTS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_pErrAccountAlreadyExistsCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_pErrAccountAlreadyExistsCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pErrProjectAlreadyAttachedCtrl = new wxCheckBox;
    m_pErrProjectAlreadyAttachedCtrl->Create(itemWizardPage2, ID_ERRPROJECTALREADYATTACHED, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_pErrProjectAlreadyAttachedCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_pErrProjectAlreadyAttachedCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pErrProjectAttachFailureCtrl = new wxCheckBox;
    m_pErrProjectAttachFailureCtrl->Create(itemWizardPage2, ID_ERRPROJECTATTACHFAILURE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_pErrProjectAttachFailureCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_pErrProjectAttachFailureCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pErrGoogleCommCtrl = new wxCheckBox;
    m_pErrGoogleCommCtrl->Create(itemWizardPage2, ID_ERRGOOGLECOMM, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_pErrGoogleCommCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_pErrGoogleCommCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer8->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pErrNetDetectionCtrl = new wxCheckBox;
    m_pErrNetDetectionCtrl->Create(itemWizardPage2, ID_ERRNETDETECTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_pErrNetDetectionCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_pErrNetDetectionCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);
#endif

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
        if ((wiz->IsUpdateWizard()) || (wiz->IsRemoveWizard())) {
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
    CWizardAccountManager* pWAM = ((CWizardAccountManager*)GetParent());
    ACCT_MGR_INFO          ami;
    bool                   is_acct_mgr_detected = false;
    wxString               strBuffer = wxEmptyString;


    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDescriptionStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);
#if defined(__WIZ_DEBUG__)
    wxASSERT(m_pErrDescriptionCtrl);
    wxASSERT(m_pErrProjectPropertiesCtrl);
    wxASSERT(m_pErrProjectCommCtrl);
    wxASSERT(m_pErrProjectPropertiesURLCtrl);
    wxASSERT(m_pErrAccountCreationDisabledCtrl);
    wxASSERT(m_pErrClientAccountCreationDisabledCtrl);
    wxASSERT(m_pErrAccountAlreadyExistsCtrl);
    wxASSERT(m_pErrProjectAlreadyAttachedCtrl);
    wxASSERT(m_pErrProjectAttachFailureCtrl);
    wxASSERT(m_pErrGoogleCommCtrl);
    wxASSERT(m_pErrNetDetectionCtrl);
#endif

    if (CheckWizardTypeByPage<CWizardAttachProject>(this)) {
        pDoc->rpc.acct_mgr_info(ami);
        is_acct_mgr_detected = ami.acct_mgr_url.size() ? true : false;

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
        if (pWAM->IsRemoveWizard()) {
            strBuffer.Printf(_("&Stop using%s"), pWAM->GetProjectName().c_str());
            m_pTitleStaticCtrl->SetLabel(strBuffer);
            strBuffer.Printf(
                _("We'll now remove this computer from %s.  From now on,\n"
                  "attach and detach projects directly from this computer.\n"
                  ),
                pWAM->GetProjectName().c_str()
            );
            m_pDescriptionStaticCtrl->SetLabel(strBuffer);
        } else {
            m_pTitleStaticCtrl->SetLabel(_("Account manager"));
            m_pDescriptionStaticCtrl->SetLabel(
                _("We'll now guide you through the process of attaching\n"
                  "to an account manager.\n\n"
			      "If you want to attach to a single project, click Cancel,\n"
			      "then select the 'Attach to project' menu item instead."
			    )
            );
        }
    } else {
        wxASSERT(FALSE);
    }

#if defined(__WIZ_DEBUG__)
    m_pErrDescriptionCtrl->SetLabel(_("Debug Flags"));
    m_pErrProjectPropertiesCtrl->SetLabel(_("Project Properties Failure"));
    m_pErrProjectCommCtrl->SetLabel(_("Project Communication Failure"));
    m_pErrProjectPropertiesURLCtrl->SetLabel(_("Project Properties URL Failure"));
    m_pErrAccountCreationDisabledCtrl->SetLabel(_("Account Creation Disabled"));
    m_pErrClientAccountCreationDisabledCtrl->SetLabel(_("Client Account Creation Disabled"));
    m_pErrAccountAlreadyExistsCtrl->SetLabel(_("Account Already Exists"));
    m_pErrProjectAlreadyAttachedCtrl->SetLabel(_("Project Already Attached"));
    m_pErrProjectAttachFailureCtrl->SetLabel(_("Project Attach Failure"));
    m_pErrGoogleCommCtrl->SetLabel(_("Failure Communicating with Reference Site"));
    m_pErrNetDetectionCtrl->SetLabel(_("Net Detection Failure"));
#endif

    m_pDirectionsStaticCtrl->SetLabel(_("To continue, click Next."));

    Fit();
    wxLogTrace(wxT("Function Start/End"), wxT("CWelcomePage::OnPageChanged - Function End"));
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WELCOMEPAGE
 */

void CWelcomePage::OnPageChanging(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;
 
    unsigned long ulFlags = 0;
 
#if defined(__WIZ_DEBUG__)
    if (m_pErrProjectPropertiesCtrl->GetValue()) {
        ulFlags |= WIZDEBUG_ERRPROJECTPROPERTIES;
    }
    if (m_pErrProjectPropertiesURLCtrl->GetValue()) {
        ulFlags |= WIZDEBUG_ERRPROJECTPROPERTIESURL;
    }
    if (m_pErrProjectCommCtrl->GetValue()) {
        ulFlags |= WIZDEBUG_ERRPROJECTCOMM;
    }
    if (m_pErrGoogleCommCtrl->GetValue()) {
        ulFlags |= WIZDEBUG_ERRGOOGLECOMM;
    }
    if (m_pErrAccountAlreadyExistsCtrl->GetValue()) {
        ulFlags |= WIZDEBUG_ERRACCOUNTALREADYEXISTS;
    }
    if (m_pErrAccountCreationDisabledCtrl->GetValue()) {
        ulFlags |= WIZDEBUG_ERRACCOUNTCREATIONDISABLED;
    }
    if (m_pErrClientAccountCreationDisabledCtrl->GetValue()) {
        ulFlags |= WIZDEBUG_ERRCLIENTACCOUNTCREATIONDISABLED;
    }
    if (m_pErrProjectAttachFailureCtrl->GetValue()) {
        ulFlags |= WIZDEBUG_ERRPROJECTATTACH;
    }
    if (m_pErrProjectAlreadyAttachedCtrl->GetValue()) {
        ulFlags |= WIZDEBUG_ERRPROJECTALREADYATTACHED;
    }
    if (m_pErrNetDetectionCtrl->GetValue()) {
        ulFlags |= WIZDEBUG_ERRNETDETECTION;
    }
#endif
 
    PROCESS_DEBUG_FLAG(ulFlags);
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