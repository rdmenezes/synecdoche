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

#include "AccountManagerProcessingPage.h"

#include "stdwx.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAccountManager.h"
#include "AccountManagerInfoPage.h"
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

/// CAccountManagerProcessingPage states
enum {
    ATTACHACCTMGR_INIT,
    ATTACHACCTMGR_ATTACHACCTMGR_BEGIN,
    ATTACHACCTMGR_ATTACHACCTMGR_EXECUTE,
    ATTACHACCTMGR_CLEANUP,
    ATTACHACCTMGR_END
};

DEFINE_EVENT_TYPE(wxEVT_ACCOUNTMANAGERPROCESSING_STATECHANGE)

IMPLEMENT_DYNAMIC_CLASS(CAccountManagerProcessingPage, wxWizardPage)

BEGIN_EVENT_TABLE(CAccountManagerProcessingPage, wxWizardPage)
    EVT_ACCOUNTMANAGERPROCESSING_STATECHANGE(CAccountManagerProcessingPage::OnStateChange)
    EVT_WIZARD_PAGE_CHANGED(-1, CAccountManagerProcessingPage::OnPageChanged)
    EVT_WIZARD_CANCEL(-1, CAccountManagerProcessingPage::OnCancel)
END_EVENT_TABLE()

/*!
 * CAccountManagerProcessingPage constructors
 */

CAccountManagerProcessingPage::CAccountManagerProcessingPage() {
}

CAccountManagerProcessingPage::CAccountManagerProcessingPage(CBOINCBaseWizard* parent) {
    Create(parent);
}

/*!
 * CAttachProjectPage creator
 */

bool CAccountManagerProcessingPage::Create(CBOINCBaseWizard* parent) {
    m_pTitleStaticCtrl = NULL;
    m_pPleaseWaitStaticCtrl = NULL;
    m_pProgressIndicator = NULL;

    m_bProjectCommunitcationsSucceeded = false;
    m_bProjectUnavailable = false;
    m_bProjectAccountNotFound = false;
    m_bProjectAccountAlreadyExists = false;
    m_iBitmapIndex = 0;
    m_iCurrentState = ATTACHACCTMGR_INIT;

    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create(parent, wizardBitmap);

    CreateControls();
    GetSizer()->Fit(this);
    return TRUE;
}

/*!
 * Control creation for CAttachProjectPage
 */

