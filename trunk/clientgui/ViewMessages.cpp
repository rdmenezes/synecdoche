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
#include "AdvancedFrame.h"
#include "BOINCBaseFrame.h"
#include "BOINCGUIApp.h"
#include "BOINCListCtrl.h"
#include "BOINCTaskCtrl.h"
#include "DlgMsgFilter.h"
#include "Events.h"
#include "MainDocument.h"

#include "res/mess.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2

// buttons in the "tasks" area
#define BTN_COPYALL      0
#define BTN_COPYSELECTED 1
#define BTN_EDITFILTER   2
#define BTN_ENABLEFILTER 3


IMPLEMENT_DYNAMIC_CLASS(CViewMessages, CTaskViewBase)

BEGIN_EVENT_TABLE (CViewMessages, CTaskViewBase)
    EVT_BUTTON(ID_TASK_MESSAGES_COPYALL, CViewMessages::OnMessagesCopyAll)
    EVT_BUTTON(ID_TASK_MESSAGES_COPYSELECTED, CViewMessages::OnMessagesCopySelected)
    EVT_BUTTON(ID_TASK_MESSAGES_EDIT_FILTER, CViewMessages::OnMessagesEditFilter)
    EVT_BUTTON(ID_TASK_MESSAGES_ENABLE_FILTER, CViewMessages::OnMessagesEnableFilter)
    EVT_LIST_ITEM_FOCUSED(ID_LIST_MESSAGESVIEW, CViewMessages::OnListSelected)
    EVT_LIST_ITEM_SELECTED(ID_LIST_MESSAGESVIEW, CViewMessages::OnListSelected)
END_EVENT_TABLE ()


CViewMessages::CViewMessages() : m_enableMsgFilter(false) {
}

CViewMessages::CViewMessages(wxNotebook* pNotebook) :
    CTaskViewBase(pNotebook), m_enableMsgFilter(false) {
    m_pMessageInfoAttr = NULL;
    m_pMessageErrorAttr = NULL;
}

CViewMessages::~CViewMessages() {
    delete m_pMessageInfoAttr;
    m_pMessageInfoAttr = 0;

    delete m_pMessageErrorAttr;
    m_pMessageErrorAttr = 0;
}

