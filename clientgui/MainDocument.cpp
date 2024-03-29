// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 David Barnard, Peter Kortschack
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

#include "MainDocument.h"

#include "stdwx.h"

#include "error_numbers.h"
#include "str_util.h"
#include "util.h"
#ifdef _WIN32
#include "proc_control.h"
#endif

#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "BOINCClientManager.h"

#ifndef _WIN32
#include <sys/wait.h>
#endif

#ifdef SANDBOX
#include <grp.h>
#endif

bool g_use_sandbox = false;

CNetworkConnection::CNetworkConnection(CMainDocument* pDocument) :
    wxObject() {
    m_pDocument = pDocument;

    m_strConnectedComputerName = wxEmptyString;
    m_strConnectedComputerPassword.clear();
    m_strNewComputerName = wxEmptyString;
    m_strNewComputerPassword.clear();
    m_bFrameShutdownDetected = false;
    m_bConnectEvent = false;
    m_bConnected = false;
    m_bReconnecting = false;
    m_bForceReconnect = false;
    m_bReconnectOnError = false;
    m_bNewConnection = false;
    m_bUsedDefaultPassword = false;
    m_iPort = GUI_RPC_PORT,
    m_iReadGUIRPCAuthFailure = 0;
}


CNetworkConnection::~CNetworkConnection() {
}

void CNetworkConnection::Poll() {
    int retval;
    wxString strComputer = wxEmptyString;

    if (IsReconnecting()) {
        wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - Reconnection Detected"));
        retval = m_pDocument->rpc.init_poll();
        if (!retval) {
            wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - init_poll() returned ERR_CONNECT, now authorizing..."));

            // Wait until we can establish a connection to the core client before reading
            //   the password so that the client has time to create one when it needs to.
            if (m_bUseDefaultPassword) {
                m_bUseDefaultPassword = FALSE;
                m_bUsedDefaultPassword = true;

                try {
                    m_strNewComputerPassword = read_gui_rpc_password();
                    m_iReadGUIRPCAuthFailure = 0;
                } catch (...) {
                    m_iReadGUIRPCAuthFailure = ERR_FOPEN;
                }
            }

            retval = m_pDocument->rpc.authorize(m_strNewComputerPassword.c_str());
            if (!retval) {
                wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - Connection Success"));
                SetStateSuccess(m_strNewComputerName, m_strNewComputerPassword);
            } else if (ERR_AUTHENTICATOR == retval) {
                wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - RPC Authorization - ERR_AUTHENTICATOR"));
                SetStateErrorAuthentication();
            } else {
                wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - RPC Authorization Failed '%d'"), retval);
                SetStateError();
            }
            m_bUsedDefaultPassword = false;
        } else if (ERR_RETRY != retval) {
            wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - RPC Connection Failed '%d'"), retval);
            SetStateError();
        }
    } else if (IsConnectEventSignaled() || m_bReconnectOnError) {
        if ((m_bForceReconnect) || (!IsConnected() && m_bReconnectOnError)) {
            wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - Resetting Document State"));
            m_pDocument->ResetState();
            wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - Setting connection state to reconnecting"));
            SetStateReconnecting();
        }

        if (!IsConnected()) {
            // determine computer name and password to use.
            // NOTE: Initial connection case.
            if (!m_strNewComputerName.empty()) {
                strComputer = m_strNewComputerName;
            } else {
                // NOTE: Reconnect after a disconnect case.
                //       Values are stored after the first successful connect to the host.
                //       See: SetStateSuccess()
                if (!m_strConnectedComputerName.empty()) {
                    strComputer = m_strConnectedComputerName;
                }
            }

            // a host value of NULL is special cased as binding to the localhost and
            //   if we are connecting to the localhost we need to retry the connection
            //   for awhile so that the users can respond to firewall prompts.
            //
            // use a timeout of 60 seconds so that slow machines do not get a
            //   timeout event right after boot-up.
            //
            if (IsComputerNameLocal(strComputer)) {
                retval = m_pDocument->rpc.init_asynch(NULL, 60.0, true, m_iPort);
            } else {
                retval = m_pDocument->rpc.init_asynch(strComputer.mb_str(), 60.0, false, m_iPort);
            }

            if (!retval) {
                wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - RPC Initialization Called"));
            } else {
                wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - RPC Initialization Failed '%d'"), retval);
                SetStateError();
            }
        }
    }
}


int CNetworkConnection::FrameShutdownDetected() {
    m_bFrameShutdownDetected = true;
    return 0;
}

int CNetworkConnection::GetConnectedComputerName(wxString& strMachine) {
    strMachine = m_strConnectedComputerName;
    return 0;
}


int CNetworkConnection::GetConnectedComputerVersion(wxString& strVersion) {
    strVersion = m_strConnectedComputerVersion;
    return 0;
}


int CNetworkConnection::GetConnectingComputerName(wxString& strMachine) {
    strMachine = m_strNewComputerName;
    return 0;
}


