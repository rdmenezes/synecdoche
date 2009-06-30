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

#include "ViewMessages.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "Events.h"

#include "res/mess.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2

// buttons in the "tasks" area
#define BTN_COPYALL      0
#define BTN_COPYSELECTED 1


IMPLEMENT_DYNAMIC_CLASS(CViewMessages, CTaskViewBase)

BEGIN_EVENT_TABLE (CViewMessages, CTaskViewBase)
    EVT_BUTTON(ID_TASK_MESSAGES_COPYALL, CViewMessages::OnMessagesCopyAll)
    EVT_BUTTON(ID_TASK_MESSAGES_COPYSELECTED, CViewMessages::OnMessagesCopySelected)
    EVT_LIST_ITEM_FOCUSED(ID_LIST_MESSAGESVIEW, CViewMessages::OnListSelected)
    EVT_LIST_ITEM_SELECTED(ID_LIST_MESSAGESVIEW, CViewMessages::OnListSelected)
END_EVENT_TABLE ()


CViewMessages::CViewMessages()
{}


CViewMessages::CViewMessages(wxNotebook* pNotebook) :
    CTaskViewBase(pNotebook)
{
    m_pMessageInfoAttr = NULL;
    m_pMessageErrorAttr = NULL;
}


CViewMessages::~CViewMessages() {
    if (m_pMessageInfoAttr) {
        delete m_pMessageInfoAttr;
        m_pMessageInfoAttr = NULL;
    }

    if (m_pMessageErrorAttr) {
        delete m_pMessageErrorAttr;
        m_pMessageErrorAttr = NULL;
    }
}


void CViewMessages::DemandLoadView() {
    wxASSERT(!m_bViewLoaded);

    CTaskViewBase::DemandLoadView(
        ID_TASK_MESSAGESVIEW,
        DEFAULT_TASK_FLAGS,
        ID_LIST_MESSAGESVIEW,
        DEFAULT_LIST_MULTI_SEL_FLAGS
    );

    CTaskItemGroup* pGroup = NULL;
    CTaskItem*      pItem = NULL;

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);


    //
    // Initialize variables used in later parts of the class
    //
    m_iPreviousDocCount = 0;


    //
    // Setup View
    //
    pGroup = new CTaskItemGroup( _("Commands") );
    m_TaskGroups.push_back( pGroup );

    pItem = new CTaskItem(
        _("Copy all messages"),
        _("Copy all the messages to the clipboard."),
        ID_TASK_MESSAGES_COPYALL 
    );
    pGroup->m_Tasks.push_back( pItem );

    pItem = new CTaskItem(
        _("Copy selected messages"),
#ifdef __WXMAC__
        _("Copy the selected messages to the clipboard. "
          "You can select multiple messages by holding down the shift "
          "or command key while clicking on messages."),
#else
        _("Copy the selected messages to the clipboard. "
          "You can select multiple messages by holding down the shift "
          "or control key while clicking on messages."),
#endif
        ID_TASK_MESSAGES_COPYSELECTED 
    );
    pGroup->m_Tasks.push_back( pItem );


    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

    // Create List Pane Items
    AddColumn(COLUMN_PROJECT, wxTRANSLATE("Project"), wxLIST_FORMAT_LEFT, 115);
    AddColumn(COLUMN_TIME,    wxTRANSLATE("Time"), wxLIST_FORMAT_LEFT, 145);
    AddColumn(COLUMN_MESSAGE, wxTRANSLATE("Message"), wxLIST_FORMAT_LEFT, 550);

    m_pMessageInfoAttr = new wxListItemAttr(*wxBLACK, *wxWHITE, wxNullFont);
    m_pMessageErrorAttr = new wxListItemAttr(*wxRED, *wxWHITE, wxNullFont);

    RestoreState();

    UpdateSelection();
}


const wxString& CViewMessages::GetViewName() {
    static wxString strViewName(wxT("Messages"));
    return strViewName;
}


const wxString& CViewMessages::GetViewDisplayName() {
    static wxString strViewName(_("Messages"));
    return strViewName;
}


