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


#ifndef DLG_MESSAGES_H
#define DLG_MESSAGES_H

#include <wx/dialog.h>
#include <wx/panel.h>

class wxCommandEvent;
class wxConfigBase;
class wxListItemAttr;
class wxTimer;
class wxTimerEvent;

class CSGUIListCtrl;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DLGMESSAGES 10000
#define SYMBOL_CDLGMESSAGES_STYLE wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER
#define SYMBOL_CDLGMESSAGES_TITLE wxT("")
#define SYMBOL_CDLGMESSAGES_IDNAME ID_DLGMESSAGES
#define SYMBOL_CDLGMESSAGES_SIZE wxDefaultSize
#define SYMBOL_CDLGMESSAGES_POSITION wxDefaultPosition
#define ID_COPYSELECTED 10001
#define ID_COPYAll 10002
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif


/*!
 * CPanelPreferences class declaration
 */

class CPanelMessages : public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CPanelMessages )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CPanelMessages( );
    CPanelMessages( wxWindow* parent );

    /// Destructors
    ~CPanelMessages( );

    /// Creation
    bool Create();

    /// Creates the controls and sizers
    void CreateControls();

    /// @name CPanelMessages event handler declarations
    /// @{
    /// wxEVT_ERASE_BACKGROUND event handler for ID_DLGMESSAGES
    void OnEraseBackground( wxEraseEvent& event );

    /// wxEVT_TIMER event handler for ID_REFRESHMESSAGESTIMER
    void OnRefresh( wxTimerEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOK( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_COPYAll
    void OnMessagesCopyAll( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_COPYSELECTED
    void OnMessagesCopySelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SIMPLE_HELP
    void OnButtonHelp( wxCommandEvent& event );

    /// @}

    virtual wxString        OnListGetItemText( long item, long column ) const;
    virtual wxListItemAttr* OnListGetItemAttr( long item ) const;

    bool                    OnSaveState(wxConfigBase* pConfig);
    bool                    OnRestoreState(wxConfigBase* pConfig);

private:
    size_t                  m_iPreviousDocCount;

    CSGUIListCtrl*          m_pList;
    wxListItemAttr*         m_pMessageInfoAttr;
    wxListItemAttr*         m_pMessageErrorAttr;

    bool                    m_bProcessingRefreshEvent;
    bool                    m_bForceUpdateSelection;

    wxTimer*                m_pRefreshMessagesTimer;

    bool                    EnsureLastItemVisible();
    wxInt32                 FormatProjectName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatTime( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatMessage( wxInt32 item, wxString& strBuffer ) const;

#ifdef wxUSE_CLIPBOARD
    bool                    m_bClipboardOpen;
    wxString                m_strClipboardData;
    bool                    OpenClipboard();
    wxInt32                 CopyToClipboard( wxInt32 item );
    bool                    CloseClipboard();
#endif
};


class CDlgMessages : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CDlgMessages )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgMessages( );
    CDlgMessages( wxWindow* parent, wxWindowID id = SYMBOL_CDLGMESSAGES_IDNAME, const wxString& caption = SYMBOL_CDLGMESSAGES_TITLE, const wxPoint& pos = SYMBOL_CDLGMESSAGES_POSITION, const wxSize& size = SYMBOL_CDLGMESSAGES_SIZE, long style = SYMBOL_CDLGMESSAGES_STYLE );

    ~CDlgMessages();

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGMESSAGES_IDNAME, const wxString& caption = SYMBOL_CDLGMESSAGES_TITLE, const wxPoint& pos = SYMBOL_CDLGMESSAGES_POSITION, const wxSize& size = SYMBOL_CDLGMESSAGES_SIZE, long style = SYMBOL_CDLGMESSAGES_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_HELP event handler for ID_DLGMESSAGES
    void OnHelp( wxHelpEvent& event );

    /// wxEVT_SHOW event handler for ID_DLGMESSAGES
    void OnShow( wxShowEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOK( wxCommandEvent& event );

private:

    bool SaveState();
    void SaveWindowDimensions();
    bool RestoreState();
    void RestoreWindowDimensions();

    /// @name CDlgMessages member variables
    /// @{
    CPanelMessages* m_pBackgroundPanel;
    /// @}
};


#endif  // end CDlgMessages
