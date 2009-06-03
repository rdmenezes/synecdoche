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

#include "ProjectProcessingPage.h"

#include "stdwx.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "ProjectInfoPage.h"
#include "AccountKeyPage.h"
#include "AccountInfoPage.h"
#include "CompletionErrorPage.h"

#include "res/wizprogress01.xpm"
#include "res/wizprogress02.xpm"
#include "res/wizprogress03.xpm"
#include "res/wizprogress04.xpm"
#include "res/wizprogress05.xpm"
#include "res/wizprogress06.xpm"
#include "res/wizprogress07.xpm"
#include "res/wizprogress08.xpm"
#include "res/wizprogress09.xpm"
#include "res/wizprogress10.xpm"
#include "res/wizprogress11.xpm"
#include "res/wizprogress12.xpm"

/// CProjectProcessingPage states.
enum {
    ATTACHPROJECT_INIT,
    ATTACHPROJECT_ACCOUNTQUERY_BEGIN,
    ATTACHPROJECT_ACCOUNTQUERY_EXECUTE,
    ATTACHPROJECT_ATTACHPROJECT_BEGIN,
    ATTACHPROJECT_ATTACHPROJECT_EXECUTE,
    ATTACHPROJECT_CLEANUP,
    ATTACHPROJECT_END
};

DEFINE_EVENT_TYPE(wxEVT_PROJECTPROCESSING_STATECHANGE)
  
IMPLEMENT_DYNAMIC_CLASS(CProjectProcessingPage, wxWizardPage)
  
BEGIN_EVENT_TABLE(CProjectProcessingPage, wxWizardPage)
    EVT_PROJECTPROCESSING_STATECHANGE(CProjectProcessingPage::OnStateChange)
    EVT_WIZARD_PAGE_CHANGED(-1, CProjectProcessingPage::OnPageChanged)
    EVT_WIZARD_CANCEL(-1, CProjectProcessingPage::OnCancel) 
END_EVENT_TABLE()
  
/*!
 * CProjectProcessingPage constructors
 */
 
CProjectProcessingPage::CProjectProcessingPage() {
}
  
CProjectProcessingPage::CProjectProcessingPage(CBOINCBaseWizard* parent) {
    Create(parent);
}
  
/*!
 * CProjectPropertiesPage creator
 */
 
bool CProjectProcessingPage::Create(CBOINCBaseWizard* parent) {
    m_pTitleStaticCtrl = NULL;
    m_pProgressIndicator = NULL;
 
    m_bProjectCommunitcationsSucceeded = false;
    m_bProjectUnavailable = false;
    m_bProjectAccountNotFound = false;
    m_bProjectAccountAlreadyExists = false;
    m_iBitmapIndex = 0;
    m_iCurrentState = ATTACHPROJECT_INIT;
 
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create(parent, wizardBitmap);

    CreateControls();
    GetSizer()->Fit(this);
    return TRUE;
}
 
/*!
 * Control creation for CProjectPropertiesPage
 */
 
void CProjectProcessingPage::CreateControls() {    
    CProjectProcessingPage* itemWizardPage36 = this;

    wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage36->SetSizer(itemBoxSizer37);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create(itemWizardPage36, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer37->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer37->Add(5, 80, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer40 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer40->AddGrowableRow(0);
    itemFlexGridSizer40->AddGrowableCol(0);
    itemFlexGridSizer40->AddGrowableCol(1);
    itemFlexGridSizer40->AddGrowableCol(2);
    itemBoxSizer37->Add(itemFlexGridSizer40, 0, wxGROW|wxALL, 5);

    itemFlexGridSizer40->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);

    wxBitmap itemBitmap41(GetBitmapResource(wxT("res/wizprogress01.xpm")));
    m_pProgressIndicator = new wxStaticBitmap;
    m_pProgressIndicator->Create(itemWizardPage36, ID_PROGRESSCTRL, itemBitmap41, wxDefaultPosition, wxSize(184, 48), 0);
    itemFlexGridSizer40->Add(m_pProgressIndicator, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer40->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CProjectProcessingPage::GetPrev() const {
    return PAGE_TRANSITION_BACK;
}
  
/*!
 * Gets the next page.
 */
 
wxWizardPage* CProjectProcessingPage::GetNext() const {
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (GetProjectAttachSucceeded()) {
        // We were successful in creating or retrieving an account
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONPAGE);
    } else if (!GetProjectCommunitcationsSucceeded() && GetProjectAccountAlreadyExists()) {
        // The requested account already exists
        return PAGE_TRANSITION_NEXT(ID_ERRALREADYEXISTSPAGE);
    } else if (!GetProjectCommunitcationsSucceeded() && GetProjectAccountNotFound()) {
        // The requested account does not exist or the password is bad
        return PAGE_TRANSITION_NEXT(ID_ERRNOTFOUNDPAGE);
    } else {
        // Ann error must have occurred
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } 
}
  
/*!
 * Should we show tooltips?
 */
 
bool CProjectProcessingPage::ShowToolTips() {
    return TRUE;
}
 
void CProjectProcessingPage::StartProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 1;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress01.xpm")));
}
 
void CProjectProcessingPage::IncrementProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex += 1;
    if (12 < m_iBitmapIndex) m_iBitmapIndex = 1;
 
    wxString str;
    str.Printf(wxT("res/wizprogress%02d.xpm"), m_iBitmapIndex);
 
    pBitmap->SetBitmap(GetBitmapResource(str));
    Update();
}
 