bool CNetworkConnection::IsComputerNameLocal(const wxString& strMachine) {
    static wxString strHostName = wxEmptyString;
    static wxString strFullHostName = wxEmptyString;

    if (strHostName.empty()) {
        strHostName = ::wxGetHostName().Lower();
    }
    if (strFullHostName.empty()) {
        strFullHostName = ::wxGetFullHostName().Lower();
    }

    if (strMachine.empty()) {
        return true;
    } else if (wxT("localhost") == strMachine.Lower()) {
        return true;
    } else if (wxT("localhost.localdomain") == strMachine.Lower()) {
        return true;
    } else if (strHostName == strMachine.Lower()) {
        return true;
    } else if (strFullHostName == strMachine.Lower()) {
        return true;
    }
    return false;
}


int CNetworkConnection::SetComputer(const wxChar* szComputer, const int iPort,
                                    const std::string& szPassword,
                                    const bool bUseDefaultPassword) {
    m_strNewComputerName.Empty();
    m_strNewComputerPassword.clear();
    m_bUseDefaultPassword = FALSE;

    m_bNewConnection = true;
    m_strNewComputerName = szComputer;
    m_iPort = iPort;
    m_strNewComputerPassword = szPassword;
    m_bUseDefaultPassword = bUseDefaultPassword;
    return 0;
}


void CNetworkConnection::SetStateErrorAuthentication() {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        m_bConnected = false;
        m_bReconnecting = false;
        m_bReconnectOnError = false;

        m_bConnectEvent = false;

        pFrame->ShowConnectionBadPasswordAlert(m_bUsedDefaultPassword, m_iReadGUIRPCAuthFailure);
    }
}


void CNetworkConnection::SetStateError() {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        m_bConnected = false;
        m_bReconnecting = false;
        m_bReconnectOnError = false;

        m_bConnectEvent = false;

        pFrame->ShowConnectionFailedAlert();
    }
}


void CNetworkConnection::SetStateReconnecting() {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        m_bConnected = false;
        m_bReconnectOnError = false;
        m_bForceReconnect = false;
        m_bReconnecting = true;
        if (!m_bNewConnection) {
            m_strNewComputerName = m_strConnectedComputerName;
            m_strNewComputerPassword = m_strConnectedComputerPassword;
        }
        pFrame->FireRefreshView();
    }
}


void CNetworkConnection::SetStateSuccess(const wxString& strComputer, const std::string& strComputerPassword) {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        m_bConnected = true;
        m_bReconnecting = false;
        m_bReconnectOnError = true;
        m_strConnectedComputerName = strComputer;
        m_strConnectedComputerPassword = strComputerPassword;
        m_strNewComputerName = wxEmptyString;
        m_strNewComputerPassword.clear();
        m_bNewConnection = false;

        // Get the version of the client and cache it
        VERSION_INFO vi;
        m_pDocument->rpc.exchange_versions(vi);
        m_strConnectedComputerVersion.Printf(
            wxT("%d.%d.%d"),
            vi.major, vi.minor, vi.release
        );

        m_bConnectEvent = false;

        pFrame->FireConnect();
    }
}


void CNetworkConnection::SetStateDisconnected() {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        m_bConnected = false;
        m_bReconnecting = false;
    }
}


IMPLEMENT_DYNAMIC_CLASS(CMainDocument, wxObject)


CMainDocument::CMainDocument() {

#ifdef __WIN32__
    int retval;
    WSADATA wsdata;

    retval = WSAStartup(MAKEWORD(1, 1), &wsdata);
    if (retval) {
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CMainDocument - Winsock Initialization Failure '%d'"), retval);
    }
#endif

    m_bClientStartCheckCompleted = false;

    m_fProjectTotalResourceShare = 0.0;

    m_iMessageSequenceNumber = 0;

    m_dtCachedStateTimestamp = wxDateTime((time_t)0);
    m_dtCachedCCStatusTimestamp = wxDateTime((time_t)0);
    m_dtProjecStatusTimestamp = wxDateTime((time_t)0);
    m_dtResultsTimestamp = wxDateTime((time_t)0);
    m_dtKillInactiveGfxTimestamp = wxDateTime((time_t)0);
    m_dtFileTransfersTimestamp = wxDateTime((time_t)0);
    m_dtDiskUsageTimestamp = wxDateTime((time_t)0);
    m_dtStatisticsStatusTimestamp = wxDateTime((time_t)0);
    m_dtCachedSimpleGUITimestamp = wxDateTime((time_t)0);
}


CMainDocument::~CMainDocument() {
    KillAllRunningGraphicsApps();
#ifdef __WIN32__
    WSACleanup();
#endif
}


int CMainDocument::OnInit() {
    int iRetVal = -1;

    m_pNetworkConnection = new CNetworkConnection(this);
    wxASSERT(m_pNetworkConnection);

    m_pClientManager = new CBOINCClientManager();
    wxASSERT(m_pClientManager);

    return iRetVal;
}


int CMainDocument::OnExit() {
    int iRetVal = 0;

    if (m_pClientManager) {
        m_pClientManager->ShutdownBOINCCore();

        delete m_pClientManager;
        m_pClientManager = NULL;
    }

    if (m_pNetworkConnection) {
        delete m_pNetworkConnection;
        m_pNetworkConnection = NULL;
    }

    return iRetVal;
}


