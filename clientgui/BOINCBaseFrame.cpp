// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
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

#include "BOINCBaseFrame.h"

#include "stdwx.h"
#include "version.h"
#include "util.h"
#include "hyperlink.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCClientManager.h"
#include "BOINCTaskBar.h"

#ifndef __WXMAC__
#    include "BOINCDialupManager.h"
#endif // __WXMAC__

#include "Events.h"


DEFINE_EVENT_TYPE(wxEVT_FRAME_ALERT)
DEFINE_EVENT_TYPE(wxEVT_FRAME_CONNECT)
DEFINE_EVENT_TYPE(wxEVT_FRAME_INITIALIZED)
DEFINE_EVENT_TYPE(wxEVT_FRAME_REFRESHVIEW)
DEFINE_EVENT_TYPE(wxEVT_FRAME_UPDATESTATUS)
DEFINE_EVENT_TYPE(wxEVT_FRAME_RELOADSKIN)


IMPLEMENT_DYNAMIC_CLASS(CBOINCBaseFrame, wxFrame)

BEGIN_EVENT_TABLE (CBOINCBaseFrame, wxFrame)
    EVT_TIMER(ID_DOCUMENTPOLLTIMER, CBOINCBaseFrame::OnDocumentPoll)
    EVT_TIMER(ID_ALERTPOLLTIMER, CBOINCBaseFrame::OnAlertPoll)
    EVT_FRAME_INITIALIZED(CBOINCBaseFrame::OnInitialized)
    EVT_FRAME_ALERT(CBOINCBaseFrame::OnAlert)
    EVT_CLOSE(CBOINCBaseFrame::OnClose)
    EVT_MENU(ID_FILECLOSEWINDOW, CBOINCBaseFrame::OnCloseWindow)
END_EVENT_TABLE ()


CBOINCBaseFrame::CBOINCBaseFrame()
{
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::CBOINCBaseFrame - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::CBOINCBaseFrame - Default Constructor Function End"));
}


CBOINCBaseFrame::CBOINCBaseFrame(wxWindow* parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long style) :
    wxFrame(parent, id, title, pos, size, style) 
{
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::CBOINCBaseFrame - Function Begin"));

    // Configuration Settings
    m_iSelectedLanguage = 0;
    m_iReminderFrequency = 0;
    wxGetApp().SetDisplayExitWarning(1);

    m_strNetworkDialupConnectionName = wxEmptyString;

    m_aSelectedComputerMRU.Clear();

    m_bShowConnectionFailedAlert = false;

#ifndef __WXMAC__
    m_pDialupManager = new CBOINCDialUpManager();
    wxASSERT(m_pDialupManager->IsOk());
#endif // __WXMAC__


    m_pDocumentPollTimer = new wxTimer(this, ID_DOCUMENTPOLLTIMER);
    wxASSERT(m_pDocumentPollTimer);

    m_pDocumentPollTimer->Start(250);                // Send event every 250 milliseconds

    m_pAlertPollTimer = new wxTimer(this, ID_ALERTPOLLTIMER);
    wxASSERT(m_pAlertPollTimer);

    m_pAlertPollTimer->Start(1000);                  // Send event every 1000 milliseconds


    // Limit the number of times the UI can update itself to two times a second
    //   NOTE: Linux and Mac were updating several times a second and eating
    //         CPU time
    wxUpdateUIEvent::SetUpdateInterval(500);

    // The second half of the initialization process picks up in the OnFrameRender()
    //   routine since the menus' and status bars' are drawn in the frameworks
    //   on idle routines, on idle events are sent in between the end of the
    //   constructor and the first call to OnFrameRender
    //
    // Look for the 'if (!bAlreadyRunOnce) {' statement

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::CBOINCBaseFrame - Function End"));
}


CBOINCBaseFrame::~CBOINCBaseFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::~CBOINCBaseFrame - Function Begin"));

    wxASSERT(m_pAlertPollTimer);
    wxASSERT(m_pDocumentPollTimer);

    if (m_pAlertPollTimer) {
        m_pAlertPollTimer->Stop();
        delete m_pAlertPollTimer;
        m_pAlertPollTimer = 0;
    }

    if (m_pDocumentPollTimer) {
        m_pDocumentPollTimer->Stop();
        delete m_pDocumentPollTimer;
        m_pDocumentPollTimer = 0;
    }

