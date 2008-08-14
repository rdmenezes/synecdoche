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

#ifndef _BOINCCLIENTMANAGER_H_
#define _BOINCCLIENTMANAGER_H_


class CBOINCClientManager : public wxObject
{
public:

    CBOINCClientManager();
    ~CBOINCClientManager();

    bool                AutoRestart();

    bool                IsSystemBooting();
    int                 IsBOINCConfiguredAsDaemon();
    bool                WasBOINCStartedByManager() { return m_bBOINCStartedByManager; };

    bool                IsBOINCCoreRunning();
    bool                StartupBOINCCore();
    void                ShutdownBOINCCore();
#ifdef __WXMAC__
    bool                ProcessExists(pid_t thePID);
    static OSErr        QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon );
#endif

protected:

    bool                m_bBOINCStartedByManager;
    int                 m_lBOINCCoreProcessId;

#ifdef __WXMSW__
    HANDLE              m_hBOINCCoreProcess;
#endif

};


#endif