int CMainDocument::OnPoll() {
    int iRetVal = 0;

    wxASSERT(wxDynamicCast(m_pClientManager, CBOINCClientManager));
    wxASSERT(wxDynamicCast(m_pNetworkConnection, CNetworkConnection));

    if (!m_bClientStartCheckCompleted) {
        m_bClientStartCheckCompleted = true;

        CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));

        pFrame->UpdateStatusText(_("Starting client services; please wait..."));

        if (m_pClientManager->StartupBOINCCore()) {
            Connect(wxT("localhost"), GUI_RPC_PORT, "", TRUE, TRUE);
        } else {
            m_pNetworkConnection->ForceDisconnect();
            pFrame->ShowDaemonStartFailedAlert();
        }

        pFrame->UpdateStatusText(wxEmptyString);
    }

    // Check connection state, connect if needed.
    m_pNetworkConnection->Poll();

    // Every 10 seconds, kill any running graphics apps 
    // whose associated worker tasks are no longer running
    wxTimeSpan ts(wxDateTime::Now() - m_dtKillInactiveGfxTimestamp);
    if (ts.GetSeconds() > 10) {
        m_dtKillInactiveGfxTimestamp = wxDateTime::Now();
        KillInactiveGraphicsApps();
    }

    return iRetVal;
}


int CMainDocument::OnRefreshState() {
    if (IsConnected()) {
        CachedStateUpdate();
    }

    return 0;
}


int CMainDocument::CachedStateUpdate() {
    //wxLogTrace(wxT("Function Start/End"), wxT("CMainDocument::CachedStateUpdate - Function Begin"));

    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    int     retval = 0;

    wxTimeSpan ts(wxDateTime::Now() - m_dtCachedStateTimestamp);
    if (IsConnected() && (ts.GetSeconds() > 3600)) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        pFrame->UpdateStatusText(_("Retrieving system state; please wait..."));

        m_dtCachedStateTimestamp = wxDateTime::Now();
        retval = rpc.get_state(state);
        if (retval) {
            wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedStateUpdate - Get State Failed '%d'"), retval);
            m_pNetworkConnection->SetStateDisconnected();
        }
        pFrame->UpdateStatusText(_("Retrieving host information; please wait..."));

        retval = rpc.get_host_info(host);
        if (retval) {
            wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedStateUpdate - Get Host Information Failed '%d'"), retval);
            m_pNetworkConnection->SetStateDisconnected();
        }

        pFrame->UpdateStatusText(wxEmptyString);
    }

    //wxLogTrace(wxT("Function Start/End"), wxT("CMainDocument::CachedStateUpdate - Function End"));
    return retval;
}


int CMainDocument::ResetState() {
    rpc.close();
    state.clear();
    host.clear_host_info();
    results.clear();
    ft.clear();
    statistics_status.clear();
    disk_usage.clear();
    proxy_info.clear();

    ForceCacheUpdate();
    return 0;
}


int CMainDocument::Connect(const wxChar* szComputer, int iPort, const std::string& szComputerPassword, const bool bDisconnect, const bool bUseDefaultPassword) {
    wxString strOldMachineName = wxEmptyString;

   GetConnectedComputerName(strOldMachineName);
   
   if (bDisconnect) {
        m_pNetworkConnection->ForceReconnect();
    }

    m_pNetworkConnection->SetComputer(szComputer, iPort, szComputerPassword, bUseDefaultPassword);
    m_pNetworkConnection->FireReconnectEvent();

    ResetMessageState();
    return 0;
}


int CMainDocument::Reconnect() {
    m_pNetworkConnection->ForceReconnect();
    m_pNetworkConnection->FireReconnectEvent();
    ResetMessageState();
    return 0;
}


/// Gets the last connected computer name, or the new name if reconnecting.
int CMainDocument::GetConnectedComputerName(wxString& strMachine) {
    if (IsReconnecting()) {
        m_pNetworkConnection->GetConnectingComputerName(strMachine);
    } else {
        m_pNetworkConnection->GetConnectedComputerName(strMachine);
    }
    return 0;
}


int CMainDocument::GetConnectedComputerVersion(wxString& strVersion) {
    m_pNetworkConnection->GetConnectedComputerVersion(strVersion);
    return 0;
}


bool CMainDocument::IsComputerNameLocal(const wxString strMachine) {
    return m_pNetworkConnection->IsComputerNameLocal(strMachine);
}


/// Checks whether the connected (or connecting) computer is local.
bool CMainDocument::IsLocalClient() {
    wxString strComputer;
    GetConnectedComputerName(strComputer);
    return m_pNetworkConnection->IsComputerNameLocal(strComputer);
}


bool CMainDocument::IsConnected() {
    return m_pNetworkConnection->IsConnected();
}


bool CMainDocument::IsReconnecting() {
    return m_pNetworkConnection->IsReconnecting();
}


void CMainDocument::ForceDisconnect() {
    return m_pNetworkConnection->ForceDisconnect();
}


int CMainDocument::FrameShutdownDetected() {
    return m_pNetworkConnection->FrameShutdownDetected();
}