const char** CViewMessages::GetViewIcon() {
    return mess_xpm;
}


void CViewMessages::OnMessagesCopyAll( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesCopyAll - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

#ifdef wxUSE_CLIPBOARD

    pFrame->UpdateStatusText(_("Copying all messages to the clipboard..."));

    int iRowCount = m_pListPane->GetItemCount();
    OpenClipboard(iRowCount);

    for (wxInt32 iIndex = 0; iIndex < iRowCount; iIndex++) {
        CopyToClipboard(iIndex);            
    }

    CloseClipboard();
    pFrame->UpdateStatusText(wxT(""));

#endif

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesCopyAll - Function End"));
}


void CViewMessages::OnMessagesCopySelected( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesCopySelected - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

#ifdef wxUSE_CLIPBOARD

    wxInt32 iIndex = -1;

    pFrame->UpdateStatusText(_("Copying selected messages to the clipboard..."));
    OpenClipboard(m_pListPane->GetSelectedItemCount());

    for (;;) {
        iIndex = m_pListPane->GetNextItem(
            iIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED
        );
        if (iIndex == -1) break;

        CopyToClipboard(iIndex);            
    }

    CloseClipboard();
    pFrame->UpdateStatusText(wxT(""));

#endif

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesCopySelected - Function End"));
}


wxInt32 CViewMessages::GetDocCount() {
    return static_cast<wxInt32>(wxGetApp().GetDocument()->GetMessageCount());
}


void CViewMessages::OnListRender (wxTimerEvent& event) {
    bool isConnected;
    static bool was_connected = false;
    static wxString strLastMachineName = wxEmptyString;
    wxString strNewMachineName = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    
    if (!m_bProcessingListRenderEvent) {
        m_bProcessingListRenderEvent = true;

        wxASSERT(m_pListPane);

        isConnected = pDoc->IsConnected();
        wxInt32 iDocCount = GetDocCount();
        if (0 >= iDocCount) {
            m_pListPane->DeleteAllItems();
        } else {
            // If connection status changed, adjust color of messages display
            if (was_connected != isConnected) {
                was_connected = isConnected;
                if (isConnected) {
                    m_pMessageInfoAttr->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
                    m_pMessageErrorAttr->SetTextColour(*wxRED);
                } else {
                    m_pMessageInfoAttr->SetTextColour(CBOINCListCtrl::GetBlendedColour(
                                wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT),
                                wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW), 0.5));
                    m_pMessageErrorAttr->SetTextColour(CBOINCListCtrl::GetBlendedColour(*wxRED,
                                wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW), 0.5));
                }
                m_pMessageInfoAttr->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
                m_pMessageErrorAttr->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
                // Force a complete update
                m_pListPane->DeleteAllItems();
                m_pListPane->SetItemCount(iDocCount);
           }
            
            if (m_iPreviousDocCount != iDocCount)
                m_pListPane->SetItemCount(iDocCount);
        }

        if ((iDocCount) && (_EnsureLastItemVisible()) && (m_iPreviousDocCount != iDocCount)) {
            m_pListPane->EnsureVisible(iDocCount - 1);
        }

        if (isConnected) {
            pDoc->GetConnectedComputerName(strNewMachineName);
            if (strLastMachineName != strNewMachineName) {
                strLastMachineName = strNewMachineName;
                if (iDocCount) {
                    m_pListPane->EnsureVisible(iDocCount - 1);
                }
            }
        }

        if (m_iPreviousDocCount != iDocCount) {
            m_iPreviousDocCount = iDocCount;
        }

        m_bProcessingListRenderEvent = false;
    }

    event.Skip();
}


wxString CViewMessages::OnListGetItemText(long item, long column) const {
    wxString        strBuffer   = wxEmptyString;

    switch(column) {
    case COLUMN_PROJECT:
        FormatProjectName(item, strBuffer);
        break;
    case COLUMN_TIME:
        FormatTime(item, strBuffer);
        break;
    case COLUMN_MESSAGE:
        FormatMessage(item, strBuffer);
        break;
    }

    return strBuffer;
}


