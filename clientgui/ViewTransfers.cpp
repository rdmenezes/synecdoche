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

#include "ViewTransfers.h"
#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "Events.h"
#include "error_numbers.h"

#include "res/xfer.xpm"

#define COLUMN_PROJECT              0
#define COLUMN_FILE                 1
#define COLUMN_PROGRESS             2
#define COLUMN_SIZE                 3
#define COLUMN_TIME                 4
#define COLUMN_SPEED                5
#define COLUMN_STATUS               6

// buttons in the "tasks" area
#define BTN_RETRY       0
#define BTN_ABORT       1

CTransfer::CTransfer() {
    m_fProgress = -1.0f;
    m_fBytesXferred = -1.0f;
    m_fTotalBytes = -1.0f;
    m_dTime = -1.0;
    m_dSpeed = -1.0;
}

IMPLEMENT_DYNAMIC_CLASS(CViewTransfers, CTaskViewBase)

BEGIN_EVENT_TABLE (CViewTransfers, CTaskViewBase)
    EVT_BUTTON(ID_TASK_TRANSFERS_RETRYNOW, CViewTransfers::OnTransfersRetryNow)
    EVT_BUTTON(ID_TASK_TRANSFERS_ABORT, CViewTransfers::OnTransfersAbort)
    EVT_LIST_ITEM_FOCUSED(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnListSelected)
    EVT_LIST_ITEM_SELECTED(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnListSelected)
    EVT_LIST_COL_CLICK(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnColClick)
    EVT_LIST_CACHE_HINT(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnCacheHint)
END_EVENT_TABLE ()

static CViewTransfers* MyCViewTransfers;

static bool CompareViewTransferItems(size_t iRowIndex1, size_t iRowIndex2) {
    CTransfer*      transfer1 = MyCViewTransfers->m_TransferCache.at(iRowIndex1);
    CTransfer*      transfer2 = MyCViewTransfers->m_TransferCache.at(iRowIndex2);
    int             result = 0;
    
    switch (MyCViewTransfers->m_iSortColumn) {
        case COLUMN_PROJECT:
            result = transfer1->m_strProjectName.CmpNoCase(transfer2->m_strProjectName);
            break;
        case COLUMN_FILE:
            result = transfer1->m_strFileName.CmpNoCase(transfer2->m_strFileName);
            break;
        case COLUMN_PROGRESS:
            if (transfer1->m_fProgress < transfer2->m_fProgress) {
                result = -1;
            } else if (transfer1->m_fProgress > transfer2->m_fProgress) {
                result = 1;
            }
            break;
        case COLUMN_SIZE:
            if (transfer1->m_fBytesXferred < transfer2->m_fBytesXferred) {
                result = -1;
            } else if (transfer1->m_fBytesXferred > transfer2->m_fBytesXferred) {
                result = 1;
            }
            break;
        case COLUMN_TIME:
            if (transfer1->m_dTime < transfer2->m_dTime) {
                result = -1;
            } else if (transfer1->m_dTime > transfer2->m_dTime) {
                result = 1;
            }
            break;
        case COLUMN_SPEED:
            if (transfer1->m_dSpeed < transfer2->m_dSpeed) {
                result = -1;
            } else if (transfer1->m_dSpeed > transfer2->m_dSpeed) {
                result = 1;
            }
            break;
        case COLUMN_STATUS:
            result = transfer1->m_strStatus.CmpNoCase(transfer2->m_strStatus);
            break;
    }

    // Always return false for equality (result == 0).
    return ((MyCViewTransfers->m_bReverseSort) ? (result > 0) : (result < 0));
}

CViewTransfers::CViewTransfers() {
}

CViewTransfers::CViewTransfers(wxNotebook* pNotebook) : CTaskViewBase(pNotebook) {
}

CViewTransfers::~CViewTransfers() {
    EmptyCache();
}

