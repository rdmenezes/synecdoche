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

#include "AccountManagerPropertiesPage.h"

#include "stdwx.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAccountManager.h"
#include "AccountManagerInfoPage.h"

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

/// CAccountManagerPropertiesPage states
enum {
    ACCTMGRPROP_INIT,
    ACCTMGRPROP_RETRPROJECTPROPERTIES_BEGIN,
    ACCTMGRPROP_RETRPROJECTPROPERTIES_EXECUTE,
    ACCTMGRPROP_DETERMINENETWORKSTATUS_BEGIN,
    ACCTMGRPROP_DETERMINENETWORKSTATUS_EXECUTE,
    ACCTMGRPROP_CLEANUP,
    ACCTMGRPROP_END
};

DEFINE_EVENT_TYPE(wxEVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE)

IMPLEMENT_DYNAMIC_CLASS(CAccountManagerPropertiesPage, wxWizardPage)

BEGIN_EVENT_TABLE(CAccountManagerPropertiesPage, wxWizardPage)
    EVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE(CAccountManagerPropertiesPage::OnStateChange)
    EVT_WIZARD_PAGE_CHANGED(-1, CAccountManagerPropertiesPage::OnPageChanged)
    EVT_WIZARD_CANCEL(-1, CAccountManagerPropertiesPage::OnCancel)
END_EVENT_TABLE()

/*!
 * CAccountManagerPropertiesPage constructors
 */

CAccountManagerPropertiesPage::CAccountManagerPropertiesPage() {
}

CAccountManagerPropertiesPage::CAccountManagerPropertiesPage(CBOINCBaseWizard* parent) {
    Create(parent);
}

/*!
 * CProjectPropertiesPage creator
 */

bool CAccountManagerPropertiesPage::Create(CBOINCBaseWizard* parent) {
    m_pTitleStaticCtrl = NULL;
    m_pPleaseWaitStaticCtrl = NULL;
    m_pProgressIndicator = NULL;

    m_bProjectPropertiesSucceeded = false;
    m_bProjectPropertiesURLFailure = false;
    m_bProjectAccountCreationDisabled = false;
    m_bProjectClientAccountCreationDisabled = false;
    m_bNetworkConnectionDetected = false;
    m_iBitmapIndex = 0;
    m_iCurrentState = ACCTMGRPROP_INIT;

    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create(parent, wizardBitmap);

    CreateControls();
    GetSizer()->Fit(this);
    return TRUE;
}

/*!
 * Control creation for CProjectPropertiesPage
 */