void CAccountManagerProcessingPage::CreateControls() {    
    CAccountManagerProcessingPage* itemWizardPage36 = this;

    wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage36->SetSizer(itemBoxSizer37);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create(itemWizardPage36, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer37->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pPleaseWaitStaticCtrl = new wxStaticText;
    m_pPleaseWaitStaticCtrl->Create(itemWizardPage36, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer37->Add(m_pPleaseWaitStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

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
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ATTACHPROJECTPAGE
 */

void CAccountManagerProcessingPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;
 
    CWizardAccountManager* pWAM = ((CWizardAccountManager*)GetParent());

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pPleaseWaitStaticCtrl);
    wxASSERT(m_pProgressIndicator);
    wxASSERT(pWAM);

    if (!pWAM->GetProjectName().IsEmpty()) {
        wxString str;

        // %s is the project name
        //    i.e. 'BOINC', 'GridRepublic'
        str.Printf(_("Communicating with %s."), pWAM->GetProjectName().c_str());

        m_pTitleStaticCtrl->SetLabel(str);
    } else {
        m_pTitleStaticCtrl->SetLabel(_("Communicating with server."));
    }

    m_pPleaseWaitStaticCtrl->SetLabel(_("Please wait..."));

    SetProjectCommunitcationsSucceeded(false);
    SetProjectUnavailable(false);
    SetProjectAccountAlreadyExists(false);
    SetNextState(ATTACHACCTMGR_INIT);
 
    CAccountManagerProcessingPageEvent TransitionEvent(wxEVT_ACCOUNTMANAGERPROCESSING_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);

    Fit();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ATTACHPROJECTPAGE
 */

void CAccountManagerProcessingPage::OnCancel(wxWizardEvent& event) {
    PROCESS_CANCELEVENT(event);
}

/*!
 * wxEVT_ACCOUNTCREATION_STATECHANGE event handler for ID_ACCOUNTCREATIONPAGE
 */
 
void CAccountManagerProcessingPage::OnStateChange(CAccountManagerProcessingPageEvent& WXUNUSED(event)) {
    CMainDocument* pDoc         = wxGetApp().GetDocument();
    CWizardAccountManager* pWAM = ((CWizardAccountManager*)GetParent());
    wxDateTime dtStartExecutionTime;
    wxDateTime dtCurrentExecutionTime;
    wxTimeSpan tsExecutionTime;
    ACCT_MGR_RPC_REPLY reply;
    wxString strBuffer = wxEmptyString;
    std::string url = "";
    std::string username = "";
    std::string password = "";
    bool bPostNewEvent = true;
    int iReturnValue = 0;
    unsigned int i;
 
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
 
    switch(GetCurrentState()) {
        case ATTACHACCTMGR_INIT:
            pWAM->DisableNextButton();
            pWAM->DisableBackButton();

            StartProgress(m_pProgressIndicator);
            SetNextState(ATTACHACCTMGR_ATTACHACCTMGR_BEGIN);
            break;
        case ATTACHACCTMGR_ATTACHACCTMGR_BEGIN:
            SetNextState(ATTACHACCTMGR_ATTACHACCTMGR_EXECUTE);
            break;
        case ATTACHACCTMGR_ATTACHACCTMGR_EXECUTE:
            // Attempt to attach to the accout manager.
            
            // Newer versions of the server-side software contain the correct
            //   master url in the get_project_config response.  If it is available
            //   use it instead of what the user typed in.
            if (!pWAM->GetProjectConfig()->master_url.empty()) {
                url = pWAM->GetProjectConfig()->master_url;
            } else {
                url = (const char*)pWAM->GetAccountManagerInfoPage()->GetProjectURL().mb_str();
            }
            username = (const char*)pWAM->GetAccountInfoPage()->GetAccountEmailAddress().mb_str();
            password = (const char*)pWAM->GetAccountInfoPage()->GetAccountPassword().mb_str();
    
            // Wait until we are done processing the request.
            dtStartExecutionTime = wxDateTime::Now();
            dtCurrentExecutionTime = wxDateTime::Now();
            tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
            iReturnValue = 0;
            reply.error_num = ERR_RETRY;
            while ((!iReturnValue) && (tsExecutionTime.GetSeconds() <= 60)
                        && (!CHECK_CLOSINGINPROGRESS())
                        && ((ERR_IN_PROGRESS == reply.error_num)
                            || (ERR_RETRY == reply.error_num))) {
                if (ERR_RETRY == reply.error_num) {
                    pDoc->rpc.acct_mgr_rpc(url.c_str(), username.c_str(), password.c_str(),
                                            pWAM->GetCredentialsCached());
                }
                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                iReturnValue = pDoc->rpc.acct_mgr_rpc_poll(reply);

                IncrementProgress(m_pProgressIndicator);

                ::wxMilliSleep(500);
                ::wxSafeYield(GetParent());
            }
    
            if (!iReturnValue && !reply.error_num) {
                SetProjectAttachSucceeded(true);
            } else {
                SetProjectAttachSucceeded(false);

                if ((ERR_NOT_FOUND == reply.error_num) ||
                    (ERR_DB_NOT_FOUND == reply.error_num) ||
                    (ERR_BAD_EMAIL_ADDR == reply.error_num) ||
                    (ERR_BAD_PASSWD == reply.error_num)) {

                    // For any logon error, make sure we do not attempt to use cached credentials
                    //   on any follow-ups.
                    pWAM->SetCredentialsCached(false);
                    SetProjectAccountNotFound(true);
                } else {
                    SetProjectAccountNotFound(false);
                }

                strBuffer = pWAM->GetCompletionErrorPage()->GetErrorMessage();
                if (HTTP_STATUS_INTERNAL_SERVER_ERROR == reply.error_num) {
                    strBuffer += _("An internal server error has occurred.\n");
                } else {
                    for (i=0; i<reply.messages.size(); i++) {
                        strBuffer += wxString(reply.messages[i].c_str(), wxConvUTF8) + wxString(wxT("\n"));
                    }
                }
                pWAM->GetCompletionErrorPage()->SetErrorMessage(strBuffer);
            }
            SetNextState(ATTACHACCTMGR_CLEANUP);
            break;
        case ATTACHACCTMGR_CLEANUP:
            FinishProgress(m_pProgressIndicator);
            SetNextState(ATTACHACCTMGR_END);
            break;
        default:
            // Allow a glimps of what the result was before advancing to the next page.
            wxSleep(1);
            pWAM->EnableNextButton();
            pWAM->EnableBackButton();
            pWAM->SimulateNextButton();
            bPostNewEvent = false;
            break;
    }
 
    Update();
 
    if (bPostNewEvent && !CHECK_CLOSINGINPROGRESS()) {
        CAccountManagerProcessingPageEvent TransitionEvent(wxEVT_ACCOUNTMANAGERPROCESSING_STATECHANGE, this);
        AddPendingEvent(TransitionEvent);
    }
}
  
/*!
 * Gets the previous page.
 */

wxWizardPage* CAccountManagerProcessingPage::GetPrev() const {
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAccountManagerProcessingPage::GetNext() const {
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (GetProjectAttachSucceeded()) {
        // We were successful in creating or retrieving an account
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONPAGE);
    } else if (!GetProjectCommunitcationsSucceeded() && GetProjectAccountNotFound()) {
        // The requested account does not exist or the password is bad
        return PAGE_TRANSITION_NEXT(ID_ERRNOTFOUNDPAGE);
    } else {
        // The project much be down for maintenance
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } 
}

/*!
 * Should we show tooltips?
 */

bool CAccountManagerProcessingPage::ShowToolTips() {
    return TRUE;
}

void CAccountManagerProcessingPage::StartProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 1;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress01.xpm")));
}
 
void CAccountManagerProcessingPage::IncrementProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex += 1;
    if (12 < m_iBitmapIndex) m_iBitmapIndex = 1;
 
    wxString str;
    str.Printf(wxT("res/wizprogress%02d.xpm"), m_iBitmapIndex);
 
    pBitmap->SetBitmap(GetBitmapResource(str));
    Update();
}
 
void CAccountManagerProcessingPage::FinishProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 12;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress12.xpm")));
}