void CViewTransfers::DemandLoadView() {
    wxASSERT(!m_bViewLoaded);

    CTaskViewBase::DemandLoadView(ID_TASK_TRANSFERSVIEW, DEFAULT_TASK_FLAGS,
                                    ID_LIST_TRANSFERSVIEW, DEFAULT_LIST_MULTI_SEL_FLAGS);

    CTaskItemGroup* pGroup = NULL;
    CTaskItem*      pItem = NULL;

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    // Setup View
    pGroup = new CTaskItemGroup(_("Commands"));
    m_TaskGroups.push_back(pGroup);

    pItem = new CTaskItem(_("Retry Now"), _("Click 'Retry now' to transfer the file now"),
                            ID_TASK_TRANSFERS_RETRYNOW);
    pGroup->m_Tasks.push_back(pItem);

    pItem = new CTaskItem(_("Abort Transfer"),
                            _("Click 'Abort transfer' to delete the file from the transfer queue. "
                              "This will prevent you from being granted credit for this result."),
                            ID_TASK_TRANSFERS_ABORT);
    pGroup->m_Tasks.push_back(pItem);

    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

    // Create List Pane Items
    AddColumn(COLUMN_PROJECT,  _T("Project"), wxLIST_FORMAT_LEFT, 125);
    AddColumn(COLUMN_FILE,     _T("File"), wxLIST_FORMAT_LEFT, 205);
    AddColumn(COLUMN_PROGRESS, _T("Progress"), wxLIST_FORMAT_CENTRE, 60);
    AddColumn(COLUMN_SIZE,     _T("Size"), wxLIST_FORMAT_LEFT, 80);
    AddColumn(COLUMN_TIME,     _T("Elapsed Time"), wxLIST_FORMAT_LEFT, 80);
    AddColumn(COLUMN_SPEED,    _T("Speed"), wxLIST_FORMAT_LEFT, 80);
    AddColumn(COLUMN_STATUS,   _T("Status"), wxLIST_FORMAT_LEFT, 150);

    m_iProgressColumn = COLUMN_PROGRESS;

    // Needed by static sort routine;
    MyCViewTransfers = this;
    m_funcSortCompare = CompareViewTransferItems;

    RestoreState();

    UpdateSelection();
}

const wxString& CViewTransfers::GetViewName() {
    static wxString strViewName(wxT("Transfers"));
    return strViewName;
}

const wxString& CViewTransfers::GetViewDisplayName() {
    static wxString strViewName(_("Transfers"));
    return strViewName;
}

const char** CViewTransfers::GetViewIcon() {
    return xfer_xpm;
}

void CViewTransfers::OnTransfersRetryNow( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfers::OnTransfersRetryNow - Function Begin"));

    CMainDocument*  pDoc    = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    pFrame->UpdateStatusText(_("Retrying transfer now..."));
    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        pDoc->TransferRetryNow(m_iSortedIndexes[row]);
    }
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->ResetReminderTimers();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfers::OnTransfersRetryNow - Function End"));
}

void CViewTransfers::OnTransfersAbort( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfers::OnTransfersAbort - Function Begin"));

    wxInt32         iAnswer    = 0; 
    wxString        strMessage = wxEmptyString;
    CMainDocument*  pDoc       = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame     = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    CTransfer*      pTransfer  = NULL;
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Aborting transfer..."));

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        pTransfer = m_TransferCache.at(m_iSortedIndexes[row]);

        strMessage.Printf(
            _("Are you sure you want to abort this file transfer '%s'?\n"
              "NOTE: Aborting a transfer will invalidate a task and you\n"
              "will not receive credit for it."), 
            pTransfer->m_strFileName.c_str()
        );

        iAnswer = ::wxMessageBox(
            strMessage,
            _("Abort File Transfer"),
            wxYES_NO | wxICON_QUESTION,
            this
        );

        if (wxYES == iAnswer) {
            pDoc->TransferAbort(m_iSortedIndexes[row]);
        }
    }
    
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfers::OnTransfersAbort - Function End"));
}

wxInt32 CViewTransfers::GetDocCount() {
    return static_cast<wxInt32>(wxGetApp().GetDocument()->GetTransferCount());
}

