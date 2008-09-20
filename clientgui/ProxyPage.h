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
#ifndef _WIZ_PROXYPAGE_H_
#define WIZ_PROXYPAGE_H

#include <wx/wizard.h>

class wxStaticText;
class wxTextCtrl;
class wxStaticBox;
class CBOINCBaseWizard;

class CErrProxyPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS(CErrProxyPage)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CErrProxyPage();
    CErrProxyPage(CBOINCBaseWizard* parent);

    /// Creation
    bool Create(CBOINCBaseWizard* parent);

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROXYPAGE
    void OnPageChanged(wxWizardEvent& event);

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ERRPROXYPAGE
    void OnPageChanging(wxWizardEvent& event);

    /// wxEVT_WIZARD_CANCEL event handler for ID_ERRPROXYPAGE
    void OnCancel(wxWizardEvent& event);

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    wxString GetProxyHTTPServer() const { return m_strProxyHTTPServer; }
    void SetProxyHTTPServer(wxString value) { m_strProxyHTTPServer = value; }

    wxString GetProxyHTTPPort() const { return m_strProxyHTTPPort; }
    void SetProxyHTTPPort(wxString value) { m_strProxyHTTPPort = value; }

    wxString GetProxyHTTPUsername() const { return m_strProxyHTTPUsername; }
    void SetProxyHTTPUsername(wxString value) { m_strProxyHTTPUsername = value; }

    wxString GetProxyHTTPPassword() const { return m_strProxyHTTPPassword; }
    void SetProxyHTTPPassword(wxString value) { m_strProxyHTTPPassword = value; }

    wxString GetProxySOCKSServer() const { return m_strProxySOCKSServer; }
    void SetProxySOCKSServer(wxString value) { m_strProxySOCKSServer = value; }

    wxString GetProxySOCKSPort() const { return m_strProxySOCKSPort; }
    void SetProxySOCKSPort(wxString value) { m_strProxySOCKSPort = value; }

    wxString GetProxySOCKSUsername() const { return m_strProxySOCKSUsername; }
    void SetProxySOCKSUsername(wxString value) { m_strProxySOCKSUsername = value; }

    wxString GetProxySOCKSPassword() const { return m_strProxySOCKSPassword; }
    void SetProxySOCKSPassword(wxString value) { m_strProxySOCKSPassword = value; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource(const wxString& name);

    /// Retrieves icon resources
    wxIcon GetIconResource(const wxString& name);

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Disables the validators for all controls on this page.
    void DisableValidators();

private:
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticBox* m_pProxyHTTPDescriptionCtrl; 
    wxStaticText* m_pProxyHTTPServerStaticCtrl;
    wxTextCtrl* m_pProxyHTTPServerCtrl;
    wxStaticText* m_pProxyHTTPPortStaticCtrl;
    wxTextCtrl* m_pProxyHTTPPortCtrl;
    wxStaticText* m_pProxyHTTPUsernameStaticCtrl;
    wxTextCtrl* m_pProxyHTTPUsernameCtrl;
    wxStaticText* m_pProxyHTTPPasswordStaticCtrl;
    wxTextCtrl* m_pProxyHTTPPasswordCtrl;
#if 0
    wxButton* m_pProxyHTTPAutodetectCtrl;
#endif
    wxStaticBox* m_pProxySOCKSDescriptionCtrl; 
    wxStaticText* m_pProxySOCKSServerStaticCtrl;
    wxTextCtrl* m_pProxySOCKSServerCtrl;
    wxStaticText* m_pProxySOCKSPortStaticCtrl;
    wxTextCtrl* m_pProxySOCKSPortCtrl;
    wxStaticText* m_pProxySOCKSUsernameStaticCtrl;
    wxTextCtrl* m_pProxySOCKSUsernameCtrl;
    wxStaticText* m_pProxySOCKSPasswordStaticCtrl;
    wxTextCtrl* m_pProxySOCKSPasswordCtrl;
    wxString m_strProxyHTTPServer;
    wxString m_strProxyHTTPPort;
    wxString m_strProxyHTTPUsername;
    wxString m_strProxyHTTPPassword;
    wxString m_strProxySOCKSServer;
    wxString m_strProxySOCKSPort;
    wxString m_strProxySOCKSUsername;
    wxString m_strProxySOCKSPassword;
};

#endif // WIZ_PROXYPAGE_H