void CViewMessages::DemandLoadView() {
    wxASSERT(!m_bViewLoaded);

    CTaskViewBase::DemandLoadView(
        ID_TASK_MESSAGESVIEW,
        DEFAULT_TASK_FLAGS,
        ID_LIST_MESSAGESVIEW,
        DEFAULT_LIST_MULTI_SEL_FLAGS
    );

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    // Initialize variables used in later parts of the class
    m_iPreviousDocCount = 0;
    m_maxFilteredIndex = 0;


    // Setup View
    CTaskItemGroup* pGroup = new CTaskItemGroup( _("Commands") );
    m_TaskGroups.push_back( pGroup );

    CTaskItem* pItem = new CTaskItem(
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

    pItem = new CTaskItem(_("Edit message filter"),
                          _("Edit the settings of the message filter."),
                          ID_TASK_MESSAGES_EDIT_FILTER);
    pGroup->m_Tasks.push_back(pItem);
    
    pItem = new CTaskItem(_("Enable message filter"),
                          _("Enable message filtering based on the current filter settings."),
                          ID_TASK_MESSAGES_ENABLE_FILTER);
    pGroup->m_Tasks.push_back(pItem);

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

void CViewMessages::OnMessagesEditFilter(wxCommandEvent& /* event */) {
    DlgMsgFilter dlg(this);
    dlg.SetFilterData(m_msgFilterData);
    if (dlg.ShowModal() == wxID_OK) {
        m_msgFilterData = dlg.GetFilterData();
        m_maxFilteredIndex = 0;
        m_filteredIndexes.clear();
        
        // Refresh the list if filtering is currently enabled:
        if (m_enableMsgFilter) {
            long docCount = GetDocCount();
            m_pListPane->SetItemCount(docCount);
            m_pListPane->RefreshItems(0, docCount - 1);
        }
    }
}

void CViewMessages::OnMessagesEnableFilter(wxCommandEvent& /* event */) {
    m_enableMsgFilter = !m_enableMsgFilter;
    if (m_enableMsgFilter) {
        m_pTaskPane->UpdateTask(m_TaskGroups[0]->m_Tasks[BTN_ENABLEFILTER],
                                _("Disable message filter"),
                                _("Disable message filtering."));
    } else {
        m_pTaskPane->UpdateTask(m_TaskGroups[0]->m_Tasks[BTN_ENABLEFILTER],
                                _("Enable message filter"),
                                _("Enable message filtering based on the current filter settings."));
    }

    // Refresh the list:
    long docCount = GetDocCount();
    m_pListPane->SetItemCount(docCount);
    m_pListPane->RefreshItems(0, docCount - 1);
}

/// Called when the manager has successfully connected to a client.
void CViewMessages::OnConnect() {
    // Reset the filter data because the client the manager is now connected to may be
    // attached to different porjects than the client the manager was connected to before:
    m_msgFilterData.ResetToDefaults();
    m_filteredIndexes.clear();
    m_maxFilteredIndex = 0;
    
    if (m_enableMsgFilter) {
        m_enableMsgFilter = false;
        m_pTaskPane->UpdateTask(m_TaskGroups[0]->m_Tasks[BTN_ENABLEFILTER],
                                _("Enable message filter"),
                                _("Enable message filtering based on the current filter settings."));
    }
}

wxInt32 CViewMessages::GetDocCount() {
    if (m_enableMsgFilter) {
        FilterMessages();
        return m_filteredIndexes.size();
    } else {
        return wxGetApp().GetDocument()->GetMessageCount();
    }
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
    wxString strBuffer = wxEmptyString;
    size_t index = GetFilteredIndex(item);

    switch(column) {
    case COLUMN_PROJECT:
        FormatProjectName(index, strBuffer);
        break;
    case COLUMN_TIME:
        FormatTime(index, strBuffer);
        break;
    case COLUMN_MESSAGE:
        FormatMessage(index, strBuffer);
        break;
    }

    return strBuffer;
}

wxListItemAttr* CViewMessages::OnListGetItemAttr(long item) const {
    wxListItemAttr* pAttribute = 0;
    MESSAGE* message = wxGetApp().GetDocument()->message(GetFilteredIndex(item));

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

/// Filter the messages based on the current filter settings.
/// This function stores the index of the last filtered message and won't touch
/// messages older than this if the function is called again. It will only check all
/// messages that are newer than the one remembered. This makes calling this function
/// as cheap as possible.
/// If you want this function to filter all the messages, for example because the
/// filter settings changed or the manager is now connected to a different client,
/// then you need to set m_maxFilteredIndex to zero and clear m_filteredIndexes
/// before calling this function.
void CViewMessages::FilterMessages() {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    size_t end = pDoc->GetMessageCount();
    
    for (size_t index = m_maxFilteredIndex; index < end; ++index) {
        MESSAGE* msg = pDoc->message(index);
        if ((msg->project.empty()) || m_msgFilterData.IsProjectSelected(msg->project)) {            
            bool includeThisMsg = true;
            
            // Check if the message contains a debug flag:
            std::string::size_type start = msg->body.find('[');
            if (start != std::string::npos) {
                std::string::size_type end = msg->body.find(']', start);
                if (end != std::string::npos) {
                    std::string debugFlag = msg->body.substr(start + 1, end - start - 1);
                    if ((m_msgFilterData.IsDebugFlagValid(debugFlag))
                            && (!m_msgFilterData.IsDebugFlagSelected(debugFlag))) {
                        includeThisMsg = false;
                    }
                }
            }

            if (includeThisMsg) {
                // The message has passed all filters, thus add it to the list:
                m_filteredIndexes.push_back(index);
            }
        }
    }
    
    // Check if more messages passed the filters than should be shown and truncate the
    // list if necessary. The oldest messages (e.g. the ones added first) are removed first.
    if (m_filteredIndexes.size() > m_msgFilterData.GetNumVisibleMsg()) {
        m_filteredIndexes.erase(m_filteredIndexes.begin(),
                                m_filteredIndexes.end() - m_msgFilterData.GetNumVisibleMsg());
    }
    
    m_maxFilteredIndex = end;
}

/// Return the message index for the requested row index.
/// Call this function whenever you need to get a certain message from the client
/// based on a row index of the list control. These too indexes may not be identical
/// if message filtering is activated, therefore using this function is the only way
/// to get the correct message.
///
/// \param[in] index The row index as returned by the list control.
/// \return The corresponding message index that can be used to get the correct message
///         from the core client.
size_t CViewMessages::GetFilteredIndex(size_t index) const {
    if (m_enableMsgFilter) {
        return m_filteredIndexes.at(index);
    } else {
        return index;
    }
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
        
        size_t index = GetFilteredIndex(item);

        FormatTime(index, strTimeStamp);
        FormatProjectName(index, strProject);
        FormatMessage(index, strMessage);

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