wxString CViewTransfers::OnListGetItemText(long item, long column) const {
    wxString   strBuffer  = wxEmptyString;

    CTransfer* transfer = m_TransferCache.at(m_iSortedIndexes[item]);
    
    switch (column) {
        case COLUMN_PROJECT:
            strBuffer = transfer->m_strProjectName;
            break;
        case COLUMN_FILE:
            strBuffer = transfer->m_strFileName;
            break;
        case COLUMN_PROGRESS:
            strBuffer = transfer->m_strProgress;
            break;
        case COLUMN_SIZE:
            strBuffer = transfer->m_strSize;
            break;
        case COLUMN_TIME:
            strBuffer = transfer->m_strTime;
            break;
        case COLUMN_SPEED:
            strBuffer = transfer->m_strSpeed;
            break;
        case COLUMN_STATUS:
            strBuffer = transfer->m_strStatus;
            break;
    }
    return strBuffer;
}

wxInt32 CViewTransfers::AddCacheElement() {
    CTransfer* pItem = new CTransfer();
    wxASSERT(pItem);
    if (pItem) {
        m_TransferCache.push_back(pItem);
        m_iSortedIndexes.push_back((int)m_TransferCache.size()-1);
        return 0;
    }
    return -1;
}

wxInt32 CViewTransfers::EmptyCache() {
    unsigned int i;
    for (i=0; i<m_TransferCache.size(); i++) {
        delete m_TransferCache[i];
    }
    m_TransferCache.clear();
    m_iSortedIndexes.clear();
    return 0;
}

wxInt32 CViewTransfers::GetCacheCount() {
    return (wxInt32)m_TransferCache.size();
}

wxInt32 CViewTransfers::RemoveCacheElement() {
    unsigned int i;
    delete m_TransferCache.back();
    m_TransferCache.erase(m_TransferCache.end() - 1);
    m_iSortedIndexes.clear();
    for (i=0; i<m_TransferCache.size(); i++) {
        m_iSortedIndexes.push_back(i);
    }
    return 0;
}

void CViewTransfers::UpdateSelection() {
    CTaskItemGroup* pGroup = m_TaskGroups[0];

    CTaskViewBase::PreUpdateSelection();

    if (m_pListPane->GetSelectedItemCount()) {
        m_pTaskPane->EnableTaskGroupTasks(pGroup);
    } else {
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
    }

    CTaskViewBase::PostUpdateSelection();
}

bool CViewTransfers::SynchronizeCacheItem(wxInt32 iRowIndex, wxInt32 iColumnIndex) {
    wxString    strDocumentText  = wxEmptyString;
    float       fDocumentFloat   = 0.0f;
    double      fDocumentDouble  = 0.0;
    double      fDocumentDouble2 = 0.0;
    CTransfer*  transfer         = m_TransferCache.at(m_iSortedIndexes[iRowIndex]);

    strDocumentText.Empty();

    switch(iColumnIndex) {
        case COLUMN_PROJECT:
            GetDocProjectName(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(transfer->m_strProjectName)) {
                transfer->m_strProjectName = strDocumentText;
                return true;
            }
            break;
        case COLUMN_FILE:
            GetDocFileName(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(transfer->m_strFileName)) {
                transfer->m_strFileName = strDocumentText;
                return true;
            }
            break;
        case COLUMN_PROGRESS:
            GetDocProgress(m_iSortedIndexes[iRowIndex], fDocumentFloat);
            if (fDocumentFloat != transfer->m_fProgress) {
                transfer->m_fProgress = fDocumentFloat;
                FormatProgress(fDocumentFloat, transfer->m_strProgress);
                return true;
            }
            break;
        case COLUMN_SIZE:
            GetDocBytesXferred(m_iSortedIndexes[iRowIndex], fDocumentDouble);
            GetDocTotalBytes(m_iSortedIndexes[iRowIndex], fDocumentDouble2);
            if ((fDocumentDouble != transfer->m_fBytesXferred) || (fDocumentDouble2 != transfer->m_fTotalBytes)) {
                transfer->m_fBytesXferred = fDocumentDouble;
                transfer->m_fTotalBytes = fDocumentDouble2;
                FormatSize(fDocumentDouble, fDocumentDouble2, transfer->m_strSize);
                return true;
            }
            break;
        case COLUMN_TIME:
            GetDocTime(m_iSortedIndexes[iRowIndex], fDocumentDouble);
            if (fDocumentDouble != transfer->m_dTime) {
                transfer->m_dTime = fDocumentDouble;
                FormatTime(fDocumentDouble, transfer->m_strTime);
                return true;
            }
            break;
        case COLUMN_SPEED:
            GetDocSpeed(m_iSortedIndexes[iRowIndex], fDocumentDouble);
            if (fDocumentDouble != transfer->m_dSpeed) {
                transfer->m_dSpeed = fDocumentDouble;
                FormatSpeed(fDocumentDouble, transfer->m_strSpeed);
                return true;
            }
            break;
        case COLUMN_STATUS:
            GetDocStatus(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(transfer->m_strStatus)) {
                transfer->m_strStatus = strDocumentText;
                return true;
            }
            break;
    }

    return false;
}

