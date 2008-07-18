// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _BOINCDIALUPMANAGER_H_
#define _BOINCDIALUPMANAGER_H_


class CBOINCDialUpManager : public wxObject
{
public:

    CBOINCDialUpManager();
    ~CBOINCDialUpManager();

    bool IsOk();
    size_t GetISPNames(wxArrayString& names);

    void OnPoll();

    int NotifyUserNeedConnection(bool bNotificationOnly);

    int Connect();
    int ConnectionSucceeded();
    int ConnectionFailed();

    int NetworkAvailable();

    int Disconnect();

    void ResetReminderTimers();

protected:
    wxDialUpManager* m_pDialupManager;
    wxDateTime       m_dtLastDialupAlertSent;
    wxDateTime       m_dtLastDialupRequest;
    wxDateTime       m_dtDialupConnectionTimeout;
    bool             m_bSetConnectionTimer;
    bool             m_bNotifyConnectionAvailable;
    bool             m_bConnectedSuccessfully;
    bool             m_bResetTimers;
    bool             m_bWasDialing;
    int              m_iNetworkStatus;
    int              m_iConnectAttemptRetVal;

    wxString         m_strDialogTitle;
};


#endif

