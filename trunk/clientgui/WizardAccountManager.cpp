// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
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

#include "WizardAccountManager.h"

#include "BOINCWizards.h"

#include "stdwx.h"
#include "hyperlink.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseWizard.h"
#include "BOINCBaseFrame.h"
#include "WelcomePage.h"
#include "AccountManagerInfoPage.h"
#include "AccountManagerPropertiesPage.h"
#include "AccountManagerProcessingPage.h"
#include "AccountInfoPage.h"
#include "CompletionPage.h"
#include "CompletionErrorPage.h"
#include "NotDetectedPage.h"
#include "UnavailablePage.h"
#include "NoInternetConnectionPage.h"
#include "NotFoundPage.h"
#include "ProxyInfoPage.h"
#include "ProxyPage.h"

IMPLEMENT_DYNAMIC_CLASS(CWizardAccountManager, CBOINCBaseWizard)

BEGIN_EVENT_TABLE(CWizardAccountManager, CBOINCBaseWizard)
    EVT_WIZARD_FINISHED(ID_ATTACHACCOUNTMANAGERWIZARD, CWizardAccountManager::OnFinished)
END_EVENT_TABLE()

/*!
 * CWizardAccountManager constructors
 */

CWizardAccountManager::CWizardAccountManager() : m_IsUpdateWizard(false), m_IsRemoveWizard(false) {
}

CWizardAccountManager::CWizardAccountManager(wxWindow* parent, wxWindowID id, const wxPoint& pos) : m_IsUpdateWizard(false), m_IsRemoveWizard(false) {
    Create(parent, id, pos);
}

/*!
 * CWizardAccountManager creator
 */

bool CWizardAccountManager::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos) {
    m_WelcomePage = NULL;
    m_AccountManagerInfoPage = NULL;
    m_AccountManagerPropertiesPage = NULL;
    m_AccountManagerProcessingPage = NULL;
    m_AccountInfoPage = NULL;
    m_CompletionPage = NULL;
    m_CompletionErrorPage = NULL;
    m_ErrNotDetectedPage = NULL;
    m_ErrUnavailablePage = NULL;
    m_ErrNoInternetConnectionPage = NULL;
    m_ErrNotFoundPage = NULL;
    m_ErrProxyInfoPage = NULL;
    m_ErrProxyPage = NULL;

    m_IsUpdateWizard = false;
    m_IsRemoveWizard = false;

    m_bCredentialsCached = false;

    CSkinAdvanced*   pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CSkinWizardATAM* pSkinWizardATAM = wxGetApp().GetSkinManager()->GetWizards()->GetWizardATAM();

    wxASSERT(pSkinAdvanced);
    wxASSERT(pSkinWizardATAM);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));
    wxASSERT(wxDynamicCast(pSkinWizardATAM, CSkinWizardATAM));

    wxString strTitle;
    if (!pSkinWizardATAM->GetWizardTitle().IsEmpty()) {
        strTitle = pSkinWizardATAM->GetWizardTitle();
    } else {
        strTitle = pSkinAdvanced->GetApplicationName();
    }

    wxBitmap wizardBitmap = wxBitmap(*(pSkinWizardATAM->GetWizardBitmap()));

    CBOINCBaseWizard::Create(parent, id, strTitle, wizardBitmap, pos);

    CreateControls();
    return TRUE;
}

/*!
 * Control creation for CWizardAccountManager
 */

