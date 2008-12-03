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
#ifndef WIZ_ACCOUNTMANAGER_H
#define WIZ_ACCOUNTMANAGER_H

#include "BOINCBaseWizard.h"

/// Forward declarations of all used pages:
class CAccountInfoPage;
class CAccountManagerInfoPage;
class CAccountManagerPropertiesPage;
class CAccountManagerProcessingPage;
class CCompletionErrorPage;
class CCompletionPage;
class CErrNotDetectedPage;
class CErrUnavailablePage;
class CErrNoInternetConnectionPage;
class CErrNotFoundPage;
class CErrProxyInfoPage;
class CErrProxyPage;
class CWelcomePage;

#define ACCOUNTMANAGER_ATTACH       0
#define ACCOUNTMANAGER_UPDATE       1
#define ACCOUNTMANAGER_DETACH       2

class CWizardAccountManager: public CBOINCBaseWizard
{    
    DECLARE_DYNAMIC_CLASS(CWizardAccountManager)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWizardAccountManager();
    CWizardAccountManager(wxWindow* parent, wxWindowID id = SYMBOL_CWIZARDACCOUNTMANAGER_IDNAME, const wxPoint& pos = wxDefaultPosition);

    /// Creation
    bool Create(wxWindow* parent, wxWindowID id = SYMBOL_CWIZARDACCOUNTMANAGER_IDNAME, const wxPoint& pos = wxDefaultPosition);

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_FINISHED event handler for ID_ATTACHACCOUNTMANAGERWIZARD
    void OnFinished(wxWizardEvent& event);

    /// Runs the wizard.
    bool Run(int action = ACCOUNTMANAGER_ATTACH);

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource(const wxString& name);

    /// Retrieves icon resources
    wxIcon GetIconResource(const wxString& name);

    /// Overrides
    virtual bool HasNextPage(wxWizardPage* page);
    virtual bool HasPrevPage(wxWizardPage* page);

    /// Track page transitions
    wxWizardPage* _PopPageTransition();
    wxWizardPage* _PushPageTransition(wxWizardPage* pCurrentPage, unsigned long ulPageID);

    /// Cancel Event Infrastructure
    void _ProcessCancelEvent(wxWizardEvent& event);

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Get the name of the project.
    wxString GetProjectName() const;

    /// Set the name of the project.
    void SetProjectName(const wxString& pr_name);

    /// Return a pointer to the current account info page.
    CAccountInfoPage* GetAccountInfoPage() const;

    /// Return a pointer to the current account manager info page.
    CAccountManagerInfoPage* GetAccountManagerInfoPage() const;

    /// Return a pointer to the current completion error page.
    CCompletionErrorPage* GetCompletionErrorPage() const;

    /// Return credentials cache status.
    bool GetCredentialsCached() const;

    /// Set credentials cache status.
    void SetCredentialsCached(bool credentials_cached);

    /// Check if the wizard is currently in update mode.
    bool IsUpdateWizard() const;

    /// Check if the wizard is currently in remove mode.
    bool IsRemoveWizard() const;

private:
    CWelcomePage* m_WelcomePage;
    CAccountManagerInfoPage* m_AccountManagerInfoPage;
    CAccountManagerPropertiesPage* m_AccountManagerPropertiesPage;
    CAccountManagerProcessingPage* m_AccountManagerProcessingPage;
    CAccountInfoPage* m_AccountInfoPage;
    CCompletionPage* m_CompletionPage;
    CCompletionErrorPage* m_CompletionErrorPage;
    CErrNotDetectedPage* m_ErrNotDetectedPage;
    CErrUnavailablePage* m_ErrUnavailablePage;
    CErrNoInternetConnectionPage* m_ErrNoInternetConnectionPage;
    CErrNotFoundPage* m_ErrNotFoundPage;
    CErrProxyInfoPage* m_ErrProxyInfoPage;
    CErrProxyPage* m_ErrProxyPage;
    wxString m_strProjectName;
    bool m_bCredentialsCached;
    bool m_IsUpdateWizard;
    bool m_IsRemoveWizard;
};

#endif // WIZ_ACCOUNTMANAGER_H
