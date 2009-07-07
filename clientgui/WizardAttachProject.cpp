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

#include "BOINCWizards.h"
#include "WizardAttachProject.h"

#include "stdwx.h"
#include "hyperlink.h"
#include "browser.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseWizard.h"
#include "BOINCBaseFrame.h"
#include "WelcomePage.h"
#include "ProjectInfoPage.h"
#include "ProjectPropertiesPage.h"
#include "AccountInfoPage.h"
#include "ProjectProcessingPage.h"
#include "CompletionPage.h"
#include "CompletionErrorPage.h"
#include "NotDetectedPage.h"
#include "UnavailablePage.h"
#include "NoInternetConnectionPage.h"
#include "NotFoundPage.h"
#include "AlreadyExistsPage.h"
#include "ProxyInfoPage.h"
#include "ProxyPage.h"


#ifdef __WXMSW__
EXTERN_C BOOL DetectSetupAuthenticator(LPCSTR szProjectURL, LPSTR szAuthenticator, LPDWORD lpdwSize);
#endif

IMPLEMENT_DYNAMIC_CLASS(CWizardAttachProject, CBOINCBaseWizard)
 
BEGIN_EVENT_TABLE(CWizardAttachProject, CBOINCBaseWizard)
    EVT_WIZARD_FINISHED(ID_ATTACHPROJECTWIZARD, CWizardAttachProject::OnFinished)
END_EVENT_TABLE()
 
/*!
 * CWizardAttachProject constructors
 */
 
CWizardAttachProject::CWizardAttachProject() {
}
 
CWizardAttachProject::CWizardAttachProject(wxWindow* parent, wxWindowID id, const wxPoint& pos) {
    Create(parent, id, pos);
}
 
/*!
 * CWizardAttachProject creator
 */
 
bool CWizardAttachProject::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos) {
    m_WelcomePage = NULL;
    m_ProjectInfoPage = NULL;
    m_ProjectPropertiesPage = NULL;
    m_AccountInfoPage = NULL;
    m_ProjectProcessingPage = NULL;
    m_CompletionPage = NULL;
    m_CompletionErrorPage = NULL;
    m_ErrNotDetectedPage = NULL;
    m_ErrUnavailablePage = NULL;
    m_ErrNoInternetConnectionPage = NULL;
    m_ErrNotFoundPage = NULL;
    m_ErrAlreadyExistsPage = NULL;
    m_ErrProxyInfoPage = NULL;
    m_ErrProxyPage = NULL;
 
    m_bCredentialsCached = false;
    m_bCredentialsDetected = false;

    CSkinAdvanced*  pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CSkinWizardATP* pSkinWizardATP = wxGetApp().GetSkinManager()->GetWizards()->GetWizardATP();

    wxASSERT(pSkinAdvanced);
    wxASSERT(pSkinWizardATP);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));
    wxASSERT(wxDynamicCast(pSkinWizardATP, CSkinWizardATP));


    wxString strTitle;
    if (!pSkinWizardATP->GetWizardTitle().IsEmpty()) {
        strTitle = pSkinWizardATP->GetWizardTitle();
    } else {
        strTitle = pSkinAdvanced->GetApplicationName();
    }

    wxBitmap wizardBitmap = wxBitmap(*(pSkinWizardATP->GetWizardBitmap()));

    CBOINCBaseWizard::Create(parent, id, strTitle, wizardBitmap, pos);

    CreateControls();
    return TRUE;
}
 
/*!
 * Control creation for CWizardAttachProject
 */