void CWizardAccountManager::CreateControls() {    
    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAccountManager::CreateControls - Function Begin"));

    CBOINCBaseWizard* itemWizard1 = this;

    m_WelcomePage = new CWelcomePage;
    m_WelcomePage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_WelcomePage);

    m_AccountManagerInfoPage = new CAccountManagerInfoPage;
    m_AccountManagerInfoPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_AccountManagerInfoPage);

    m_AccountManagerPropertiesPage = new CAccountManagerPropertiesPage;
    m_AccountManagerPropertiesPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_AccountManagerPropertiesPage);

    m_AccountInfoPage = new CAccountInfoPage;
    m_AccountInfoPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_AccountInfoPage);

    m_AccountManagerProcessingPage = new CAccountManagerProcessingPage;
    m_AccountManagerProcessingPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_AccountManagerProcessingPage);

    m_CompletionPage = new CCompletionPage;
    m_CompletionPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_CompletionPage);

    m_CompletionErrorPage = new CCompletionErrorPage;
    m_CompletionErrorPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_CompletionErrorPage);

    m_ErrNotDetectedPage = new CErrNotDetectedPage;
    m_ErrNotDetectedPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_ErrNotDetectedPage);

    m_ErrUnavailablePage = new CErrUnavailablePage;
    m_ErrUnavailablePage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_ErrUnavailablePage);

    m_ErrNoInternetConnectionPage = new CErrNoInternetConnectionPage;
    m_ErrNoInternetConnectionPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_ErrNoInternetConnectionPage);

    m_ErrNotFoundPage = new CErrNotFoundPage;
    m_ErrNotFoundPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_ErrNotFoundPage);

    m_ErrProxyInfoPage = new CErrProxyInfoPage;
    m_ErrProxyInfoPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_ErrProxyInfoPage);

    m_ErrProxyPage = new CErrProxyPage;
    m_ErrProxyPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_ErrProxyPage);

    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls - Begin Page Map"));
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_WelcomePage = id: '%d', location: '%p'"), m_WelcomePage->GetId(), m_WelcomePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_AccountManagerInfoPage = id: '%d', location: '%p'"), m_AccountManagerInfoPage->GetId(), m_AccountManagerInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_AccountManagerPropertiesPage = id: '%d', location: '%p'"), m_AccountManagerPropertiesPage->GetId(), m_AccountManagerPropertiesPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_AccountInfoPage = id: '%d', location: '%p'"), m_AccountInfoPage->GetId(), m_AccountInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_AccountManagerProcessingPage = id: '%d', location: '%p'"), m_AccountManagerProcessingPage->GetId(), m_AccountManagerProcessingPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_CompletionPage = id: '%d', location: '%p'"), m_CompletionPage->GetId(), m_CompletionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_CompletionErrorPage = id: '%d', location: '%p'"), m_CompletionErrorPage->GetId(), m_CompletionErrorPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrNotDetectedPage = id: '%d', location: '%p'"), m_ErrNotDetectedPage->GetId(), m_ErrNotDetectedPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrUnavailablePage = id: '%d', location: '%p'"), m_ErrUnavailablePage->GetId(), m_ErrUnavailablePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrNoInternetConnectionPage = id: '%d', location: '%p'"), m_ErrNoInternetConnectionPage->GetId(), m_ErrNoInternetConnectionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrNotFoundPage = id: '%d', location: '%p'"), m_ErrNotFoundPage->GetId(), m_ErrNotFoundPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrProxyInfoPage = id: '%d', location: '%p'"), m_ErrProxyInfoPage->GetId(), m_ErrProxyInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrProxyPage = id: '%d', location: '%p'"), m_ErrProxyPage->GetId(), m_ErrProxyPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls - End Page Map"));
    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAccountManager::CreateControls - Function End"));
}

/*!
 * Runs the wizard.
 */