/*!
 * Get bitmap resources
 */

wxBitmap CAccountManagerProcessingPage::GetBitmapResource(const wxString& name) {
    if (name == wxT("res/wizprogress01.xpm")) {
        wxBitmap bitmap(wizprogress01_xpm);
        return bitmap;
    } else if (name == wxT("res/wizprogress02.xpm")) {
        wxBitmap bitmap(wizprogress02_xpm);
        return bitmap;
    } else if (name == wxT("res/wizprogress03.xpm")) {
        wxBitmap bitmap(wizprogress03_xpm);
        return bitmap;
    } else if (name == wxT("res/wizprogress04.xpm")) {
        wxBitmap bitmap(wizprogress04_xpm);
        return bitmap;
    } else if (name == wxT("res/wizprogress05.xpm")) {
        wxBitmap bitmap(wizprogress05_xpm);
        return bitmap;
    } else if (name == wxT("res/wizprogress06.xpm")) {
        wxBitmap bitmap(wizprogress06_xpm);
        return bitmap;
    } else if (name == wxT("res/wizprogress07.xpm")) {
        wxBitmap bitmap(wizprogress07_xpm);
        return bitmap;
    } else if (name == wxT("res/wizprogress08.xpm")) {
        wxBitmap bitmap(wizprogress08_xpm);
        return bitmap;
    } else if (name == wxT("res/wizprogress09.xpm")) {
        wxBitmap bitmap(wizprogress09_xpm);
        return bitmap;
    } else if (name == wxT("res/wizprogress10.xpm")) {
        wxBitmap bitmap(wizprogress10_xpm);
        return bitmap;
    } else if (name == wxT("res/wizprogress11.xpm")) {
        wxBitmap bitmap(wizprogress11_xpm);
        return bitmap;
    } else if (name == wxT("res/wizprogress12.xpm")) {
        wxBitmap bitmap(wizprogress12_xpm);
        return bitmap;
    }
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CAccountManagerProcessingPage::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}
