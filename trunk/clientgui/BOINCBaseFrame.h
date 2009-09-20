// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
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

#ifndef BOINCBASEFRAME_H
#define BOINCBASEFRAME_H

#include <wx/frame.h>
#include <wx/string.h>
#include <wx/timer.h>

#include "attributes.h"

class CFrameEvent;
class CFrameAlertEvent;
#ifndef __WXMAC__
class CBOINCDialUpManager;
#endif // __WXMAC__

enum FrameAlertEventType {
    AlertNormal = 0,
    AlertProcessResponse
};


class CBOINCBaseFrame : public wxFrame {

    DECLARE_DYNAMIC_CLASS( CBOINCBaseFrame )

public:

    CBOINCBaseFrame();
    CBOINCBaseFrame(
        wxWindow *parent,
        const wxWindowID id,
        const wxString& title,
        const wxPoint& pos,
        const wxSize& size,
        const long style
    );

    virtual ~CBOINCBaseFrame();

    void                OnDocumentPoll( wxTimerEvent& event );
    void                OnAlertPoll( wxTimerEvent& event );

    void                OnInitialized( CFrameEvent& event );

    virtual void        OnAlert( CFrameAlertEvent& event );
    virtual void        OnClose( wxCloseEvent& event );
    virtual void        OnCloseWindow( wxCommandEvent& event );
    virtual void        OnExit( wxCommandEvent& event );

    void                OnContextHelp(wxHelpEvent& event);
    void                OnHelp(wxCommandEvent& event);

    int                 GetReminderFrequency() { return m_iReminderFrequency; }
    wxString            GetDialupConnectionName() { return m_strNetworkDialupConnectionName; }

    void                FireInitialize();
    void                FireRefreshView();
    void                FireConnect();
    void                FireReloadSkin();
    void                ShowConnectionBadPasswordAlert( bool bUsedDefaultPassword, int m_iReadGUIRPCAuthFailure );
    void                ShowConnectionFailedAlert();
    void                ShowDaemonStartFailedAlert();
    void                ShowNotCurrentlyConnectedAlert();

    virtual void        StartTimers();
    virtual void        StopTimers();

    void                UpdateStatusText( const wxChar* szStatus );

    void                ShowAlert( 
                            const wxString title,
                            const wxString message,
                            const int style,
                            const bool notification_only = false,
                            const FrameAlertEventType alert_event_type = AlertNormal
                        );

#ifdef __WXMAC__
    bool                Show( bool show = true );
    int                 m_iWindowType;              // BOINC_SIMPLEGUI or BOINC_ADVANCEDGUI
#endif

protected:

#ifndef __WXMAC__
    CBOINCDialUpManager* m_pDialupManager;
#endif // __WXMAC__

    wxTimer*            m_pDocumentPollTimer;
    wxTimer*            m_pAlertPollTimer;

    int                 m_iSelectedLanguage;
    int                 m_iReminderFrequency;

    wxString            m_strNetworkDialupConnectionName;

    wxArrayString       m_aSelectedComputerMRU;

    bool                m_bShowConnectionFailedAlert;

    virtual bool        SaveState();
    virtual bool        RestoreState();
    void                ShowHelp(wxEvent& event);

    DECLARE_EVENT_TABLE()
};


class CFrameEvent : public wxEvent
{
public:
    CFrameEvent(wxEventType evtType, CBOINCBaseFrame *frame)
        : wxEvent(-1, evtType)
        {
            SetEventObject(frame);
        }

    CFrameEvent(wxEventType evtType, CBOINCBaseFrame *frame, wxString message)
        : wxEvent(-1, evtType), m_message(message)
        {
            SetEventObject(frame);
        }

    virtual wxEvent *Clone() const { return new CFrameEvent(*this); }

    wxString                m_message;
};


class CFrameAlertEvent : public wxEvent
{
public:
    CFrameAlertEvent(wxEventType evtType, CBOINCBaseFrame *frame, wxString title, wxString message, int style, bool notification_only, FrameAlertEventType alert_event_type)
        : wxEvent(-1, evtType), m_title(title), m_message(message), m_style(style), m_notification_only(notification_only), m_alert_event_type(alert_event_type)
        {
            SetEventObject(frame);
        }

    CFrameAlertEvent(wxEventType evtType, CBOINCBaseFrame *frame, wxString title, wxString message, int style, bool notification_only)
        : wxEvent(-1, evtType), m_title(title), m_message(message), m_style(style), m_notification_only(notification_only)
        {
            SetEventObject(frame);
            m_alert_event_type = AlertNormal;
        }

    CFrameAlertEvent(const CFrameAlertEvent& event)
        : wxEvent(event)
        {
            m_title = event.m_title;
            m_message = event.m_message;
            m_style = event.m_style;
            m_notification_only = event.m_notification_only;
            m_alert_event_type = event.m_alert_event_type;
        }

    virtual wxEvent *Clone() const { return new CFrameAlertEvent(*this); }
    virtual void     ProcessResponse(const int response) const;

    wxString                m_title;
    wxString                m_message;
    int                     m_style;
    bool                    m_notification_only;
    FrameAlertEventType m_alert_event_type;
};


DECLARE_EVENT_TYPE( wxEVT_FRAME_ALERT, -1 )
DECLARE_EVENT_TYPE( wxEVT_FRAME_CONNECT, -1 )
DECLARE_EVENT_TYPE( wxEVT_FRAME_INITIALIZED, -1 )
DECLARE_EVENT_TYPE( wxEVT_FRAME_REFRESHVIEW, -1 )
DECLARE_EVENT_TYPE( wxEVT_FRAME_UPDATESTATUS, -1 )
DECLARE_EVENT_TYPE( wxEVT_FRAME_RELOADSKIN, -1 )

#define EVT_FRAME_ALERT(fn)              DECLARE_EVENT_TABLE_ENTRY(wxEVT_FRAME_ALERT, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_FRAME_CONNECT(fn)            DECLARE_EVENT_TABLE_ENTRY(wxEVT_FRAME_CONNECT, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_FRAME_INITIALIZED(fn)        DECLARE_EVENT_TABLE_ENTRY(wxEVT_FRAME_INITIALIZED, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_FRAME_REFRESH(fn)            DECLARE_EVENT_TABLE_ENTRY(wxEVT_FRAME_REFRESHVIEW, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_FRAME_UPDATESTATUS(fn)       DECLARE_EVENT_TABLE_ENTRY(wxEVT_FRAME_UPDATESTATUS, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_FRAME_RELOADSKIN(fn)         DECLARE_EVENT_TABLE_ENTRY(wxEVT_FRAME_RELOADSKIN, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),


#endif