bool CWizardAccountManager::Run(int action) {
    ACCT_MGR_INFO ami;
    CMainDocument*            pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    pDoc->rpc.acct_mgr_info(ami);

    if (!ami.acct_mgr_url.empty()) {
        m_AccountManagerInfoPage->SetProjectURL( wxString(ami.acct_mgr_url.c_str(), wxConvUTF8) );
        m_strProjectName = wxString(ami.acct_mgr_name.c_str(), wxConvUTF8);
        m_bCredentialsCached = ami.have_credentials;
    }

    if (!ami.acct_mgr_url.empty() && !ami.have_credentials) {
        return RunWizard(m_AccountManagerPropertiesPage);
    } else if (!ami.acct_mgr_url.empty() && ami.have_credentials && (action == ACCOUNTMANAGER_UPDATE)) {
        m_IsUpdateWizard = true;
        m_IsRemoveWizard = false;
        return RunWizard(m_AccountManagerProcessingPage);
    } else if (!ami.acct_mgr_url.empty() && ami.have_credentials && (action == ACCOUNTMANAGER_DETACH)) {
        m_IsUpdateWizard = false;
        m_IsRemoveWizard = true;
        m_AccountManagerInfoPage->SetProjectURL(wxEmptyString);
        m_AccountInfoPage->SetAccountEmailAddress(wxEmptyString);
        m_AccountInfoPage->SetAccountPassword(wxEmptyString);
        m_bCredentialsCached = false;
        return RunWizard(m_WelcomePage);
    } else if (m_WelcomePage) {
        return RunWizard(m_WelcomePage);
    }
    return FALSE;
}

/*!
 * Should we show tooltips?
 */

bool CWizardAccountManager::ShowToolTips() {
    return TRUE;
}

/// Get the name of the project.
///
/// \return The name of the project.
wxString CWizardAccountManager::GetProjectName() const {
    return m_strProjectName;
}

/// Set the name of the project.
///
/// \param[in] pr_name The name of the project.
void CWizardAccountManager::SetProjectName(const wxString& pr_name) {
    m_strProjectName = pr_name;
}

/// Return a pointer to the current account info page.
///
/// \return a pointer to the current account info page.
CAccountInfoPage* CWizardAccountManager::GetAccountInfoPage() const {
    return m_AccountInfoPage;
}

/// Return a pointer to the current account manager info page.
///
/// \return a pointer to the current account manager info page.
CAccountManagerInfoPage* CWizardAccountManager::GetAccountManagerInfoPage() const {
    return m_AccountManagerInfoPage;
}

/// Return a pointer to the current completion error page.
///
/// \return a pointer to the current completion error page.
CCompletionErrorPage* CWizardAccountManager::GetCompletionErrorPage() const {
    return m_CompletionErrorPage;
}

/// Return credentials cache status.
///
/// \return True if the credentials are cached, false otherwise.
bool CWizardAccountManager::GetCredentialsCached() const {
    return m_bCredentialsCached;
}

/// Set credentials cache status.
///
/// \param[in] credentials_cached New value for the credentials cache status.
void CWizardAccountManager::SetCredentialsCached(bool credentials_cached) {
    m_bCredentialsCached = credentials_cached;
}

/// Check if the wizard is currently in update mode.
///
/// \return True if the wizard is in update mode, false otherwise.
bool CWizardAccountManager::IsUpdateWizard() const {
    return m_IsUpdateWizard;
}

/*!
 * Get bitmap resources
 */

wxBitmap CWizardAccountManager::GetBitmapResource(const wxString& WXUNUSED(name)) {
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CWizardAccountManager::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}

/*!
 * Determine if the wizard page has a next page
 */

bool CWizardAccountManager::HasNextPage(wxWizardPage* page) {
    bool bNoNextPageDetected = false;

    bNoNextPageDetected |= (page == m_CompletionPage);
    bNoNextPageDetected |= (page == m_CompletionErrorPage);
    bNoNextPageDetected |= (page == m_ErrNotDetectedPage);
    bNoNextPageDetected |= (page == m_ErrUnavailablePage);
    bNoNextPageDetected |= (page == m_ErrNoInternetConnectionPage);
 
    if (bNoNextPageDetected)
        return false;
    return true;
}
  
/*!
 * Determine if the wizard page has a previous page
 */
 
bool CWizardAccountManager::HasPrevPage(wxWizardPage* page) {
    bool bNoPrevPageDetected = false;

    bNoPrevPageDetected |= (page == m_WelcomePage);
    bNoPrevPageDetected |= (page == m_CompletionPage);
    bNoPrevPageDetected |= (page == m_CompletionErrorPage);

    if (bNoPrevPageDetected)
        return false;
    return true;
}
 