void CWizardAttachProject::CreateControls() {    
    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAttachProject::CreateControls - Function Begin"));

    CBOINCBaseWizard* itemWizard1 = this;

    m_WelcomePage = new CWelcomePage;
    m_WelcomePage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_WelcomePage);

    m_ProjectInfoPage = new CProjectInfoPage;
    m_ProjectInfoPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_ProjectInfoPage);

    m_ProjectPropertiesPage = new CProjectPropertiesPage;
    m_ProjectPropertiesPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_ProjectPropertiesPage);

    m_AccountInfoPage = new CAccountInfoPage;
    m_AccountInfoPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_AccountInfoPage);

    m_ProjectProcessingPage = new CProjectProcessingPage;
    m_ProjectProcessingPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_ProjectProcessingPage);

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

    m_ErrAlreadyExistsPage = new CErrAlreadyExistsPage;
    m_ErrAlreadyExistsPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_ErrAlreadyExistsPage);

    m_ErrProxyInfoPage = new CErrProxyInfoPage;
    m_ErrProxyInfoPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_ErrProxyInfoPage);

    m_ErrProxyPage = new CErrProxyPage;
    m_ErrProxyPage->Create(itemWizard1);
    GetPageAreaSizer()->Add(m_ErrProxyPage);

    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls - Begin Page Map"));
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_WelcomePage = id: '%d', location: '%p'"), m_WelcomePage->GetId(), m_WelcomePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ProjectInfoPage = id: '%d', location: '%p'"), m_ProjectInfoPage->GetId(), m_ProjectInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ProjectPropertiesPage = id: '%d', location: '%p'"), m_ProjectPropertiesPage->GetId(), m_ProjectPropertiesPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_AccountInfoPage = id: '%d', location: '%p'"), m_AccountInfoPage->GetId(), m_AccountInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ProjectProcessingPage = id: '%d', location: '%p'"), m_ProjectProcessingPage->GetId(), m_ProjectProcessingPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_CompletionPage = id: '%d', location: '%p'"), m_CompletionPage->GetId(), m_CompletionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_CompletionErrorPage = id: '%d', location: '%p'"), m_CompletionErrorPage->GetId(), m_CompletionErrorPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrNotDetectedPage = id: '%d', location: '%p'"), m_ErrNotDetectedPage->GetId(), m_ErrNotDetectedPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrUnavailablePage = id: '%d', location: '%p'"), m_ErrUnavailablePage->GetId(), m_ErrUnavailablePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrNoInternetConnectionPage = id: '%d', location: '%p'"), m_ErrNoInternetConnectionPage->GetId(), m_ErrNoInternetConnectionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrNotFoundPage = id: '%d', location: '%p'"), m_ErrNotFoundPage->GetId(), m_ErrNotFoundPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrAlreadyExistsPage = id: '%d', location: '%p'"), m_ErrAlreadyExistsPage->GetId(), m_ErrAlreadyExistsPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrProxyInfoPage = id: '%d', location: '%p'"), m_ErrProxyInfoPage->GetId(), m_ErrProxyInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrProxyPage = id: '%d', location: '%p'"), m_ErrProxyPage->GetId(), m_ErrProxyPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls - End Page Map"));
    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAttachProject::CreateControls - Function End"));
}
 
/*!
 * Runs the wizard.
 */
 
bool CWizardAttachProject::Run(wxString& WXUNUSED(strName), wxString& strURL, bool bCredentialsCached) {
    if (strURL.Length()) {
        m_ProjectInfoPage->SetProjectURL( strURL );
        m_bCredentialsCached = bCredentialsCached;
    }

    // If credentials are not cached, then we should try one last place to look up the
    //   authenticator.  Some projects will set a "Setup" cookie off of their URL with a
    //   pretty short timeout.  Lets take a crack at detecting it.
    //
    if (!strURL.IsEmpty() && !bCredentialsCached) {
        std::string url = std::string(strURL.mb_str());
        std::string authenticator;

        if (detect_setup_authenticator(url, authenticator)) {
            m_bCredentialsDetected = true;
            SetCloseWhenCompleted(true);
            SetProjectAuthenticator(wxString(authenticator.c_str(), wxConvUTF8));
        }
    }

    if ( strURL.Length() && (bCredentialsCached || m_bCredentialsDetected) && m_ProjectProcessingPage) {
        return RunWizard(m_ProjectProcessingPage);
    } else if (strURL.Length() && !bCredentialsCached && m_ProjectPropertiesPage) {
        return RunWizard(m_ProjectPropertiesPage);
    } else if (m_WelcomePage) {
        return RunWizard(m_WelcomePage);
    }

    return FALSE;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CWizardAttachProject::ShowToolTips() {
    return TRUE;
}

/// Return a pointer to the current account info page.
///
/// \return a pointer to the current account info page.
CAccountInfoPage* CWizardAttachProject::GetAccountInfoPage() const {
    return m_AccountInfoPage;
}

/// Return a pointer to the current completion error page.
///
/// \return a pointer to the current completion error page.
CCompletionErrorPage* CWizardAttachProject::GetCompletionErrorPage() const {
    return m_CompletionErrorPage;
}

/// Return a pointer to the current project info page.
///
/// \return a pointer to the current project info page.
CProjectInfoPage* CWizardAttachProject::GetProjectInfoPage() const {
    return m_ProjectInfoPage;
}

/// Return credentials cache status.
///
/// \return True if the credentials are cached, false otherwise.
bool CWizardAttachProject::GetCredentialsCached() const {
    return m_bCredentialsCached;
}

/// Tell whether the credentials were detected or not.
///
/// \return True if the credentials were detected, false otherwise.
bool CWizardAttachProject::GetCretentialsDetected() const {
    return m_bCredentialsDetected;
}

/*!
 * Get bitmap resources
 */
 
wxBitmap CWizardAttachProject::GetBitmapResource(const wxString& WXUNUSED(name)) {
    return wxNullBitmap;
}
 
/*!
 * Get icon resources
 */
 
wxIcon CWizardAttachProject::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}
 