int CMainDocument::GetCoreClientStatus(CC_STATUS& ccs, bool bForce) {
    wxString         strMachine = wxEmptyString;
    int              iRetVal = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtCachedCCStatusTimestamp);
        if ((ts.GetSeconds() > 0) || bForce) {
            m_dtCachedCCStatusTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_cc_status(ccs);
            if (0 == iRetVal) {
                status = ccs;
            } else {
                m_pNetworkConnection->SetStateDisconnected();
            }
        } else {
            ccs = status;
        }
    } else {
        iRetVal = -1;
    }

    return iRetVal;
}


int CMainDocument::SetActivityRunMode(int iMode, int iTimeout) {
    int       iRetVal = 0;
    CC_STATUS ccs;

    if (IsConnected()) {
        iRetVal = rpc.set_run_mode(iMode, iTimeout);
        if (0 == iRetVal) {
            if (RUN_MODE_RESTORE == iMode) {
                GetCoreClientStatus(ccs, true);
            } else {
                status.task_mode = iMode;
            }
        }
    }

    return iRetVal;
}


int CMainDocument::SetNetworkRunMode(int iMode, int iTimeout) {
    int       iRetVal = 0;
    CC_STATUS ccs;

    if (IsConnected()) {
        iRetVal = rpc.set_network_mode(iMode, iTimeout);
        if (0 == iRetVal) {
            if (RUN_MODE_RESTORE == iMode) {
                GetCoreClientStatus(ccs, true);
            } else {
                status.network_mode = iMode;
            }
        }
    }

    return iRetVal;
}


int CMainDocument::ForceCacheUpdate() {
    m_dtCachedStateTimestamp = wxDateTime((time_t)0);
    CachedStateUpdate();
    return 0;
}


int CMainDocument::RunBenchmarks() {
    return rpc.run_benchmarks();
}


int CMainDocument::CoreClientQuit() {
    return rpc.quit();
}


bool CMainDocument::IsUserAuthorized() {
#ifndef _WIN32
#ifdef SANDBOX
    static bool         sIsAuthorized = false;
    group               *grp;
    gid_t               rgid, boinc_master_gid;
    char                *userName, *groupMember;
    int                 i;

    if (g_use_sandbox) {
        if (sIsAuthorized)
            return true;            // We already checked and OK'd current user

        grp = getgrnam(BOINC_MASTER_GROUP_NAME);
        if (grp) {
            boinc_master_gid = grp->gr_gid;

            rgid = getgid();
            if (rgid == boinc_master_gid) {
                sIsAuthorized = true;           // User's primary group is boinc_master
                return true;
            }

            userName = getlogin();
            if (userName) {
                for (i=0; ; i++) {              // Step through all users in group boinc_master
                    groupMember = grp->gr_mem[i];
                    if (groupMember == NULL)
                        break;                  // User is not a member of group boinc_master
                    if (strcmp(userName, groupMember) == 0) {
                        sIsAuthorized = true;   // User is a member of group boinc_master
                        return true;
                    }
                }       // for (i)
            }           // if (userName)
        }               // if grp

#ifdef __WXMAC__
        if (Mac_Authorize()) {          // Run Mac Authentication dialog
            sIsAuthorized = true;       // Authenticated by password
            return true;
        }
#endif      // __WXMAC__

        return false;

    }       // if (g_use_sandbox)
#endif      // SANDBOX
#endif      // #ifndef _WIN32

    return true;
}


int CMainDocument::CachedProjectStatusUpdate() {
    int     iRetVal = 0;
    int     i = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtProjecStatusTimestamp);
        if (ts.GetSeconds() > 0) {
            m_dtProjecStatusTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_project_status(state);
            if (iRetVal) {
                wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedProjectStatusUpdate - Get Project Status Failed '%d'"), iRetVal);
                ForceCacheUpdate();
            }

            m_fProjectTotalResourceShare = 0.0;
            for (i=0; i < (long)state.projects.size(); i++) {
                m_fProjectTotalResourceShare += state.projects.at(i)->resource_share;
            }
        }
    } else {
        iRetVal = -1;
    }

    return iRetVal;
}


PROJECT* CMainDocument::project(size_t i) {
    try {
        if (!state.projects.empty())
            return state.projects.at(i);
    }
    catch (std::out_of_range e) {
        return NULL;
    }

    return NULL;
}
#if 0
PROJECT* CMainDocument::project(const wxString& masterUrl) {
    for (unsigned int i=0; i< state.projects.size(); i++) {
        PROJECT* tp = state.projects[i];
        wxString t2(tp->master_url.c_str(), wxConvUTF8);
        if(t2.IsSameAs(masterUrl)) return tp;
    }
    return NULL;
}
#endif
/// Return a std::map containing all projects this client is attached to.
/// The master URL is used as key in this map.
///
/// \return A map containing all projects this client is attached to.
projects_map CMainDocument::GetProjectsMap() const {
    projects_map ret_val;
    for (std::vector<PROJECT*>::const_iterator p = state.projects.begin(); p != state.projects.end(); ++p) {
        std::string key = (*p)->master_url;
        canonicalize_master_url(key);
        ret_val.insert(std::make_pair(key, *p));
    }
    return ret_val;
}