/*!
 * Remove the page transition to the stack.
 */
 
wxWizardPage* CWizardAccountManager::_PopPageTransition() {
    wxWizardPage* pPage = NULL;
    if (GetCurrentPage()) {
        if (!m_PageTransition.empty()) {
            pPage = m_PageTransition.top();
            m_PageTransition.pop();
            if ((pPage == m_AccountManagerPropertiesPage) || (pPage == m_AccountManagerProcessingPage)) {
                // We want to go back to the page before we attempted to communicate
                //   with any server.
                pPage = m_PageTransition.top();
                m_PageTransition.pop();
            }
            wxASSERT(pPage);
            return pPage;
        }
    }
    return NULL;
}
 
/*!
 * Add the page transition to the stack.
 */

wxWizardPage* CWizardAccountManager::_PushPageTransition(wxWizardPage* pCurrentPage, unsigned long ulPageID) {
    if (GetCurrentPage()) {
        wxWizardPage* pPage = NULL;

        switch (ulPageID) {
            case ID_WELCOMEPAGE:
                pPage = m_WelcomePage;
                break;
            case ID_ACCOUNTMANAGERINFOPAGE:
                pPage = m_AccountManagerInfoPage;
                break;
            case ID_ACCOUNTMANAGERPROPERTIESPAGE:
                pPage = m_AccountManagerPropertiesPage;
                break;
            case ID_ACCOUNTINFOPAGE:
                pPage = m_AccountInfoPage;
                break;
            case ID_ACCOUNTMANAGERPROCESSINGPAGE:
                pPage = m_AccountManagerProcessingPage;
                break;
            case ID_COMPLETIONPAGE:
                pPage = m_CompletionPage;
                break;
            case ID_COMPLETIONERRORPAGE:
                pPage = m_CompletionErrorPage;
                break;
            case ID_ERRNOTDETECTEDPAGE:
                pPage = m_ErrNotDetectedPage;
                break;
            case ID_ERRUNAVAILABLEPAGE:
                pPage = m_ErrUnavailablePage;
                break;
            case ID_ERRNOINTERNETCONNECTIONPAGE:
                pPage = m_ErrNoInternetConnectionPage;
                break;
            case ID_ERRNOTFOUNDPAGE:
                pPage = m_ErrNotFoundPage;
                break;
            case ID_ERRPROXYINFOPAGE:
                pPage = m_ErrProxyInfoPage;
                break;
            case ID_ERRPROXYPAGE:
                pPage = m_ErrProxyPage;
                break;
        }
        if (pPage) {
            if (m_PageTransition.empty()) {
                m_PageTransition.push(NULL);
            }
            if (m_PageTransition.top() != pCurrentPage) {
                m_PageTransition.push(pCurrentPage);
            }
            return pPage;
        }
    }
    return NULL;
}
  
void CWizardAccountManager::_ProcessCancelEvent(wxWizardEvent& event) {
    wxWizardPage* page = GetCurrentPage();

    int iRetVal = ::wxMessageBox(_("Do you really want to cancel?"), _("Question"), wxICON_QUESTION | wxYES_NO, this);

    // Reenable the next and back buttons if they have been disabled
    EnableNextButton();
    EnableBackButton();

    // Page specific rules - Disable the validator(s)
    if (wxYES == iRetVal) {
         if (page == m_AccountManagerInfoPage) {
            m_AccountManagerInfoPage->DisableValidators();
        } else if (page == m_AccountInfoPage) {
            m_AccountInfoPage->DisableValidators();
        } else if (page == m_ErrProxyPage) {
            m_ErrProxyPage->DisableValidators();
        }
    } else {
        event.Veto();
    }
}
 
/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_ATTACHACCOUNTMANAGERWIZARD
 */

void CWizardAccountManager::OnFinished(wxWizardEvent& event) {
    event.Skip();
}