#ifndef __WXMAC__
    delete m_pDialupManager;
    m_pDialupManager = 0;
#endif // __WXMAC__

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::~CBOINCBaseFrame - Function End"));
}


void CBOINCBaseFrame::OnDocumentPoll(wxTimerEvent& WXUNUSED(event)) {
    static bool        bAlreadyRunOnce = false;
    CMainDocument*     pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!bAlreadyRunOnce && m_pDocumentPollTimer->IsRunning()) {
        // Complete any remaining initialization that has to happen after we are up
        //   and running
        FireInitialize();
        bAlreadyRunOnce = true;
    }

    pDoc->OnPoll();
}


void CBOINCBaseFrame::OnAlertPoll(wxTimerEvent& WXUNUSED(event)) {
    static bool       bAlreadyRunningLoop = false;
    CMainDocument*    pDoc = wxGetApp().GetDocument();

    if (!bAlreadyRunningLoop && m_pAlertPollTimer->IsRunning()) {
        bAlreadyRunningLoop = true;

        // Update idle detection if needed.
        wxGetApp().UpdateSystemIdleDetection();

#ifndef __WXMAC__
        // Check to see if there is anything that we need to do from the
        //   dial up user perspective.
        if (pDoc && m_pDialupManager) {
            if (pDoc->IsConnected()) {
                m_pDialupManager->OnPoll();
            }
        }
#endif // __WXMAC__

        if (m_bShowConnectionFailedAlert && IsShown() && !IsIconized()) {
            m_bShowConnectionFailedAlert = false;
            ShowConnectionFailedAlert();
        }

        bAlreadyRunningLoop = false;
    }
}


void CBOINCBaseFrame::OnInitialized(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnInitialized - Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnInitialized - Function End"));
}