/// Returns a sorted vector containing the names of all projects this client is attached to.
///
/// \return A sorted vector containing the names of all projects this client is attached to.
project_names_vec CMainDocument::GetProjectNames() const {
    project_names_vec ret_val;
    ret_val.reserve(state.projects.size());
    for (std::vector<PROJECT*>::const_iterator p = state.projects.begin(); p != state.projects.end(); ++p) {
        ret_val.push_back((*p)->project_name);
    }
    std::sort(ret_val.begin(), ret_val.end(), NoCaseLess);
    return ret_val;
}

size_t CMainDocument::GetProjectCount() {
    CachedProjectStatusUpdate();
    CachedStateUpdate();

    return state.projects.size();
}

int CMainDocument::ProjectDetach(size_t iIndex) {
    int iRetVal = -1;
    PROJECT* pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "detach");

    return iRetVal;
}

int CMainDocument::ProjectUpdate(size_t iIndex) {
    int iRetVal = -1;
    PROJECT* pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "update");

    return iRetVal;
}

int CMainDocument::ProjectReset(size_t iIndex) {
    int iRetVal = -1;
    PROJECT* pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "reset");

    return iRetVal;
}

int CMainDocument::ProjectSuspend(size_t iIndex) {
    int iRetVal = -1;
    PROJECT* pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "suspend");

    return iRetVal;
}

int CMainDocument::ProjectResume(size_t iIndex) {
    int iRetVal = -1;
    PROJECT* pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "resume");

    return iRetVal;
}

int CMainDocument::ProjectNoMoreWork(size_t iIndex) {
    int iRetVal = -1;
    PROJECT* pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "nomorework");

    return iRetVal;
}

int CMainDocument::ProjectAllowMoreWork(size_t iIndex) {
    int iRetVal = -1;
    PROJECT* pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "allowmorework");

    return iRetVal;
}

int CMainDocument::CachedResultsStatusUpdate() {
    int     iRetVal = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtResultsTimestamp);
        if (ts.GetSeconds() > 0) {
            m_dtResultsTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_results(results);
            if (iRetVal) {
                wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedResultsStatusUpdate - Get Result Status Failed '%d'"), iRetVal);
                ForceCacheUpdate();
            }
        }
    } else {
        iRetVal = -1;
    }

    return iRetVal;
}


RESULT* CMainDocument::result(size_t i) {
    RESULT* pResult = NULL;

    try {
        if (!results.results.empty())
            pResult = results.results.at(i);
    }
    catch (std::out_of_range e) {
        pResult = NULL;
    }

    return pResult;
}

/* get the result not by index, but by name */
RESULT* CMainDocument::result(const wxString& name, const wxString& project_url) {
    RESULT* pResult = NULL;

    try {
        if (!results.results.empty()) {
            //iterating over the vector and find the right result
            for(unsigned int i=0; i< results.results.size();i++) {
                RESULT* tResult = results.results.at(i);
                wxString resname(tResult->name.c_str(),wxConvUTF8);
                if(resname.IsSameAs(name)){
                    wxString resurl(tResult->project_url.c_str(),wxConvUTF8);
                    if(resurl.IsSameAs(project_url)){
                        pResult = tResult;
                        break;
                    }
                }
            }
        }
    }
    catch (std::out_of_range e) {
        pResult = NULL;
    }

    return pResult;
}

size_t CMainDocument::GetWorkCount() {
    CachedResultsStatusUpdate();
    CachedStateUpdate();

    return results.results.size();
}


int CMainDocument::WorkSuspend(const std::string& strProjectURL, const std::string& strName) {
    int iRetVal = 0;

    RESULT* pStateResult = state.lookup_result(strProjectURL, strName);
    if (pStateResult) {
        iRetVal = rpc.result_op((*pStateResult), "suspend");
    } else {
        ForceCacheUpdate();
    }

    return iRetVal;
}


int CMainDocument::WorkResume(const std::string& strProjectURL, const std::string& strName) {
    int iRetVal = 0;

    RESULT* pStateResult = state.lookup_result(strProjectURL, strName);
    if (pStateResult) {
        iRetVal = rpc.result_op((*pStateResult), "resume");
    } else {
        ForceCacheUpdate();
    }

    return iRetVal;
}


/// If the graphics application for the current task is already 
/// running, return a pointer to its RUNNING_GFX_APP struct.
RUNNING_GFX_APP* CMainDocument::GetRunningGraphicsApp(const RESULT* result, int slot)
{
    bool exited = false;
    std::vector<RUNNING_GFX_APP>::iterator gfx_app_iter;
    
    for( gfx_app_iter = m_running_gfx_apps.begin(); 
        gfx_app_iter != m_running_gfx_apps.end(); 
        gfx_app_iter++
    ) {
         if ( (slot >= 0) && ((*gfx_app_iter).slot != slot) ) continue;

#ifdef _WIN32
        unsigned long exit_code;
        if (GetExitCodeProcess((*gfx_app_iter).pid, &exit_code)) {
            if (exit_code != STILL_ACTIVE) {
                exited = true;
            }
        }
#else
        if (waitpid((*gfx_app_iter).pid, 0, WNOHANG) == (*gfx_app_iter).pid) {
            exited = true;
        }
#endif
        if (! exited) {
            if ( (result->name == (*gfx_app_iter).name) &&
                (result->project_url == (*gfx_app_iter).project_url) ) {
                return &(*gfx_app_iter);
            }
    
            // Graphics app is still running but the slot now has a different task
            KillGraphicsApp((*gfx_app_iter).pid);
        }

        // Either the graphics app had already exited or we just killed it
        (*gfx_app_iter).name.clear();
        (*gfx_app_iter).project_url.clear();
        m_running_gfx_apps.erase(gfx_app_iter);
        return NULL;
    }
    return NULL;
}


