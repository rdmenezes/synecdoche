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


#ifndef ADVANCEDFRAME_H
#define ADVANCEDFRAME_H

#include <wx/statusbr.h>
#include "BOINCBaseFrame.h"

class CC_STATUS;

class wxMenuBar;
class wxNotebook;
class wxNotebookEvent;
class CStatusBar;
class CBOINCBaseView;

class CStatusBar : public wxStatusBar
{
    DECLARE_DYNAMIC_CLASS(CStatusBar)

public:
    CStatusBar();
    CStatusBar(wxWindow *parent);
    ~CStatusBar() {}

};


class CAdvancedFrame : public CBOINCBaseFrame
{
    DECLARE_DYNAMIC_CLASS(CAdvancedFrame)

public:
    CAdvancedFrame();
    CAdvancedFrame(wxString title, wxIcon* icon, wxIcon* icon32);

    ~CAdvancedFrame(void);

    void OnSwitchGUI( wxCommandEvent& event );

    void OnActivitySelection( wxCommandEvent& event );
    void OnNetworkSelection( wxCommandEvent& event );

    void OnProjectsAttachToProject( wxCommandEvent& event );
    void OnProjectsAttachToAccountManager( wxCommandEvent& event );
    void OnAccountManagerUpdate( wxCommandEvent& event );
    void OnAccountManagerDetach( wxCommandEvent& event );

    void OnOptionsOptions( wxCommandEvent& event );
    void OnDlgPreferences( wxCommandEvent& event );
    void OnSelectComputer( wxCommandEvent& event );
    void OnClientShutdown( wxCommandEvent& event );
    void OnRunBenchmarks( wxCommandEvent& event );
    void OnCommandsRetryCommunications( wxCommandEvent& event );
    void Onread_prefs( wxCommandEvent& event );
    void Onread_config( wxCommandEvent& event );

    void OnHelpAbout( wxCommandEvent& event );

    void OnSize(wxSizeEvent& event);
    void OnMove(wxMoveEvent& event);

    void OnRefreshState( wxTimerEvent& event );
    void OnFrameRender( wxTimerEvent& event );
    void OnListPanelRender( wxTimerEvent& event );

    void OnNotebookSelectionChanged( wxNotebookEvent& event );

    void OnRefreshView( CFrameEvent& event );
    void OnConnect( CFrameEvent& event );
    void OnUpdateStatus( CFrameEvent& event );

    void OnIdleInit(wxIdleEvent& event);

    void ResetReminderTimers();

    wxTimer*        m_pRefreshStateTimer;
    wxTimer*        m_pFrameRenderTimer;
    wxTimer*        m_pFrameListPanelRenderTimer;

private:

    wxMenuBar*      m_pMenubar;
    wxNotebook*     m_pNotebook;
    CStatusBar*     m_pStatusbar;

    bool            m_bDisplayShutdownClientWarning;

    wxString        m_strBaseTitle;
    wxString        m_cachedStatusText;

    /// Next page to load in the background.
    size_t          m_pageToLoad;

    /// Store window size and location (but not maximised size).
    wxRect          m_windowRect;

    bool            CreateMenu();
    bool            DeleteMenu();

    bool            CreateNotebook();
    bool            RepopulateNotebook();

    bool            CreateNotebookPage( CBOINCBaseView* pwndNewNotebookPage );
    bool            DeleteNotebook();

    bool            CreateStatusbar();
    bool            DeleteStatusbar();

    bool            SaveState();
    bool            SaveViewState();
    bool            RestoreState();
    bool            RestoreViewState();

    void            SaveWindowDimensions();
    void            RestoreWindowDimensions();

    void            UpdateActivityModeControls(const CC_STATUS& status);
    void            UpdateNetworkModeControls(const CC_STATUS& status);
    void            UpdateRefreshTimerInterval(wxInt32 iCurrentNotebookPage);

    void            StartTimers();
    void            StopTimers();

#ifdef __WXMAC__
protected:

    wxAcceleratorEntry  m_Shortcuts[1];     // For HELP keyboard shortcut
    wxAcceleratorTable* m_pAccelTable;
#endif

    DECLARE_EVENT_TABLE()
};


#endif // ADVANCEDFRAME_H
