// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
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

#ifndef _BOINCDIALUPMANAGER_H_
#define _BOINCDIALUPMANAGER_H_

// I need to include defs.h first or wxUSE_DIALUP_MANAGER won't be set,
// and dialup.h does nothing. I think this is a wx bug;
// dialup.h should be including defs.h

#include <wx/defs.h>
#include <wx/dialup.h>
#include <wx/datetime.h>

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