/// Kill any running graphics apps whose worker tasks aren't running
void CMainDocument::KillInactiveGraphicsApps()
{
/*
    std::vector<RUNNING_GFX_APP>::iterator gfx_app_iter;
    unsigned int i;
    bool bStillRunning;

    if (m_running_gfx_apps.size() <= 0) return;
    
    // If none of the Tasks displays are visible, we need to update 
    // the results vector.  This call does nothing if recently updated
    // by a call from CViewWork, CViewWorkGrid or CViewTabPage.
    CachedResultsStatusUpdate();
    
    gfx_app_iter = m_running_gfx_apps.begin();
    while (gfx_app_iter != m_running_gfx_apps.end()) {
        bStillRunning = false;
        
        for (i=0; i<results.results.size(); i++) {
            if ((results.results.at(i))->state != RESULT_FILES_DOWNLOADED) continue;
            if (!(results.results.at(i))->active_task) continue;
            if ((results.results.at(i))->scheduler_state != CPU_SCHED_SCHEDULED) continue;
            if ((results.results.at(i))->name != (*gfx_app_iter).name) continue;
            if ((results.results.at(i))->project_url != (*gfx_app_iter).project_url) continue;    
            bStillRunning =  true;
            break;
        }
        
        if (!bStillRunning) {
            KillGraphicsApp((*gfx_app_iter).pid);
            gfx_app_iter = m_running_gfx_apps.erase(gfx_app_iter);
        } else {
            gfx_app_iter++;
        }
    }
*/
}


void CMainDocument::KillAllRunningGraphicsApps()
{
    size_t i, n;
    std::vector<RUNNING_GFX_APP>::iterator gfx_app_iter;

    n = m_running_gfx_apps.size();
    for (i=0; i<n; i++) {
        gfx_app_iter = m_running_gfx_apps.begin(); 
        KillGraphicsApp((*gfx_app_iter).pid);
        (*gfx_app_iter).name.clear();
        (*gfx_app_iter).project_url.clear();
        m_running_gfx_apps.erase(gfx_app_iter);
    }
}


#ifdef _WIN32
void CMainDocument::KillGraphicsApp(HANDLE pid) {
    kill_program(pid);
}
#else
void CMainDocument::KillGraphicsApp(int pid) {
    const char* argv[6];
    char currentDir[1024];
    char thePIDbuf[10];
    int id, iRetVal;
    

    if (g_use_sandbox) {
        snprintf(thePIDbuf, sizeof(thePIDbuf), "%d", pid);
        argv[0] = "switcher";
        argv[1] = "/bin/kill";
        argv[2] =  "kill";
        argv[3] = "-KILL";
        argv[4] = thePIDbuf;
        argv[5] = 0;
    
        iRetVal = run_program(
            getcwd(currentDir, sizeof(currentDir)),
            "./switcher/switcher",
            5,
            argv,
            0,
            id
        );
    } else {
        kill_program(pid);
    }
}
#endif

int CMainDocument::WorkShowGraphics(const RESULT* result)
{
    int iRetVal = 0;
    
    if (!result->graphics_exec_path.empty()) {
        // V6 Graphics
        RUNNING_GFX_APP gfx_app;
        RUNNING_GFX_APP* previous_gfx_app;
        char *p;
        int slot;
#ifdef __WXMSW__
        HANDLE   id;
#else
        int      id;
#endif

        p = strrchr((char*)result->slot_path.c_str(), '/');
        if (!p) return ERR_INVALID_PARAM;
        slot = atoi(p+1);
        
        // See if we are already running the graphics application for this task
        previous_gfx_app = GetRunningGraphicsApp(result, slot);

#ifndef __WXMSW__
        const char* argv[4];

        if (previous_gfx_app) {
#ifdef __WXMAC__
        ProcessSerialNumber gfx_app_psn;
            // If this graphics app is already running,
            // just bring it to the front
            //
            if (!GetProcessForPID(previous_gfx_app->pid, &gfx_app_psn)) {
                SetFrontProcess(&gfx_app_psn);
            }
#endif
            // If graphics app is already running, don't launch a second instance
            //
            return 0;
        }
        argv[0] = "switcher";
        // For unknown reasons on Macs, the graphics application 
        // exits with "RegisterProcess failed (error = -50)" unless 
        // we pass its full path twice in the argument list to execv.
        //
        argv[1] = result->graphics_exec_path.c_str();
        argv[2] = result->graphics_exec_path.c_str();
        argv[3] = 0;
    
         if (g_use_sandbox) {
            iRetVal = run_program(
                result->slot_path.c_str(),
               "../../switcher/switcher",
                3,
                argv,
                0,
                id
            );
        } else {        
            iRetVal = run_program(
                result->slot_path.c_str(),
                result->graphics_exec_path.c_str(),
                1,
                &argv[2],
                0,
                id
            );
        }
#else
        char* argv[2];

        // If graphics app is already running, don't launch a second instance
        //
        if (previous_gfx_app) return 0;
        argv[0] =0;
        
        iRetVal = run_program(
            result->slot_path.c_str(),
            result->graphics_exec_path.c_str(),
            0,
            argv,
            0,
            id
        );
#endif

        if (!iRetVal) {
            gfx_app.slot = slot;
            gfx_app.project_url = result->project_url;
            gfx_app.name = result->name;
            gfx_app.pid = id;
            m_running_gfx_apps.push_back(gfx_app);
        }

    } else {
        // V5 and Older
        DISPLAY_INFO di;

        di.window_station = (const char*)wxGetApp().m_strDefaultWindowStation.mb_str();
        di.desktop = (const char*)wxGetApp().m_strDefaultDesktop.mb_str();
        di.display = (const char*)wxGetApp().m_strDefaultDisplay.mb_str();

        iRetVal = rpc.show_graphics(result->project_url.c_str(),
            result->name.c_str(), MODE_WINDOW, di);
    }

    return iRetVal;
}