void CBOINCBaseFrame::OnAlert(CFrameAlertEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnAlert - Function Begin"));
    static bool       bAlreadyRunningLoop = false;

    if (!bAlreadyRunningLoop) {
        bAlreadyRunningLoop = true;

#ifdef __WXMSW__
        CTaskBarIcon* pTaskbar = wxGetApp().GetTaskBarIcon();
        wxASSERT(pTaskbar);

        if ((event.m_notification_only || !IsShown() || IsIconized())
            && pTaskbar->IsBalloonsSupported()) {
            // If the main window is hidden or minimized use the system tray ballon
            //   to notify the user instead.  This keeps dialogs from interfering
            //   with people typing email messages or any other activity where they
            //   do not want keyboard focus changed to another window while typing.
            unsigned int  icon_type;

            if (wxICON_ERROR & event.m_style) {
                icon_type = NIIF_ERROR;
            } else if (wxICON_WARNING & event.m_style) {
                icon_type = NIIF_WARNING;
            } else if (wxICON_INFORMATION & event.m_style) {
                icon_type = NIIF_INFO;
            } else {
                icon_type = NIIF_NONE;
            }

            pTaskbar->SetBalloon(
                pTaskbar->m_iconTaskBarNormal,
                event.m_title,
                event.m_message,
                5000,
                icon_type
            );
        } else {
            if (!event.m_notification_only) {
                int retval = 0;

                if (!IsShown()) {
                    Show();
                }

                retval = ::wxMessageBox(event.m_message, event.m_title, event.m_style, this);
                if (event.m_alert_event_type == AlertProcessResponse) {
                    event.ProcessResponse(retval);
                }
            }
        }
#elif defined (__WXMAC__)
        // wxMessageBox() / ProcessResponse() hangs the Manager if hidden.
        // Currently, the only non-notification-only alert is Connection Failed,
        // which now has logic to be displayed when Manager is restored.

        // Notification only events on platforms other than Windows are 
        //   currently discarded.  Otherwise the application would be restored 
        //   and input focus set on the notification which interrupts whatever 
        //   the user was doing.
        if (IsShown() && !event.m_notification_only) {
            int retval = 0;

            retval = ::wxMessageBox(event.m_message, event.m_title, event.m_style, this);
            if (event.m_alert_event_type == AlertProcessResponse) {
                event.ProcessResponse(retval);
            }
        }
#endif

        bAlreadyRunningLoop = false;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnAlert - Function End"));
}


void CBOINCBaseFrame::OnClose(wxCloseEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnClose - Function Begin"));

#ifdef SYNEC_USE_TASKBARICON
    if (!event.CanVeto()) {
        Destroy();
    } else {
        Hide();
    }
#else
    Destroy();
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnClose - Function End"));
}


void CBOINCBaseFrame::OnCloseWindow(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnCloseWindow - Function Begin"));

    Close();

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnCloseWindow - Function End"));
}


void CBOINCBaseFrame::OnExit(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnExit - Function Begin"));

    if (wxGetApp().ConfirmExit()) {
        // Under wxWidgets 2.8.0, the task bar icons must be deleted for app to exit its main loop
#ifdef __WXMAC__
        CMacSystemMenu* pMSM = wxGetApp().GetMacSystemMenu();
        if (pMSM)
            delete pMSM;
#endif

        // TaskBarIcon isn't used in Linux
#ifdef SYNEC_USE_TASKBARICON
        CTaskBarIcon* pTBI = wxGetApp().GetTaskBarIcon();
        if (pTBI && !pTBI->m_bTaskbarInitiatedShutdown) {
            delete pTBI;
        }
#endif
        Close(true);

    }
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnExit - Function End"));
}


void CBOINCBaseFrame::OnContextHelp(wxHelpEvent& event) {
    ShowHelp(event);
}


void CBOINCBaseFrame::OnHelp(wxCommandEvent& event) {
    ShowHelp(event);
}


void CBOINCBaseFrame::ShowHelp(wxEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnHelp - Function Begin"));

    if (IsShown()) {
        CSkinAdvanced* skin = wxGetApp().GetSkinManager()->GetAdvanced();
        if (event.GetId() == ID_HELPBOINCWEBSITE) {
            HyperLink::ExecuteLink(skin->GetOrganizationWebsite());
        } else {
            wxString url = skin->GetOrganizationHelpUrl();

            url << wxT("?version=") << wxT(SYNEC_VERSION_STRING);
            HyperLink::ExecuteLink(url);
        }
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnHelp - Function End"));
}


void CBOINCBaseFrame::FireInitialize() {
    CFrameEvent event(wxEVT_FRAME_INITIALIZED, this);
    AddPendingEvent(event);
}


void CBOINCBaseFrame::FireRefreshView() {
    CFrameEvent event(wxEVT_FRAME_REFRESHVIEW, this);
    AddPendingEvent(event);
}


void CBOINCBaseFrame::FireConnect() {
    CFrameEvent event(wxEVT_FRAME_CONNECT, this);
    AddPendingEvent(event);
}


void CBOINCBaseFrame::FireReloadSkin() {
    CFrameEvent event(wxEVT_FRAME_RELOADSKIN, this);
    AddPendingEvent(event);
}


void CBOINCBaseFrame::ShowConnectionBadPasswordAlert( bool bUsedDefaultPassword, int m_iReadGUIRPCAuthFailure ) {
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString            title;
    wxString            passwordErrorReason;

    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowConnectionBadPasswordAlert - Function Begin"));

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    title.Printf(
        _("%s - Connection Error"),
        pSkinAdvanced->GetApplicationName().c_str()
    );

    if (bUsedDefaultPassword) {
        passwordErrorReason = _("Authorization failed connecting to running client.");
        if (m_iReadGUIRPCAuthFailure) {
            passwordErrorReason << wxT("\n") << _("Could not read ") << _(GUI_RPC_PASSWD_FILE);
        }
    } else {
        passwordErrorReason = _("The password you have provided is incorrect, please try again.");
    }

    ShowAlert(
        title,
        passwordErrorReason,
        wxOK | wxICON_ERROR
    );

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowConnectionBadPasswordAlert - Function End"));
}


void CBOINCBaseFrame::ShowConnectionFailedAlert() {
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    wxString            strDialogTitle = wxEmptyString;
    wxString            strDialogMessage = wxEmptyString;

    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowConnectionFailedAlert - Function Begin"));

    // Did BOINC crash on local computer? If so restart it and reconnect.
    if (pDoc->IsLocalClient()) {
        if (pDoc->m_pClientManager->AutoRestart()) {
            boinc_sleep(0.5);       // Allow time for Client to restart
            if (pDoc->m_pClientManager->IsBOINCCoreRunning()) {
                pDoc->Reconnect();        
                return;
            }
        }
    }

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Connection Failed"),
        pSkinAdvanced->GetApplicationName().c_str()
    );

    // 1st %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    // 2st %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s is not able to connect to a %s client.\n"
          "Would you like to try to connect again?"),
        pSkinAdvanced->GetApplicationName().c_str(),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );

    ShowAlert(
        strDialogTitle,
        strDialogMessage,
        wxYES_NO | wxICON_QUESTION,
        false,
        AlertProcessResponse
    );

    // If we are minimized, set flag to show alert when maximized
    m_bShowConnectionFailedAlert = !IsShown() || IsIconized();

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowConnectionFailedAlert - Function End"));
}