void CProjectProcessingPage::FinishProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 12;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress12.xpm")));
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CProjectProcessingPage::GetBitmapResource(const wxString& name) {
    if (name == wxT("res/wizprogress01.xpm")) {
        return wxBitmap(wizprogress01_xpm);
    } else if (name == wxT("res/wizprogress02.xpm")) {
        return wxBitmap(wizprogress02_xpm);
    } else if (name == wxT("res/wizprogress03.xpm")) {
        return wxBitmap(wizprogress03_xpm);
    } else if (name == wxT("res/wizprogress04.xpm")) {
        return wxBitmap(wizprogress04_xpm);
    } else if (name == wxT("res/wizprogress05.xpm")) {
        return wxBitmap(wizprogress05_xpm);
    } else if (name == wxT("res/wizprogress06.xpm")) {
        return wxBitmap(wizprogress06_xpm);
    } else if (name == wxT("res/wizprogress07.xpm")) {
        return wxBitmap(wizprogress07_xpm);
    } else if (name == wxT("res/wizprogress08.xpm")) {
        return wxBitmap(wizprogress08_xpm);
    } else if (name == wxT("res/wizprogress09.xpm")) {
        return wxBitmap(wizprogress09_xpm);
    } else if (name == wxT("res/wizprogress10.xpm")) {
        return wxBitmap(wizprogress10_xpm);
    } else if (name == wxT("res/wizprogress11.xpm")) {
        return wxBitmap(wizprogress11_xpm);
    } else if (name == wxT("res/wizprogress12.xpm")) {
        return wxBitmap(wizprogress12_xpm);
    }
    return wxNullBitmap;
}
  
/*!
 * Get icon resources
 */
 
wxIcon CProjectProcessingPage::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ATTACHPROJECTPAGE
 */
 
void CProjectProcessingPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;
 
    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pProgressIndicator);

    m_pTitleStaticCtrl->SetLabel(_("Communicating with project\nPlease wait..."));

    SetProjectCommunitcationsSucceeded(false);
    SetProjectUnavailable(false);
    SetProjectAccountAlreadyExists(false);
    SetNextState(ATTACHPROJECT_INIT);
 
    CProjectProcessingPageEvent TransitionEvent(wxEVT_PROJECTPROCESSING_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);

    Fit();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTCREATIONPAGE
 */
 
void CProjectProcessingPage::OnCancel(wxWizardEvent& event) {
    PROCESS_CANCELEVENT(event);
}
 
/*!
 * wxEVT_ACCOUNTCREATION_STATECHANGE event handler for ID_ACCOUNTCREATIONPAGE
 */
 