int CMainDocument::WorkAbort(const std::string& strProjectURL, const std::string& strName) {
    int iRetVal = 0;

    RESULT* pStateResult = state.lookup_result(strProjectURL, strName);
    if (pStateResult) {
        iRetVal = rpc.result_op((*pStateResult), "abort");
    } else {
        ForceCacheUpdate();
    }

    return iRetVal;
}


int CMainDocument::CachedMessageUpdate() {
    int retval;
    static bool in_this_func = false;
    static bool was_connected = false;

    if (in_this_func) return 0;
    in_this_func = true;

    if (IsConnected()) {
        if (! was_connected) {
            ResetMessageState();
            was_connected = true;
        }
        retval = rpc.get_messages(m_iMessageSequenceNumber, messages);
        if (retval) {
            wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedMessageUpdate - Get Messages Failed '%d'"), retval);
            m_pNetworkConnection->SetStateDisconnected();
            goto done;
        }
        if (!messages.messages.empty()) {
            size_t last_ind = messages.messages.size()-1;
            m_iMessageSequenceNumber = messages.messages[last_ind]->seqno;
        }
    } else {
        was_connected = false;
    }
done:
    in_this_func = false;
    return 0;
}


MESSAGE* CMainDocument::message(size_t i) {
    MESSAGE* pMessage = NULL;

    try {
        if (!messages.messages.empty())
            pMessage = messages.messages.at(i);
    }
    catch (std::out_of_range e) {
        pMessage = NULL;
    }

    return pMessage;
}


size_t CMainDocument::GetMessageCount() {
    CachedMessageUpdate();
    CachedStateUpdate();

    return messages.messages.size();
}


int CMainDocument::ResetMessageState() {
    messages.clear();
    m_iMessageSequenceNumber = 0;
    return 0;
}


int CMainDocument::CachedFileTransfersUpdate() {
    int     iRetVal = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtFileTransfersTimestamp);
        if (ts.GetSeconds() > 0) {
            m_dtFileTransfersTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_file_transfers(ft);
            if (iRetVal) {
                wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedFileTransfersUpdate - Get File Transfers Failed '%d'"), iRetVal);
                ForceCacheUpdate();
            }
        }
    } else {
        iRetVal = -1;
    }

    return iRetVal;
}


FILE_TRANSFER* CMainDocument::file_transfer(size_t i) {
    FILE_TRANSFER* pFT = NULL;

    try {
        if (!ft.file_transfers.empty())
            pFT = ft.file_transfers.at(i);
    }
    catch (std::out_of_range e) {
        pFT = NULL;
    }

    return pFT;
}

FILE_TRANSFER* CMainDocument::file_transfer(const wxString& fileName, const wxString& project_url) {
    FILE_TRANSFER* pFT = NULL;

    try {
        if (!ft.file_transfers.empty()) {
            for(unsigned int i=0; i< ft.file_transfers.size();i++) {
                FILE_TRANSFER* tFT = ft.file_transfers.at(i);
                wxString fname(tFT->name.c_str(),wxConvUTF8);
                if(fname.IsSameAs(fileName)) {
                    wxString furl(tFT->project_url.c_str(),wxConvUTF8);
                    if(furl.IsSameAs(project_url)){
                        pFT = tFT;
                        break;
                    }
                }
            }
        }
    }
    catch (std::out_of_range e) {
        pFT = NULL;
    }

    return pFT;
}


size_t CMainDocument::GetTransferCount() {
    size_t iCount = 0;

    CachedFileTransfersUpdate();
    CachedStateUpdate();

    if (!ft.file_transfers.empty())
        iCount = ft.file_transfers.size();

    return iCount;
}


int CMainDocument::TransferRetryNow(size_t iIndex) {
    FILE_TRANSFER* pFT = NULL;
    int iRetVal = 0;

    pFT = file_transfer(iIndex);

    if (pFT)
        iRetVal = rpc.file_transfer_op((*pFT), "retry");

    return iRetVal;
}