void CViewTransfers::GetDocProjectName(size_t item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    FILE_TRANSFER* transfer = 0;
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    if (transfer) {
        strBuffer = HtmlEntityDecode(wxString(transfer->project_name.c_str(), wxConvUTF8));
    } else {
        strBuffer = wxEmptyString;
    }
}

void CViewTransfers::GetDocFileName(size_t item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    FILE_TRANSFER* transfer = 0;
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    if (transfer) {
        strBuffer = wxString(transfer->name.c_str(), wxConvUTF8);
    } else {
        strBuffer = wxEmptyString;
    }
}

void CViewTransfers::GetDocProgress(size_t item, float& fBuffer) const {
    float          fBytesSent = 0;
    float          fFileSize = 0;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    FILE_TRANSFER* transfer = 0;
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    fBuffer = 0;
    if (transfer) {
        fBytesSent = transfer->bytes_xferred;
        fFileSize = transfer->nbytes;
    }

    // Curl apparently counts the HTTP header in byte count.
    // Prevent this from causing > 100% display
    if (fBytesSent > fFileSize) {
        fBytesSent = fFileSize;
    }

    if (fFileSize) {
        fBuffer = floor((fBytesSent / fFileSize) * 10000)/100;
    }
}

wxInt32 CViewTransfers::FormatProgress(float fBuffer, wxString& strBuffer) const {
    strBuffer.Printf(wxT("%.2f%%"), fBuffer);
    return 0;
}

void CViewTransfers::GetDocBytesXferred(size_t item, double& fBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    FILE_TRANSFER* transfer = 0;
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    if (transfer) {
        fBuffer = transfer->bytes_xferred;
    } else {
        fBuffer = 0.0;
    }
}

void CViewTransfers::GetDocTotalBytes(size_t item, double& fBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    FILE_TRANSFER* transfer = 0;
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    if (transfer) {
        fBuffer = transfer->nbytes;
    } else {
        fBuffer = 0.0;
    }
}

