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

#ifndef MAINDOCUMENT_H
#define MAINDOCUMENT_H

#include <map>
#include <string>
#include <vector>

#include <wx/datetime.h>
#include <wx/object.h>

#include "common_defs.h"
#include "gui_rpc_client.h"
#include "hostinfo.h"

typedef struct {
    int slot;
    std::string project_url;
    std::string name;
#ifdef _WIN32
    HANDLE pid;
#else
    int pid;
#endif
} RUNNING_GFX_APP;

typedef std::map<std::string, PROJECT*> projects_map;
typedef std::vector<std::string> project_names_vec;

extern bool g_use_sandbox;

class CMainDocument;
class CBOINCClientManager;

class CNetworkConnection : public wxObject {
public:
    CNetworkConnection(CMainDocument* pDocument);
    ~CNetworkConnection();

    void           Poll();
    void           FireReconnectEvent() { m_bConnectEvent = true; };
    void           ForceDisconnect() { m_bForceReconnect = false; m_bReconnectOnError = false; m_bConnectEvent = false; SetStateDisconnected(); };
    void           ForceReconnect() { m_bForceReconnect = true; SetStateDisconnected(); };
    int            FrameShutdownDetected();
    int            GetConnectedComputerName(wxString& strMachine);
    int            GetConnectedComputerVersion(wxString& strVersion);
    int            GetConnectingComputerName(wxString& strMachine);
    bool           IsComputerNameLocal(const wxString& strMachine);
    int            SetComputer(const wxChar* szComputer, const int iPort,
                               const std::string& szPassword,
                               const bool bUseDefaultPassword);
    void           SetStateError();
    void           SetStateErrorAuthentication();
    void           SetStateReconnecting();
    void           SetStateSuccess(const wxString& strComputer, const std::string& strComputerPassword);
    void           SetStateDisconnected();
    bool           IsConnectEventSignaled() { return m_bConnectEvent; };
    bool           IsConnected() { return m_bConnected; };
    bool           IsReconnecting() { return m_bReconnecting; };
    
private:
    CMainDocument* m_pDocument;
    bool           m_bFrameShutdownDetected;
    bool           m_bConnectEvent;
    bool           m_bForceReconnect;
    bool           m_bReconnectOnError;
    bool           m_bConnected;
    bool           m_bReconnecting;
    bool           m_bUseDefaultPassword;
    bool           m_bUsedDefaultPassword;
    int            m_iReadGUIRPCAuthFailure;
    bool           m_bNewConnection;
    wxString       m_strNewComputerName;
    std::string    m_strNewComputerPassword;
    wxString       m_strConnectedComputerName;
    std::string    m_strConnectedComputerPassword;
    wxString       m_strConnectedComputerVersion;
    int m_iPort;
};


class CMainDocument : public wxObject {
    DECLARE_DYNAMIC_CLASS(CMainDocument)

public:
    CMainDocument();
    ~CMainDocument();

    //
    // Global
    //
private:

    wxDateTime                  m_dtCachedCCStatusTimestamp;
    bool                        m_bClientStartCheckCompleted;


public:
    int                         OnInit();
    int                         OnExit();
    int                         OnPoll();

    int                         OnRefreshState();
    int                         CachedStateUpdate();
    int                         ResetState();

    int                         Connect(
                                    const wxChar* szComputer,
                                    const int iPort,
                                    const std::string& szComputerPassword = "",
                                    const bool bDisconnect = FALSE,
                                    const bool bUseDefaultPassword = FALSE
                                );
    int                         Reconnect();

    int                         CachedStateLock();
    int                         CachedStateUnlock();

    void                        ForceDisconnect();
    int                         FrameShutdownDetected();
    int                         CoreClientQuit();

    int                         GetConnectedComputerName(wxString& strMachine);
    int                         GetConnectedComputerVersion(wxString& strVersion);
    bool                        IsComputerNameLocal(const wxString strMachine);
    bool                        IsLocalClient();
    bool                        IsConnected();
    bool                        IsReconnecting();

    int                         GetCoreClientStatus(CC_STATUS&, bool bForce = false);
    int                         SetActivityRunMode(int iMode, int iTimeout);
    int                         SetNetworkRunMode(int iMode, int iTimeout);

    int                         ForceCacheUpdate();
    int                         RunBenchmarks();

    bool                        IsUserAuthorized();

    CNetworkConnection*         m_pNetworkConnection;
    CBOINCClientManager*        m_pClientManager;
    RPC_CLIENT                  rpc;
    CC_STATE                    state;
    CC_STATUS                   status;
    HOST_INFO                   host;
    wxDateTime                  m_dtCachedStateTimestamp;