/*!
 * Determine if the wizard page has a next page
 */

bool CWizardAttachProject::HasNextPage(wxWizardPage* page) {
    bool bNoNextPageDetected = false;

    bNoNextPageDetected |= (page == m_CompletionPage);
    bNoNextPageDetected |= (page == m_CompletionErrorPage);
    bNoNextPageDetected |= (page == m_ErrNotDetectedPage);
    bNoNextPageDetected |= (page == m_ErrUnavailablePage);
    bNoNextPageDetected |= (page == m_ErrNoInternetConnectionPage);
    bNoNextPageDetected |= (page == m_ErrAlreadyExistsPage);
 
    if (bNoNextPageDetected)
        return false;
    return true;
}
  
/*!
 * Determine if the wizard page has a previous page
 */
 
bool CWizardAttachProject::HasPrevPage(wxWizardPage* page) {
    if ((page == m_WelcomePage) || (page == m_CompletionPage) || (page == m_CompletionErrorPage))
        return false;
    return true;
}
 
/*!
 * Remove the page transition to the stack.
 */
wxWizardPage* CWizardAttachProject::_PopPageTransition() {
    wxWizardPage* pPage = NULL;
    if (GetCurrentPage()) {
        if (!m_PageTransition.empty()) {
            pPage = m_PageTransition.top();
            m_PageTransition.pop();
            if ((pPage == m_ProjectPropertiesPage) || (pPage == m_ProjectProcessingPage)) {
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
wxWizardPage* CWizardAttachProject::_PushPageTransition(wxWizardPage* pCurrentPage, unsigned long ulPageID) {
    if (GetCurrentPage()) {
        wxWizardPage* pPage = NULL;
        switch (ulPageID) {
            case ID_WELCOMEPAGE:
                pPage = m_WelcomePage;
                break;
            case ID_PROJECTINFOPAGE:
                pPage = m_ProjectInfoPage;
                break;
            case ID_PROJECTPROPERTIESPAGE:
                pPage = m_ProjectPropertiesPage;
                break;
            case ID_ACCOUNTINFOPAGE:
                pPage = m_AccountInfoPage;
                break;
            case ID_PROJECTPROCESSINGPAGE:
                pPage = m_ProjectProcessingPage;
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
            case ID_ERRALREADYEXISTSPAGE:
                pPage = m_ErrAlreadyExistsPage;
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
  
void CWizardAttachProject::_ProcessCancelEvent(wxWizardEvent& event) {
    wxWizardPage* page = GetCurrentPage();

    int iRetVal = ::wxMessageBox(_("Do you really want to cancel?"), _("Question"), wxICON_QUESTION | wxYES_NO, this);

    // Reenable the next and back buttons if they have been disabled
    EnableNextButton();
    EnableBackButton();

    // Page specific rules - Disable the validator(s)
    if (wxYES == iRetVal) {
        if (page == m_ProjectInfoPage) {
            m_ProjectInfoPage->DisableValidators();
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
 * wxEVT_WIZARD_FINISHED event handler for ID_ATTACHPROJECTWIZARD
 */

void CWizardAttachProject::OnFinished(wxWizardEvent& event) {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();

    if (GetAccountCreatedSuccessfully() && GetAttachedToProjectSuccessfully()) {
        HyperLink::ExecuteLink(GetProjectURL() + wxT("account_finish.php?auth=") + GetProjectAuthenticator());
    }

    // Let the framework clean things up.
    event.Skip();
}