void CProjectProcessingPage::OnStateChange(CProjectProcessingPageEvent& WXUNUSED(event)) {
    CMainDocument* pDoc        = wxGetApp().GetDocument();
    CWizardAttachProject* pWAP = ((CWizardAttachProject*)GetParent());
    ACCOUNT_IN* ai             = pWAP->GetAccountIn();
    ACCOUNT_OUT* ao            = pWAP->GetAccountOut();
    unsigned int i;
    PROJECT_ATTACH_REPLY reply;
    wxString strBuffer = wxEmptyString;
    wxDateTime dtStartExecutionTime;
    wxDateTime dtCurrentExecutionTime;
    wxTimeSpan tsExecutionTime;
    bool bPostNewEvent = true;
    int iReturnValue = 0;
    bool creating_account = false;
 
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
 
    switch(GetCurrentState()) {
        case ATTACHPROJECT_INIT:
            pWAP->DisableNextButton();
            pWAP->DisableBackButton();

            StartProgress(m_pProgressIndicator);
            SetNextState(ATTACHPROJECT_ACCOUNTQUERY_BEGIN);
            break;
        case ATTACHPROJECT_ACCOUNTQUERY_BEGIN:
            SetNextState(ATTACHPROJECT_ACCOUNTQUERY_EXECUTE);
            break;
        case ATTACHPROJECT_ACCOUNTQUERY_EXECUTE:
            // Attempt to create the account or reterieve the authenticator.
            ai->clear();
            ao->clear();

            ai->url = (const char*)pWAP->GetProjectInfoPage()->GetProjectURL().mb_str();

            if (!pWAP->GetAccountKeyPage()->GetAccountKey().IsEmpty() || 
                    pWAP->GetCredentialsCached() || pWAP->GetCretentialsDetected()) {
                if (!pWAP->GetCredentialsCached() || pWAP->GetCretentialsDetected()) {
                    ao->authenticator = (const char*)pWAP->GetAccountKeyPage()->GetAccountKey().mb_str();
                }
                SetProjectCommunitcationsSucceeded(true);
            } else {
                // Setup initial values for both the create and lookup API
                CAccountInfoPage* acc_info_page = pWAP->GetAccountInfoPage();
                ai->email_addr = (const char*)acc_info_page->GetAccountEmailAddress().mb_str();
                ai->passwd = (const char*)acc_info_page->GetAccountPassword().mb_str();
                ai->user_name = (const char*)::wxGetUserName().mb_str();
                if (ai->user_name.empty()) {
                    ai->user_name = (const char*)::wxGetUserId().mb_str();
                }

                if (acc_info_page->CreateNewAccount()) {
                    pDoc->rpc.create_account(*ai);
                    creating_account = true;

                    // Wait until we are done processing the request.
                    dtStartExecutionTime = wxDateTime::Now();
                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    ao->error_num = ERR_IN_PROGRESS;
                    while (
                        ERR_IN_PROGRESS == ao->error_num && !iReturnValue &&
                        tsExecutionTime.GetSeconds() <= 60 &&
                        !CHECK_CLOSINGINPROGRESS()
                        )
                    {
                        dtCurrentExecutionTime = wxDateTime::Now();
                        tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                        iReturnValue = pDoc->rpc.create_account_poll(*ao);

                        IncrementProgress(m_pProgressIndicator);

                        ::wxMilliSleep(500);
                        ::wxSafeYield(GetParent());
                    }

                    if ((!iReturnValue) && !ao->error_num && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTCOMM)) {
                        pWAP->SetAccountCreatedSuccessfully(true);
                    }
                } else {
                    pDoc->rpc.lookup_account(*ai);
 
                    // Wait until we are done processing the request.
                    dtStartExecutionTime = wxDateTime::Now();
                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    ao->error_num = ERR_IN_PROGRESS;
                    while (
                        ERR_IN_PROGRESS == ao->error_num && !iReturnValue &&
                        tsExecutionTime.GetSeconds() <= 60 &&
                        !CHECK_CLOSINGINPROGRESS()
                        )
                    {
                        dtCurrentExecutionTime = wxDateTime::Now();
                        tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                        iReturnValue = pDoc->rpc.lookup_account_poll(*ao);

                        IncrementProgress(m_pProgressIndicator);

                        ::wxMilliSleep(500);
                        ::wxSafeYield(GetParent());
                    }
                }
 
                if ((!iReturnValue) && !ao->error_num && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTCOMM)) {
                    SetProjectCommunitcationsSucceeded(true);
                } else {
                    SetProjectCommunitcationsSucceeded(false);

                    if ((ao->error_num == ERR_DB_NOT_UNIQUE)
                        || (ao->error_num == ERR_NONUNIQUE_EMAIL)
                        || (ao->error_num == ERR_BAD_PASSWD && creating_account)
                        || CHECK_DEBUG_FLAG(WIZDEBUG_ERRACCOUNTALREADYEXISTS)
                    ) {
                        SetProjectAccountAlreadyExists(true);
                    } else {
                        SetProjectAccountAlreadyExists(false);
                    }

                    if ((ERR_NOT_FOUND == ao->error_num) ||
                        (ao->error_num == ERR_DB_NOT_FOUND) ||
                        (ERR_BAD_EMAIL_ADDR == ao->error_num) ||
                        (ERR_BAD_PASSWD == ao->error_num) ||
                        CHECK_DEBUG_FLAG(WIZDEBUG_ERRACCOUNTNOTFOUND)) {
                        SetProjectAccountNotFound(true);
                    } else {
                        SetProjectAccountNotFound(false);
                    }

                    strBuffer = pWAP->GetCompletionErrorPage()->GetErrorMessage();
                    if ((HTTP_STATUS_NOT_FOUND == ao->error_num) || CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTPROPERTIESURL)) {
                        strBuffer += 
                            _("Required wizard file(s) are missing from the target server.\n"
                              "(lookup_account.php/create_account.php)\n");
                    } else if ((HTTP_STATUS_INTERNAL_SERVER_ERROR == ao->error_num) || CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTPROPERTIESURL)) {
                        strBuffer += 
                            _("An internal server error has occurred.\n");
                    } else {
                        if (ao->error_msg.size()) {
                            strBuffer += wxString(ao->error_msg.c_str(), wxConvUTF8) + wxString(wxT("\n"));
                        }
                    }
                    pWAP->GetCompletionErrorPage()->SetErrorMessage(strBuffer);
                }
            }
            SetNextState(ATTACHPROJECT_ATTACHPROJECT_BEGIN);
            break;
        case ATTACHPROJECT_ATTACHPROJECT_BEGIN:
            SetNextState(ATTACHPROJECT_ATTACHPROJECT_EXECUTE);
            break;
        case ATTACHPROJECT_ATTACHPROJECT_EXECUTE:
            if (GetProjectCommunitcationsSucceeded()) {
                if (pWAP->GetCredentialsCached()) {
                    pDoc->rpc.project_attach_from_file();
                } else {
                    pDoc->rpc.project_attach(
                        ai->url.c_str(),
                        ao->authenticator.c_str(),
                        pWAP->GetProjectConfig()->name.c_str()
                    );
                }
     
                // Wait until we are done processing the request.
                dtStartExecutionTime = wxDateTime::Now();
                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                iReturnValue = 0;
                reply.error_num = ERR_IN_PROGRESS;
                while ((!iReturnValue && (ERR_IN_PROGRESS == reply.error_num)) &&
                    tsExecutionTime.GetSeconds() <= 60 &&
                    !CHECK_CLOSINGINPROGRESS()
                    )
                {
                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    iReturnValue = pDoc->rpc.project_attach_poll(reply);

                    IncrementProgress(m_pProgressIndicator);

                    ::wxMilliSleep(500);
                    ::wxSafeYield(GetParent());
                }
     
                if (!iReturnValue && !reply.error_num && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTATTACH)) {
                    SetProjectAttachSucceeded(true);
                    pWAP->SetAttachedToProjectSuccessfully(true);
                    pWAP->SetProjectURL(wxString(ai->url.c_str(), wxConvUTF8));
                    pWAP->SetProjectAuthenticator(wxString(ao->authenticator.c_str(), wxConvUTF8));
                } else {
                    SetProjectAttachSucceeded(false);

                    strBuffer = pWAP->GetCompletionErrorPage()->GetErrorMessage();
                    if ((HTTP_STATUS_INTERNAL_SERVER_ERROR == reply.error_num) || CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTPROPERTIESURL)) {
                        strBuffer += 
                            _("An internal server error has occurred.\n");
                    } else {
                        for (i=0; i<reply.messages.size(); i++) {
                            strBuffer += wxString(reply.messages[i].c_str(), wxConvUTF8) + wxString(wxT("\n"));
                        }
                    }
                    pWAP->GetCompletionErrorPage()->SetErrorMessage(wxString(strBuffer, wxConvUTF8));
                }
            } else {
                SetProjectAttachSucceeded(false);
            }
            SetNextState(ATTACHPROJECT_CLEANUP);
            break;
        case ATTACHPROJECT_CLEANUP:
            FinishProgress(m_pProgressIndicator);
            SetNextState(ATTACHPROJECT_END);
            break;
        default:
            // Allow a glimps of what the result was before advancing to the next page.
            wxSleep(1);
            pWAP->EnableNextButton();
            pWAP->EnableBackButton();
            pWAP->SimulateNextButton();
            bPostNewEvent = false;
            break;
    }

    Update();

    if (bPostNewEvent && !CHECK_CLOSINGINPROGRESS()) {
        CProjectProcessingPageEvent TransitionEvent(wxEVT_PROJECTPROCESSING_STATECHANGE, this);
        AddPendingEvent(TransitionEvent);
    }
}