void CAccountManagerPropertiesPage::CreateControls() {    
    CAccountManagerPropertiesPage* itemWizardPage36 = this;

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
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTPROPERTIESPAGE
 */

void CAccountManagerPropertiesPage::OnPageChanged(wxWizardEvent& event) {
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

    SetProjectPropertiesSucceeded(false);
    SetProjectPropertiesURLFailure(false);
    SetProjectAccountCreationDisabled(false);
    SetProjectClientAccountCreationDisabled(false);
    SetNetworkConnectionDetected(false);
    SetNextState(ACCTMGRPROP_INIT);

    CAccountManagerPropertiesPageEvent TransitionEvent(wxEVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);

    Fit();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTPROPERTIESPAGE
 */
void CAccountManagerPropertiesPage::OnCancel(wxWizardEvent& event) {
    PROCESS_CANCELEVENT(event);
}

/*!
 * wxEVT_PROJECTPROPERTIES_STATECHANGE event handler for ID_PROJECTPROPERTIESPAGE
 */
void CAccountManagerPropertiesPage::OnStateChange(CAccountManagerPropertiesPageEvent& WXUNUSED(event)) {
    CMainDocument* pDoc         = wxGetApp().GetDocument();
    CWizardAccountManager* pWAM = ((CWizardAccountManager*)GetParent());
    PROJECT_CONFIG* pc          = pWAM->GetProjectConfig();
    CC_STATUS status;
    wxDateTime dtStartExecutionTime;
    wxDateTime dtCurrentExecutionTime;
    wxTimeSpan tsExecutionTime;
    bool bPostNewEvent = true;
    int  iReturnValue = 0;
 
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
 
    switch(GetCurrentState()) {
        case ACCTMGRPROP_INIT:
            pWAM->DisableNextButton();
            pWAM->DisableBackButton();
            StartProgress(m_pProgressIndicator);
            SetNextState(ACCTMGRPROP_RETRPROJECTPROPERTIES_BEGIN);
            break;
        case ACCTMGRPROP_RETRPROJECTPROPERTIES_BEGIN:
            SetNextState(ACCTMGRPROP_RETRPROJECTPROPERTIES_EXECUTE);
            break;
        case ACCTMGRPROP_RETRPROJECTPROPERTIES_EXECUTE:
            // Attempt to retrieve the project's account creation policies 
            // Wait until we are done processing the request.
            dtStartExecutionTime = wxDateTime::Now();
            dtCurrentExecutionTime = wxDateTime::Now();
            tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
            iReturnValue = 0;
            pc->clear();
            pc->error_num = ERR_RETRY;
            while ((!iReturnValue) && (tsExecutionTime.GetSeconds() <= 60)
                    && (!CHECK_CLOSINGINPROGRESS())
                    && ((ERR_IN_PROGRESS == pc->error_num) || (ERR_RETRY == pc->error_num))) {
                if (ERR_RETRY == pc->error_num) {
                    pDoc->rpc.get_project_config((const char*)
                                    pWAM->GetAccountManagerInfoPage()->GetProjectURL().mb_str());
                }
                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                iReturnValue = pDoc->rpc.get_project_config_poll(*pc);
                IncrementProgress(m_pProgressIndicator);

                ::wxMilliSleep(500);
                ::wxSafeYield(GetParent());
            }
 
            // We either successfully retrieved the project's account creation 
            //   policies or we were able to talk to the web server and found out
            //   they do not support account creation through the wizard.  In either
            //   case we should claim success and set the correct flags to show the
            //   correct 'next' page.
            if ((!iReturnValue) && (!pc->error_num) ||
                (!iReturnValue) && (ERR_ACCT_CREATION_DISABLED == pc->error_num)) {
                SetProjectPropertiesSucceeded(true);

                SetProjectAccountCreationDisabled(pc->account_creation_disabled);
                SetProjectClientAccountCreationDisabled(pc->client_account_creation_disabled);

                pWAM->SetProjectName(wxString(pc->name.c_str(), wxConvUTF8));
 
                SetNextState(ACCTMGRPROP_CLEANUP);
            } else {
                SetProjectPropertiesSucceeded(false);
                bool urlFailure = 
                    (!iReturnValue) && (ERR_FILE_NOT_FOUND == pc->error_num) ||
                    (!iReturnValue) && (ERR_GETHOSTBYNAME == pc->error_num) ||
                    (!iReturnValue) && (ERR_XML_PARSE == pc->error_num);
                SetProjectPropertiesURLFailure(urlFailure);
                SetNextState(ACCTMGRPROP_DETERMINENETWORKSTATUS_BEGIN);
            }
            break;
        case ACCTMGRPROP_DETERMINENETWORKSTATUS_BEGIN:
            SetNextState(ACCTMGRPROP_DETERMINENETWORKSTATUS_EXECUTE);
            break;
        case ACCTMGRPROP_DETERMINENETWORKSTATUS_EXECUTE:
            // Attempt to determine if we are even connected to a network

            // Wait until we are done processing the request.
            dtStartExecutionTime = wxDateTime::Now();
            dtCurrentExecutionTime = wxDateTime::Now();
            tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
            iReturnValue = 0;
            status.network_status = NETWORK_STATUS_LOOKUP_PENDING;
            while ((!iReturnValue && (NETWORK_STATUS_LOOKUP_PENDING == status.network_status)) &&
                   tsExecutionTime.GetSeconds() <= 60 &&
                   !CHECK_CLOSINGINPROGRESS()
                  )
            {
                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                iReturnValue = pDoc->GetCoreClientStatus(status);
                IncrementProgress(m_pProgressIndicator);

                ::wxMilliSleep(500);
                ::wxSafeYield(GetParent());
            }

            SetNetworkConnectionDetected(NETWORK_STATUS_WANT_CONNECTION != status.network_status);

            SetNextState(ACCTMGRPROP_CLEANUP);
            break;
        case ACCTMGRPROP_CLEANUP:
            FinishProgress(m_pProgressIndicator);
            SetNextState(ACCTMGRPROP_END);
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
        CAccountManagerPropertiesPageEvent TransitionEvent(wxEVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE, this);
        AddPendingEvent(TransitionEvent);
    }
}
   
/*!
 * Gets the previous page.
 */

wxWizardPage* CAccountManagerPropertiesPage::GetPrev() const {
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAccountManagerPropertiesPage::GetNext() const {
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (GetProjectPropertiesSucceeded()) {
        // We were successful in retrieving the project properties
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTINFOPAGE);
    } else if (GetProjectPropertiesURLFailure() && !GetNetworkConnectionDetected()) {
        // No Internet Connection
        return PAGE_TRANSITION_NEXT(ID_ERRPROXYINFOPAGE);
    } else if (GetProjectPropertiesURLFailure()) {
        // Not a BOINC based project
        return PAGE_TRANSITION_NEXT(ID_ERRNOTDETECTEDPAGE);
    } else {
        // The project must be down for maintenance
        return PAGE_TRANSITION_NEXT(ID_ERRUNAVAILABLEPAGE);
    }
}

/*!
 * Should we show tooltips?
 */

bool CAccountManagerPropertiesPage::ShowToolTips() {
    return TRUE;
}

 
void CAccountManagerPropertiesPage::StartProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 1;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress01.xpm")));
}
 
void CAccountManagerPropertiesPage::IncrementProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex += 1;
    if (12 < m_iBitmapIndex) m_iBitmapIndex = 1;
 
    wxString str;
    str.Printf(wxT("res/wizprogress%02d.xpm"), m_iBitmapIndex);
 
    pBitmap->SetBitmap(GetBitmapResource(str));
    Update();
}
 
void CAccountManagerPropertiesPage::FinishProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 12;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress12.xpm")));
}
 
/*!
 * Get bitmap resources
 */

wxBitmap CAccountManagerPropertiesPage::GetBitmapResource(const wxString& name) {
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

wxIcon CAccountManagerPropertiesPage::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}
