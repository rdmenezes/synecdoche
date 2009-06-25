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

#ifndef BOINCTASKBAR_H
#define BOINCTASKBAR_H

#ifdef __WXMSW__
#include "msw/taskbarex.h"
#else
#define wxTaskBarIconEx         wxTaskBarIcon
#define wxTaskBarIconExEvent    wxTaskBarIconEvent
#endif

#include <vector>
#include <wx/datetime.h>
#include <wx/event.h>
#include <wx/taskbar.h>
#include <wx/icon.h>

class wxCommandEvent;
class wxCloseEvent;
class wxIdleEvent;
class wxMenu;
class wxTimer;
class wxTimerEvent;
class CMainDocument;
class CTaskbarEvent;
class RESULT;

class CTaskBarIcon : public wxTaskBarIconEx {
public:
    CTaskBarIcon(wxString title, wxIcon* icon, wxIcon* iconDisconnected, wxIcon* iconSnooze);
    ~CTaskBarIcon();

    void OnOpenWebsite(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSuspendResume(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnShutdown(wxTaskBarIconExEvent& event);

    void OnIdle(wxIdleEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnRefresh(wxTimerEvent& event);
    void OnReloadSkin(CTaskbarEvent& event);

    void OnMouseMove(wxTaskBarIconEvent& event);
    void OnLButtonDClick(wxTaskBarIconEvent& event);
    void OnContextMenu(wxTaskBarIconExEvent& event);
    void OnRButtonDown(wxTaskBarIconEvent& event);
    void OnRButtonUp(wxTaskBarIconEvent& event);

    void FireReloadSkin();

    wxMenu *BuildContextMenu();
    void AdjustMenuItems(wxMenu* menu);

#ifdef __APPLE__
    wxMenu *CreatePopupMenu();
    bool SetIcon(const wxIcon& icon);
#endif

#ifndef __WXMSW__
    inline bool IsBalloonsSupported() {
        return false;
    }
#endif

    wxIcon     m_iconTaskBarNormal;
    wxIcon     m_iconTaskBarDisconnected;
    wxIcon     m_iconTaskBarSnooze;
    wxString   m_strDefaultTitle;
    bool       m_bTaskbarInitiatedShutdown;

private:
    wxDateTime m_dtLastHoverDetected;

    wxTimer*   m_pRefreshTimer;

    bool       m_bMouseButtonPressed;

    void       ResetTaskBar();

    void       DisplayContextMenu();
    std::vector<RESULT*> GetRunningTasks(CMainDocument* pDoc);
    
    DECLARE_EVENT_TABLE()

};


class CTaskbarEvent : public wxEvent
{
public:
    CTaskbarEvent(wxEventType evtType, CTaskBarIcon *taskbar)
        : wxEvent(-1, evtType)
        {
            SetEventObject(taskbar);
        }

    CTaskbarEvent(wxEventType evtType, CTaskBarIcon *taskbar, wxString message)
        : wxEvent(-1, evtType), m_message(message)
        {
            SetEventObject(taskbar);
        }

    virtual wxEvent *Clone() const { return new CTaskbarEvent(*this); }

    wxString                m_message;
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_TASKBAR_RELOADSKIN, 10100 )
END_DECLARE_EVENT_TYPES()

#define EVT_TASKBAR_RELOADSKIN(fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_TASKBAR_RELOADSKIN, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),


#endif //BOINCTASKBAR_H