    //
    // Project Tab
    //
private:
    int                         CachedProjectStatusUpdate();
    wxDateTime                  m_dtProjecStatusTimestamp;

public:
    PROJECT*                    project(size_t i);
    //PROJECT*                    project(const wxString& masterUrl);
    
    /// Return a std::map containing all projects this client is attached to.
    projects_map                GetProjectsMap() const;
    
    /// Returns a sorted vector containing the names of all projects this client is attached to.
    project_names_vec           GetProjectNames() const;
    
    float                       m_fProjectTotalResourceShare;

    size_t                      GetProjectCount();

    int                         ProjectNoMoreWork(size_t iIndex);
    int                         ProjectAllowMoreWork(size_t iIndex);
    int                         ProjectAttach(const wxString& strURL, const wxString& strAccountKey);
    int                         ProjectDetach(size_t iIndex);
    int                         ProjectUpdate(size_t iIndex);
    int                         ProjectReset(size_t iIndex);
    int                         ProjectSuspend(size_t iIndex);
    int                         ProjectResume(size_t iIndex);


    //
    // Work Tab
    //
private:
    int                         CachedResultsStatusUpdate();
    wxDateTime                  m_dtResultsTimestamp;
    wxDateTime                  m_dtKillInactiveGfxTimestamp;
    std::vector<RUNNING_GFX_APP> m_running_gfx_apps;
    RUNNING_GFX_APP*            GetRunningGraphicsApp(const RESULT* result, int slot);
    void                        KillAllRunningGraphicsApps();
    void                        KillInactiveGraphicsApps();
#ifdef _WIN32
    void                        KillGraphicsApp(HANDLE pid);
#else
    void                        KillGraphicsApp(int tpid);
#endif

public:
    RESULTS                     results;
    RESULT*                     result(size_t i);
    RESULT*                     result(const wxString& name, const wxString& project_url);

    size_t                      GetWorkCount();

    int                         WorkSuspend(
                                    const std::string& strProjectURL,
                                    const std::string& strName
                                );
    int                         WorkResume(
                                    const std::string& strProjectURL,
                                    const std::string& strName
                                );
    int                         WorkShowGraphics(const RESULT* result);
    int                         WorkAbort(
                                    const std::string& strProjectURL,
                                    const std::string& strName
                                );
    CC_STATE*                   GetState() { return &state; };


    //
    // Messages Tab
    //
private:


public:
    MESSAGES                    messages;
    MESSAGE*                    message(size_t i);
    int                         CachedMessageUpdate();

    size_t                      GetMessageCount();

    int                         ResetMessageState();

    int                         m_iMessageSequenceNumber;


    //
    // Transfers Tab
    //
private:
    int                         CachedFileTransfersUpdate();
    wxDateTime                  m_dtFileTransfersTimestamp;

public:
    FILE_TRANSFERS              ft;
    FILE_TRANSFER*              file_transfer(size_t i);
    FILE_TRANSFER*              file_transfer(const wxString& fileName, const wxString& project_url);

    size_t                      GetTransferCount();

    int                         TransferRetryNow(size_t iIndex);
    int                         TransferRetryNow(const wxString& fileName, const wxString& project_url);
    int                         TransferAbort(size_t iIndex);
    int                         TransferAbort(const wxString& fileName, const wxString& project_url);


    //
    // Disk Tab
    //
private:
    wxDateTime                  m_dtDiskUsageTimestamp;

public:
    DISK_USAGE                  disk_usage;
    PROJECT*                    DiskUsageProject(unsigned int);
    int                         CachedDiskUsageUpdate();

    //
    // Statistics Tab
    //
private:
    int                         CachedStatisticsStatusUpdate();
    wxDateTime                  m_dtStatisticsStatusTimestamp;

public:
    PROJECTS                    statistics_status;
    PROJECT*                    statistic(unsigned int);

    size_t                      GetStatisticsCount();
    

    //
    // Proxy Configuration
    //
private:

public:
    GR_PROXY_INFO               proxy_info;
    int                         GetProxyConfiguration();
    int                         SetProxyConfiguration();


    //
    // Simple GUI Updates
    //
private:
    wxDateTime                  m_dtCachedSimpleGUITimestamp;
    int                         CachedSimpleGUIUpdate();

public:
    size_t                      GetSimpleGUIWorkCount();

};

#ifdef SANDBOX
#define BOINC_MASTER_GROUP_NAME "boinc_master"
#endif

#endif // MAINDOCUMENT_H