int CMainDocument::TransferRetryNow(const wxString& fileName, const wxString& project_url) {
    FILE_TRANSFER* pFT = NULL;
    int iRetVal = 0;

    pFT = file_transfer(fileName, project_url);

    if (pFT)
        iRetVal = rpc.file_transfer_op((*pFT), "retry");

    return iRetVal;
}


int CMainDocument::TransferAbort(size_t iIndex) {
    FILE_TRANSFER* pFT = NULL;
    int iRetVal = 0;

    pFT = file_transfer(iIndex);

    if (pFT)
        iRetVal = rpc.file_transfer_op((*pFT), "abort");

    return iRetVal;
}

int CMainDocument::TransferAbort(const wxString& fileName, const wxString& project_url) {
    FILE_TRANSFER* pFT = NULL;
    int iRetVal = 0;

    pFT = file_transfer(fileName, project_url);

    if (pFT)
        iRetVal = rpc.file_transfer_op((*pFT), "abort");

    return iRetVal;
}


int CMainDocument::CachedDiskUsageUpdate() {
    int     iRetVal = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtDiskUsageTimestamp);

        // don't get disk usage more than once per minute
                // unless we just connected to a client
        //
        if ((ts.GetSeconds() > 60) || disk_usage.projects.empty()) {
            m_dtDiskUsageTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_disk_usage(disk_usage);
            if (iRetVal) {
                wxLogTrace(wxT("Function Status"), wxT("Get Disk Usage Failed '%d'"), iRetVal);
                ForceCacheUpdate();
            }
        }
    } else {
        iRetVal = -1;
    }

    return iRetVal;
}


PROJECT* CMainDocument::DiskUsageProject(unsigned int i) {
    PROJECT* pProject = NULL;

    try {
        if (!disk_usage.projects.empty()) {
            pProject = disk_usage.projects.at(i);
        }
    }
    catch (std::out_of_range e) {
        pProject = NULL;
    }

    return pProject;
}

int CMainDocument::CachedStatisticsStatusUpdate() {
    int     iRetVal = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtStatisticsStatusTimestamp);
        if ((ts.GetSeconds() > 0) || statistics_status.projects.empty()) {
            m_dtStatisticsStatusTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_statistics(statistics_status);
            if (iRetVal) {
                wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedStatisticsStatusUpdate - Get Statistics Failed '%d'"), iRetVal);
                ForceCacheUpdate();
            }
        }
    } else {
        iRetVal = -1;
    }

    return iRetVal;
}


PROJECT* CMainDocument::statistic(unsigned int i) {
    PROJECT* pProject = NULL;


    try {
        if (!statistics_status.projects.empty())
            pProject = statistics_status.projects.at(i);
    }
    catch (std::out_of_range e) {
        pProject = NULL;
    }


    return pProject;
}


size_t CMainDocument::GetStatisticsCount() {
    CachedStatisticsStatusUpdate();
    CachedStateUpdate();

    return statistics_status.projects.size();
}


int CMainDocument::GetProxyConfiguration() {
    int     iRetVal = 0;
    wxString    strEmpty = wxEmptyString;

    iRetVal = rpc.get_proxy_settings(proxy_info);
    if (iRetVal) {
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::GetProxyInfo - Get Proxy Info Failed '%d'"), iRetVal);
    }

    return iRetVal;
}


int CMainDocument::SetProxyConfiguration() {
    int     iRetVal = 0;

    if (!proxy_info.http_user_name.empty() || !proxy_info.http_user_passwd.empty())
        proxy_info.use_http_authentication = true;

    proxy_info.socks_version = 4;
    if (!proxy_info.socks5_user_name.empty() || !proxy_info.socks5_user_passwd.empty())
        proxy_info.socks_version = 5;

    iRetVal = rpc.set_proxy_settings(proxy_info);
    if (iRetVal) {
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::SetProxyInfo - Set Proxy Info Failed '%d'"), iRetVal);
    }

    return iRetVal;
}


int CMainDocument::CachedSimpleGUIUpdate() {
    int     iRetVal = 0;
    int     i = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtCachedSimpleGUITimestamp);
        if (ts.GetSeconds() > 0) {
            m_dtCachedSimpleGUITimestamp = wxDateTime::Now();

            iRetVal = rpc.get_simple_gui_info(state, results);
            if (iRetVal) {
                wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedSimpleGUIUpdate - Get Simple GUI Failed '%d'"), iRetVal);
                ForceCacheUpdate();
            }

            m_fProjectTotalResourceShare = 0.0;
            for (i=0; i < (long)state.projects.size(); i++) {
                m_fProjectTotalResourceShare += state.projects.at(i)->resource_share;
            }
        }
    } else {
        iRetVal = -1;
    }

    return iRetVal;
}


size_t CMainDocument::GetSimpleGUIWorkCount() {
    size_t iCount = 0;
    size_t i = 0;

    CachedSimpleGUIUpdate();
    CachedStateUpdate();

    for(i=0; i<results.results.size(); i++) {
        if (results.results[i]->active_task) {
            iCount++;
        }
    }
    return iCount;
}