void CBOINCBaseFrame::ShowDaemonStartFailedAlert() {
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString            strDialogTitle = wxEmptyString;
    wxString            strDialogMessage = wxEmptyString;


    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowDaemonStartFailedAlert - Function Begin"));


    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Daemon Start Failed"),
        pSkinAdvanced->GetApplicationName().c_str()
    );

    // 1st %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    // 2st %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
#ifdef __WXMSW__
    strDialogMessage.Printf(
        _("%s is not able to start a %s client.\n"
          "Please launch the Control Panel->Administative Tools->Services "
          "applet and start the BOINC service."),
        pSkinAdvanced->GetApplicationName().c_str(),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
#else
    strDialogMessage.Printf(
        _("%s is not able to start a %s client.\n"
          "Please start the daemon and try again."),
        pSkinAdvanced->GetApplicationName().c_str(),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
#endif

    ShowAlert(
        strDialogTitle,
        strDialogMessage,
        wxOK | wxICON_ERROR
    );

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowDaemonStartFailedAlert - Function End"));
}


void CBOINCBaseFrame::ShowNotCurrentlyConnectedAlert() {
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    wxString            strDialogTitle = wxEmptyString;
    wxString            strDialogMessage = wxEmptyString;

    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowNotCurrentlyConnectedAlert - Function Begin"));

    // Did BOINC crash on local computer? If so restart it and reconnect.
    if (pDoc->IsLocalClient()) {
        if (pDoc->m_pClientManager->AutoRestart()) {
            boinc_sleep(0.5);       // Allow time for Client to restart
            if (pDoc->m_pClientManager->IsBOINCCoreRunning()) {
                pDoc->Reconnect();        
                return;
            }
        }
    }
    
    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Connection Status"),
        pSkinAdvanced->GetApplicationName().c_str()
    );

    // 1st %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    // 2nd %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    // 3nd %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s is not currently connected to a %s client.\n"
          "Please use the 'Advanced\\Select Computer...' menu option to connect up to a %s client.\n"
          "To connect up to your local computer please use 'localhost' as the host name."),
        pSkinAdvanced->GetApplicationName().c_str(),
        pSkinAdvanced->GetApplicationShortName().c_str(),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    ShowAlert(
        strDialogTitle,
        strDialogMessage,
        wxOK | wxICON_ERROR
    );

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowNotCurrentlyConnectedAlert - Function End"));
}


void CBOINCBaseFrame::StartTimers() {
    wxASSERT(m_pAlertPollTimer);
    m_pAlertPollTimer->Start();
}


void CBOINCBaseFrame::StopTimers() {
    wxASSERT(m_pAlertPollTimer);
    m_pAlertPollTimer->Stop();
}


void CBOINCBaseFrame::UpdateStatusText(const wxChar* szStatus) {
    CFrameEvent event(wxEVT_FRAME_UPDATESTATUS, this, szStatus);
    ProcessEvent(event);
}