wxListItemAttr* CViewMessages::OnListGetItemAttr(long item) const {
    wxListItemAttr* pAttribute  = NULL;
    MESSAGE*        message     = wxGetApp().GetDocument()->message(item);

    if (message) {
        switch(message->priority) {
        case MSG_USER_ERROR:
            pAttribute = m_pMessageErrorAttr;
            break;
        default:
            pAttribute = m_pMessageInfoAttr;
            break;
        }
    }

    return pAttribute;
}


bool CViewMessages::EnsureLastItemVisible() {
    int numVisible = m_pListPane->GetCountPerPage();

    // Auto-scroll only if already at bottom of list
    if ((m_iPreviousDocCount > numVisible)
         && ((m_pListPane->GetTopItem() + numVisible) < (m_iPreviousDocCount-1)) 
    ) {
        return false;
    }
    
    return true;
}


void CViewMessages::UpdateSelection() {
    CTaskItemGroup*     pGroup = NULL;

    CTaskViewBase::PreUpdateSelection();

    pGroup = m_TaskGroups[0];
    if (m_pListPane->GetSelectedItemCount()) {
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_COPYSELECTED]);
    } else {
        m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_COPYSELECTED]);
    }

    CTaskViewBase::PostUpdateSelection();
}


wxInt32 CViewMessages::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    MESSAGE* message = wxGetApp().GetDocument()->message(item);

    if (message) {
        strBuffer = HtmlEntityDecode(wxString(message->project.c_str(), wxConvUTF8));
    }

    return 0;
}


wxInt32 CViewMessages::FormatTime(wxInt32 item, wxString& strBuffer) const {
    wxDateTime dtBuffer;
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);

    if (message) {
        dtBuffer.Set((time_t)message->timestamp);
        strBuffer = dtBuffer.Format();
    }

    return 0;
}


wxInt32 CViewMessages::FormatMessage(wxInt32 item, wxString& strBuffer) const {
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);

    if (message) {
        strBuffer = wxString(message->body.c_str(), wxConvUTF8);
    }

    strBuffer.Replace(wxT("\n"), wxT(""), true);

    return 0;
}


#ifdef wxUSE_CLIPBOARD
bool CViewMessages::OpenClipboard(wxInt32 row_count) {
    bool bRetVal = false;

    bRetVal = wxTheClipboard->Open();
    if (bRetVal) {
        m_bClipboardOpen = true;
        m_strClipboardData = wxEmptyString;

        // Pre-allocate enough space to store all the lines
        // that should be copied. We estimate 130 characters per line.
        m_strClipboardData.Alloc(130 * row_count);
        wxTheClipboard->Clear();
    }

    return bRetVal;
}


wxInt32 CViewMessages::CopyToClipboard(wxInt32 item) {
    wxInt32        iRetVal = -1;

    if (m_bClipboardOpen) {
        wxString       strBuffer = wxEmptyString;
        wxString       strTimeStamp = wxEmptyString;
        wxString       strProject = wxEmptyString;
        wxString       strMessage = wxEmptyString;

        FormatTime(item, strTimeStamp);
        FormatProjectName(item, strProject);
        FormatMessage(item, strMessage);

#ifdef __WXMSW__
        strBuffer.Printf(wxT("%s|%s|%s\r\n"), strTimeStamp.c_str(), strProject.c_str(), strMessage.c_str());
#else
        strBuffer.Printf(wxT("%s|%s|%s\n"), strTimeStamp.c_str(), strProject.c_str(), strMessage.c_str());
#endif

        m_strClipboardData += strBuffer;

        iRetVal = 0;
    }

    return iRetVal;
}


bool CViewMessages::CloseClipboard() {
    bool bRetVal = false;

    if (m_bClipboardOpen) {
        wxTheClipboard->SetData(new wxTextDataObject(m_strClipboardData));
        wxTheClipboard->Close();

        m_bClipboardOpen = false;
        m_strClipboardData = wxEmptyString;
    }

    return bRetVal;
}

#endif