wxInt32 CViewTransfers::FormatSize(double fBytesSent, double fFileSize, wxString& strBuffer) const {
    const double xTera = 1099511627776.0;
    const double xGiga = 1073741824.0;
    const double xMega = 1048576.0;
    const double xKilo = 1024.0;

    if (fFileSize != 0) {
        if (fFileSize >= xTera) {
            strBuffer.Printf(wxT("%0.2f/%0.2f TB"), fBytesSent/xTera, fFileSize/xTera);
        } else if (fFileSize >= xGiga) {
            strBuffer.Printf(wxT("%0.2f/%0.2f GB"), fBytesSent/xGiga, fFileSize/xGiga);
        } else if (fFileSize >= xMega) {
            strBuffer.Printf(wxT("%0.2f/%0.2f MB"), fBytesSent/xMega, fFileSize/xMega);
        } else if (fFileSize >= xKilo) {
            strBuffer.Printf(wxT("%0.2f/%0.2f KB"), fBytesSent/xKilo, fFileSize/xKilo);
        } else {
            strBuffer.Printf(wxT("%0.0f/%0.0f bytes"), fBytesSent, fFileSize);
        }
    } else {
        if (fBytesSent >= xTera) {
            strBuffer.Printf(wxT("%0.2f TB"), fBytesSent/xTera);
        } else if (fBytesSent >= xGiga) {
            strBuffer.Printf(wxT("%0.2f GB"), fBytesSent/xGiga);
        } else if (fBytesSent >= xMega) {
            strBuffer.Printf(wxT("%0.2f MB"), fBytesSent/xMega);
        } else if (fBytesSent >= xKilo) {
            strBuffer.Printf(wxT("%0.2f KB"), fBytesSent/xKilo);
        } else {
            strBuffer.Printf(wxT("%0.0f bytes"), fBytesSent);
        }
    }
    return 0;
}

void CViewTransfers::GetDocTime(size_t item, double& fBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    FILE_TRANSFER* transfer = 0;
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    if (transfer) {
        fBuffer = transfer->time_so_far;
    } else {
        fBuffer = 0.0;
    }
}

wxInt32 CViewTransfers::FormatTime(float fBuffer, wxString& strBuffer) const {
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;

    iHour = (wxInt32)(fBuffer / (60 * 60));
    iMin  = (wxInt32)(fBuffer / 60) % 60;
    iSec  = (wxInt32)(fBuffer) % 60;

    ts = wxTimeSpan(iHour, iMin, iSec);

    strBuffer = ts.Format();

    return 0;
}

void CViewTransfers::GetDocSpeed(size_t item, double& fBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    FILE_TRANSFER* transfer = 0;
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    if (transfer) {
        if (transfer->xfer_active)
            fBuffer = transfer->xfer_speed / 1024;
        else
            fBuffer = 0.0;
    }
}

wxInt32 CViewTransfers::FormatSpeed(float fBuffer, wxString& strBuffer) const {
    strBuffer.Printf(wxT("%.2f KBps"), fBuffer);
    return 0;
}

void CViewTransfers::GetDocStatus(size_t item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    FILE_TRANSFER* transfer = 0;
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    CC_STATUS status;
    int retval = pDoc->GetCoreClientStatus(status);

    if ((transfer) && (!retval)) {
        wxDateTime dtNextRequest((time_t)transfer->next_request_time);
        wxDateTime dtNow(wxDateTime::Now());
        if (dtNextRequest > dtNow) {
            wxTimeSpan tsNextRequest(dtNextRequest - dtNow);
            strBuffer = _("Retry in ") + tsNextRequest.Format();
        } else if (ERR_GIVEUP_DOWNLOAD == transfer->status) {
            strBuffer = _("Download failed");
        } else if (ERR_GIVEUP_UPLOAD == transfer->status) {
            strBuffer = _("Upload failed");
        } else {
            if (status.network_suspend_reason) {
                strBuffer = _("Suspended");
            } else {
                if (transfer->xfer_active) {
                    strBuffer = transfer->generated_locally? _("Uploading") : _("Downloading");
                } else {
                    strBuffer = transfer->generated_locally? _("Upload pending") : _("Download pending");
                }
            }
        }
    }
}

double CViewTransfers::GetProgressValue(long item) {
    double          fBytesSent = 0;
    double          fFileSize = 0;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    FILE_TRANSFER* transfer = 0;
    if (pDoc) {
        transfer = pDoc->file_transfer(m_iSortedIndexes[item]);
    }

    if (transfer) {
        fBytesSent = transfer->bytes_xferred;
        fFileSize = transfer->nbytes;
    }

    // Curl apparently counts the HTTP header in byte count.
    // Prevent this from causing > 100% display
    if (fBytesSent > fFileSize) {
        fBytesSent = fFileSize;
    }

    if ( 0.0 == fFileSize ) {
        return 0.0;
    }

    return (fBytesSent / fFileSize);
}