void CBOINCBaseFrame::ShowAlert( const wxString title, const wxString message, const int style, const bool notification_only, const FrameAlertEventType alert_event_type ) {
    CFrameAlertEvent event(wxEVT_FRAME_ALERT, this, title, message, style, notification_only, alert_event_type);
    AddPendingEvent(event);
}

bool CBOINCBaseFrame::SaveState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::SaveState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxString        strConfigLocation;
    wxString        strPreviousLocation;
    wxString        strBuffer;

    wxASSERT(pConfig);

    // An odd case happens every once and awhile where wxWidgets looses
    // the pointer to the config object, or it is cleaned up before
    // the window has finished it's cleanup duty.  If we detect a NULL
    // pointer, return false.
    if (!pConfig) {
        return false;
    }

    // Save Frame State
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("Language"), m_iSelectedLanguage);
    pConfig->Write(wxT("ReminderFrequency"), m_iReminderFrequency);
    pConfig->Write(wxT("DisplayExitWarning"), wxGetApp().GetDisplayExitWarning());
    pConfig->Write(wxT("NetworkDialupConnectionName"), m_strNetworkDialupConnectionName);

    // Save Computer MRU list
    strPreviousLocation = pConfig->GetPath();
    strConfigLocation = strPreviousLocation + wxT("ComputerMRU");

    pConfig->SetPath(strConfigLocation);
    size_t iItemCount = m_aSelectedComputerMRU.GetCount();
    for (size_t iIndex = 0; iIndex < iItemCount; ++iIndex) {
        strBuffer.Printf(wxT("%lu"), iIndex);
        pConfig->Write(strBuffer, m_aSelectedComputerMRU.Item(iIndex));
    }

    pConfig->SetPath(strPreviousLocation);

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::SaveState - Function End"));
    return true;
}


bool CBOINCBaseFrame::RestoreState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::RestoreState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxString        strConfigLocation;
    wxString        strPreviousLocation;
    wxString        strBuffer;
    wxString        strValue;
    long            iIndex;
    bool            bKeepEnumerating = false;
    int             iDisplayExitWarning;


    wxASSERT(pConfig);

    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Restore Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read(wxT("Language"), &m_iSelectedLanguage, 0L);
    pConfig->Read(wxT("ReminderFrequency"), &m_iReminderFrequency, 60L);
    pConfig->Read(wxT("DisplayExitWarning"), &iDisplayExitWarning, 1L);
    wxGetApp().SetDisplayExitWarning(iDisplayExitWarning);

    pConfig->Read(wxT("NetworkDialupConnectionName"), &m_strNetworkDialupConnectionName, wxEmptyString);


    //
    // Restore Computer MRU list
    //
    strPreviousLocation = pConfig->GetPath();
    strConfigLocation = strPreviousLocation + wxT("ComputerMRU");

    pConfig->SetPath(strConfigLocation);

    m_aSelectedComputerMRU.Clear();
    bKeepEnumerating = pConfig->GetFirstEntry(strBuffer, iIndex);
    while (bKeepEnumerating) {
        pConfig->Read(strBuffer, &strValue);

        m_aSelectedComputerMRU.Add(strValue);
        bKeepEnumerating = pConfig->GetNextEntry(strBuffer, iIndex);
    }

    pConfig->SetPath(strPreviousLocation);


    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::RestoreState - Function End"));
    return true;
}


#ifdef __WXMAC__
bool CBOINCBaseFrame::Show(bool show) {
    ProcessSerialNumber psn;

    GetCurrentProcess(&psn);
    if (show) {
        SetFrontProcess(&psn);  // Shows process if hidden
    } else {
//        GetWindowDimensions();
        if (wxGetApp().GetCurrentGUISelection() == m_iWindowType)
            if (IsProcessVisible(&psn))
                ShowHideProcess(&psn, false);
    }
    
    return wxFrame::Show(show);
}

#endif // __WXMAC__


void CFrameAlertEvent::ProcessResponse(const int response) const {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
   
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if ((AlertProcessResponse == m_alert_event_type) && (wxYES == response)) {
        pDoc->Reconnect();
    }
}
