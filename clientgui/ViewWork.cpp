// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 David Barnard, Nicolas Alvarez, Peter Kortschack
// Copyright (C) 2009 University of California
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

#include "ViewWork.h"

#include "stdwx.h"

#include "AdvancedFrame.h"
#include "app_ipc.h"
#include "BOINCBaseFrame.h"
#include "BOINCGUIApp.h"
#include "BOINCListCtrl.h"
#include "BOINCTaskCtrl.h"
#include "error_numbers.h"
#include "Events.h"
#include "hyperlink.h"
#include "MainDocument.h"
#include "util.h"

#include "res/result.xpm"

#define COLUMN_PROJECT              0
#define COLUMN_APPLICATION          1
#define COLUMN_NAME                 2
#define COLUMN_CPUTIME              3
#define COLUMN_PROGRESS             4
#define COLUMN_TOCOMPLETION         5
#define COLUMN_REPORTDEADLINE       6
#define COLUMN_STATUS               7

// groups that contain buttons
#define GRP_TASKS    0
#define GRP_WEBSITES 1

// buttons in the "tasks" area
#define BTN_GRAPHICS                0
#define BTN_SUSPEND                 1
#define BTN_RESUME                  2
#define BTN_ABORT                   3

CWork::CWork() {
    m_fCPUTime          = -1.0f;
    m_fProgress         = -1.0f;
    m_fTimeToCompletion = -1.0f;
    m_tReportDeadline   = (time_t)0;
}

enum DlgButtons {
    Cancel      = 0x01,
    Yes         = 0x02,
    No          = 0x04,
    YesToAll    = 0x08,
};

IMPLEMENT_DYNAMIC_CLASS(DlgYesToAll, wxDialog)

BEGIN_EVENT_TABLE (DlgYesToAll, wxDialog)
    EVT_BUTTON(wxID_ANY, DlgYesToAll::OnButton)
END_EVENT_TABLE ()

DlgYesToAll::DlgYesToAll(wxWindow* parent, const wxString& caption, const wxString& message, long buttons)
                : wxDialog(parent, wxID_ANY, caption, wxDefaultPosition, wxDefaultSize, wxCAPTION) {
    wxBoxSizer* verticalSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* messageLabel = new wxStaticText(this, wxID_STATIC, message);
    verticalSizer->Add(messageLabel, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    if (buttons & YesToAll) {
        wxButton* yesToAllButton = new wxButton(this, wxID_YESTOALL, _("Yes to &All"));
        buttonSizer->Add(yesToAllButton, 0, wxALL, 5);
    }
    if (buttons & Yes) {
        wxButton* yesButton = new wxButton(this, wxID_YES);
        yesButton->SetDefault();
        buttonSizer->Add(yesButton, 0, wxALL, 5);
    }
    if (buttons & No) {
        wxButton* noButton = new wxButton(this, wxID_NO);
        buttonSizer->Add(noButton, 0, wxALL, 5);
    }
    if (buttons & Cancel) {
        wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
        buttonSizer->Add(cancelButton, 0, wxALL, 5);
    }

    verticalSizer->Add(buttonSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    verticalSizer->SetSizeHints(this);
    SetSizer(verticalSizer);
}

void DlgYesToAll::OnButton(wxCommandEvent& event) {
    EndModal(event.GetId());
}

IMPLEMENT_DYNAMIC_CLASS(CViewWork, CTaskViewBase)

BEGIN_EVENT_TABLE (CViewWork, CTaskViewBase)
    EVT_BUTTON(ID_TASK_WORK_SUSPEND, CViewWork::OnWorkSuspend)
    EVT_BUTTON(ID_TASK_WORK_RESUME, CViewWork::OnWorkResume)
    EVT_BUTTON(ID_TASK_WORK_SHOWGRAPHICS, CViewWork::OnWorkShowGraphics)
    EVT_BUTTON(ID_TASK_WORK_ABORT, CViewWork::OnWorkAbort)
    EVT_CUSTOM_RANGE(wxEVT_COMMAND_BUTTON_CLICKED, ID_TASK_PROJECT_WEB_PROJDEF_MIN, ID_TASK_PROJECT_WEB_PROJDEF_MAX, CViewWork::OnProjectWebsiteClicked)
    EVT_LIST_ITEM_FOCUSED(ID_LIST_WORKVIEW, CViewWork::OnListSelected)
    EVT_LIST_ITEM_SELECTED(ID_LIST_WORKVIEW, CViewWork::OnListSelected)
    EVT_LIST_COL_CLICK(ID_LIST_WORKVIEW, CViewWork::OnColClick)
    EVT_LIST_CACHE_HINT(ID_LIST_WORKVIEW, CViewWork::OnCacheHint)
END_EVENT_TABLE ()

static CViewWork* myCViewWork;

static bool CompareViewWorkItems(size_t iRowIndex1, size_t iRowIndex2) {
    CWork*          work1 = myCViewWork->m_WorkCache.at(iRowIndex1);
    CWork*          work2 = myCViewWork->m_WorkCache.at(iRowIndex2);
    int             result = 0;
    
    switch (myCViewWork->m_iSortColumn) {
        case COLUMN_PROJECT:
            result = work1->m_strProjectName.CmpNoCase(work2->m_strProjectName);
            break;
        case COLUMN_APPLICATION:
            result = work1->m_strApplicationName.CmpNoCase(work2->m_strApplicationName);
            break;
        case COLUMN_NAME:
            result = work1->m_strName.CmpNoCase(work2->m_strName);
            break;
        case COLUMN_CPUTIME:
            if (work1->m_fCPUTime < work2->m_fCPUTime) {
                result = -1;
            } else if (work1->m_fCPUTime > work2->m_fCPUTime) {
                result = 1;
            }
            break;
        case COLUMN_PROGRESS:
            if (work1->m_fProgress < work2->m_fProgress) {
                result = -1;
            } else if (work1->m_fProgress > work2->m_fProgress) {
                result = 1;
            }
            break;
        case COLUMN_TOCOMPLETION:
            if (work1->m_fTimeToCompletion < work2->m_fTimeToCompletion) {
                result = -1;
            } else if (work1->m_fTimeToCompletion > work2->m_fTimeToCompletion) {
                result = 1;
            }
            break;
        case COLUMN_REPORTDEADLINE:
            if (work1->m_tReportDeadline < work2->m_tReportDeadline) {
                result = -1;
            } else if (work1->m_tReportDeadline > work2->m_tReportDeadline) {
                result = 1;
            }
            break;
        case COLUMN_STATUS:
            result = work1->m_strStatus.CmpNoCase(work2->m_strStatus);
            break;
    }

    // Always return false for equality (result == 0).
    return ((myCViewWork->m_bReverseSort) ? (result > 0) : (result < 0));
}

CViewWork::CViewWork() {
}

CViewWork::CViewWork(wxNotebook* pNotebook) : CTaskViewBase(pNotebook) {
}

CViewWork::~CViewWork() {
    EmptyCache();
}

void CViewWork::DemandLoadView() {
    wxASSERT(!m_bViewLoaded);

    CTaskViewBase::DemandLoadView(ID_TASK_WORKVIEW, DEFAULT_TASK_FLAGS, ID_LIST_WORKVIEW, DEFAULT_LIST_MULTI_SEL_FLAGS);

    CTaskItemGroup* pGroup = NULL;
    CTaskItem*      pItem = NULL;

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    // Setup View
    pGroup = new CTaskItemGroup(_("Commands"));
    m_TaskGroups.push_back(pGroup);

    pItem = new CTaskItem(_("Show graphics"), _("Show application graphics in a window."),
                            ID_TASK_WORK_SHOWGRAPHICS);
    pGroup->m_Tasks.push_back(pItem);

    pItem = new CTaskItem(_("Suspend"), _("Suspend work for this result."),
                            ID_TASK_WORK_SUSPEND);
    pGroup->m_Tasks.push_back(pItem);

    pItem = new CTaskItem(_("Resume"), _("Resume work for this result."),
                            ID_TASK_WORK_RESUME);
    pGroup->m_Tasks.push_back(pItem);

    pItem = new CTaskItem(_("Abort"),
                            _("Abandon work on the result. "
                              "You will get no credit for it."),
                            ID_TASK_WORK_ABORT);
    pGroup->m_Tasks.push_back(pItem);

    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

    // Create List Pane Items
    AddColumn(COLUMN_PROJECT,        _T("Project"), wxLIST_FORMAT_LEFT, 125);
    AddColumn(COLUMN_APPLICATION,    _T("Application"), wxLIST_FORMAT_LEFT, 95);
    AddColumn(COLUMN_NAME,           _T("Name"), wxLIST_FORMAT_LEFT, 285);
    AddColumn(COLUMN_CPUTIME,        _T("CPU time"), wxLIST_FORMAT_RIGHT, 80);
    AddColumn(COLUMN_PROGRESS,       _T("Progress"), wxLIST_FORMAT_CENTER, 60);
    AddColumn(COLUMN_TOCOMPLETION,   _T("To completion"), wxLIST_FORMAT_RIGHT, 100);
    AddColumn(COLUMN_REPORTDEADLINE, _T("Report deadline"), wxLIST_FORMAT_LEFT, 150);
    AddColumn(COLUMN_STATUS,         _T("Status"), wxLIST_FORMAT_LEFT, 135);

    m_iProgressColumn = COLUMN_PROGRESS;

    // Needed by static sort routine;
    myCViewWork = this;
    m_funcSortCompare = CompareViewWorkItems;

    RestoreState();

    UpdateSelection();
}

const wxString& CViewWork::GetViewName() {
    static wxString strViewName(wxT("Tasks"));
    return strViewName;
}

const wxString& CViewWork::GetViewDisplayName() {
    static wxString strViewName(_("Tasks"));
    return strViewName;
}

const char** CViewWork::GetViewIcon() {
    return result_xpm;
}

void CViewWork::OnWorkSuspend(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkSuspend - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) {
            break;
        }
        
        RESULT* result = pDoc->result(m_iSortedIndexes.at(row));
        if (result) {
            if (!result->suspended_via_gui) {
                pFrame->UpdateStatusText(_("Suspending task..."));
                pDoc->WorkSuspend(result->project_url, result->name);
            }
        }
    }
    pFrame->UpdateStatusText(wxT(""));
    
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkSuspend - Function End"));
}

void CViewWork::OnWorkResume(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkResume - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) {
            break;
        }
        
        RESULT* result = pDoc->result(m_iSortedIndexes.at(row));
        if (result) {
            if (result->suspended_via_gui) {
                pFrame->UpdateStatusText(_("Resuming task..."));
                pDoc->WorkResume(result->project_url, result->name);
            }
        }
    }
    pFrame->UpdateStatusText(wxT(""));
    
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkResume - Function End"));
}

void CViewWork::OnWorkShowGraphics(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkShowGraphics - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

    pFrame->UpdateStatusText(_("Showing graphics for task..."));

    int row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) {
            break;
        }
        
        RESULT* result = pDoc->result(m_iSortedIndexes.at(row));
        if (result) {
            pDoc->WorkShowGraphics(result);
        }
    }
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkShowGraphics - Function End"));
}

void CViewWork::OnWorkAbort( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkAbort - Function Begin"));

    wxInt32  iAnswer        = 0;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    long buttons = Yes | No;
    bool yesToAll = false;

    wxASSERT(pDoc);
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Aborting result..."));

    if (m_pListPane->GetSelectedItemCount() > 1) {
        buttons |= YesToAll | Cancel;
    }

    int row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;

        if (!yesToAll) {
            CWork* work = m_WorkCache.at(m_iSortedIndexes.at(row));

            strMessage.Printf(_("Are you sure you want to abort task '%s'?\n"
                                "(Progress: %s, Status: %s)"), work->m_strName.c_str(),
                                work->m_strProgress.c_str(), work->m_strStatus.c_str());

            DlgYesToAll dlg(this, _("Abort task"), strMessage, buttons);

            iAnswer = dlg.ShowModal();

            if (wxID_NO == iAnswer) {
                continue;
            }
            if (wxID_CANCEL == iAnswer) {
                break;
            }

            if (wxID_YESTOALL == iAnswer) {
                yesToAll = true;
            }
        }

        // Abort the result:
        RESULT* result = pDoc->result(m_iSortedIndexes.at(row));
        if (result) {
            pDoc->WorkAbort(result->project_url, result->name);
        }
    }

    pFrame->UpdateStatusText(wxEmptyString);

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkAbort - Function End"));
}

void CViewWork::OnProjectWebsiteClicked(wxEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnProjectWebsiteClicked - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    pFrame->UpdateStatusText(_("Launching browser..."));

    int website_task_index = event.GetId() - ID_TASK_PROJECT_WEB_PROJDEF_MIN;
    HyperLink::ExecuteLink(m_TaskGroups[1]->m_Tasks[website_task_index]->m_strWebSiteLink);

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnProjectWebsiteClicked - Function End"));
}

wxInt32 CViewWork::GetDocCount() {
    return static_cast<wxInt32>(wxGetApp().GetDocument()->GetWorkCount());
}

wxString CViewWork::OnListGetItemText(long item, long column) const {
    wxString       strBuffer = wxEmptyString;

    CWork* work = m_WorkCache.at(m_iSortedIndexes.at(item));

    switch (column) {
        case COLUMN_PROJECT:
            strBuffer = work->m_strProjectName;
            break;
        case COLUMN_APPLICATION:
            strBuffer = work->m_strApplicationName;
            break;
        case COLUMN_NAME:
            strBuffer = work->m_strName;
            break;
        case COLUMN_CPUTIME:
            strBuffer = work->m_strCPUTime;
            break;
        case COLUMN_PROGRESS:
            strBuffer = work->m_strProgress;
            break;
        case COLUMN_TOCOMPLETION:
            strBuffer = work->m_strTimeToCompletion;
            break;
        case COLUMN_REPORTDEADLINE:
            strBuffer = work->m_strReportDeadline;
            break;
        case COLUMN_STATUS:
            strBuffer = work->m_strStatus;
            break;
    }
    return strBuffer;
}

wxInt32 CViewWork::AddCacheElement() {
    CWork* pItem = new CWork();
    wxASSERT(pItem);
    if (pItem) {
        m_WorkCache.push_back(pItem);
        m_iSortedIndexes.push_back((int)m_WorkCache.size()-1);
        return 0;
    }
    return -1;
}

wxInt32 CViewWork::EmptyCache() {
    unsigned int i;
    for (i=0; i<m_WorkCache.size(); i++) {
        delete m_WorkCache[i];
    }
    m_WorkCache.clear();
    m_iSortedIndexes.clear();
    return 0;
}

wxInt32 CViewWork::GetCacheCount() {
    return (wxInt32)m_WorkCache.size();
}

wxInt32 CViewWork::RemoveCacheElement() {
    delete m_WorkCache.back();
    m_WorkCache.erase(m_WorkCache.end() - 1);
    m_iSortedIndexes.clear();
    for (size_t i = 0; i < m_WorkCache.size(); ++i) {
        m_iSortedIndexes.push_back(i);
    }
    return 0;
}

void CViewWork::UpdateSelection() {
    PROJECT*            project = NULL;
    CMainDocument*      pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);

    CTaskViewBase::PreUpdateSelection();

    CTaskItemGroup* pGroup = m_TaskGroups[0];

    int row = -1;
    int n = m_pListPane->GetSelectedItemCount();
    if (n == 1) {
        // Single selection
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        RESULT* result = pDoc->result(m_iSortedIndexes.at(row));

        if (result) {
            // Start with everything enabled.
            m_pTaskPane->EnableTaskGroupTasks(pGroup);

            // Disable Show Graphics button if selected task can't display graphics
            if (((!result->supports_graphics) || pDoc->GetState()->executing_as_daemon) 
                && result->graphics_exec_path.empty()) {

                m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_GRAPHICS]);
            } else {
                // Disable graphics if the manager is connected to a remote client:
                wxString strMachineName;
                pDoc->GetConnectedComputerName(strMachineName);
                if (!pDoc->IsComputerNameLocal(strMachineName)) {
                    m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_GRAPHICS]);
                }
            }

            // Disable the "Abort" button if the selected task already is aborted:
            if (result->active_task_state == PROCESS_ABORT_PENDING ||
                result->active_task_state == PROCESS_ABORTED ||
                result->state == RESULT_ABORTED) {

                m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_ABORT]);
                // Disabling the "Abort" button means that all selected tasks are
                // already aborted. Suspending or resuming them makes no sense
                // therefore disable these two buttons, too:
                m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_SUSPEND]);
                m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_RESUME]);
            } else {
                // Disable Suspend / Resume button
                if (result->suspended_via_gui) {
                    m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_SUSPEND]);
                } else {
                    m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_RESUME]);
                }
            }
            project = pDoc->state.lookup_project(result->project_url);
        }
    } else if (n > 1) {
        // Multiple selection.
        // Check if all selected tasks are in the same state.
        // If yes allow only the buttons that should be allowed for this task
        // state, otherwise allow everything. The graphics button will always
        // be disabled.
        m_pTaskPane->EnableTaskGroupTasks(pGroup);
        m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_GRAPHICS]);

        // First gather the needed information by iterating over all selected
        // tasks:
        int num_suspended = 0;
        int num_aborted = 0;
        while (true) {
            row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (row < 0) {
                break;
            }
            RESULT* result = pDoc->result(m_iSortedIndexes.at(row));

            if (result) {
                // Check aborted tasks:
                if ((result->active_task_state == PROCESS_ABORT_PENDING)
                    || (result->active_task_state == PROCESS_ABORTED)
                    || (result->state == RESULT_ABORTED)) {
                        ++num_aborted;
                } else if (result->suspended_via_gui) { // Check suspended tasks
                    ++num_suspended;
                }
            }

        }

        // Check if the "Abort" button needs to be disabled:
        if (num_aborted == n) {
            m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_ABORT]);

            // Disabling the "Abort" button means that all selected tasks are
            // already aborted. Suspending or resuming them makes no sense
            // therefore disable these two buttons, too:
            m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_SUSPEND]);
            m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_RESUME]);
        } else {
            // Check the "Suspend"/"Resume" buttons:
            if (num_suspended == 0) {
                // None of the selected tasks is suspended.
                // => Disable the "Resume" button.
                m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_RESUME]);
            } else if (num_suspended == (n - num_aborted)) {
                // All selected tasks which are not aborted are suspended.
                // => Disable the "Suspend" button.
                m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_SUSPEND]);
            } // else: Leave both buttons enabled.
        }
    } else {
        // No selection
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
    }

    UpdateWebsiteSelection(GRP_WEBSITES, project);
    CTaskViewBase::PostUpdateSelection();
}

bool CViewWork::SynchronizeCacheItem(wxInt32 iRowIndex, wxInt32 iColumnIndex) {
    wxString    strDocumentText  = wxEmptyString;
    float       fDocumentFloat = 0.0;
    time_t      tDocumentTime = (time_t)0;
    CWork*      work = m_WorkCache.at(m_iSortedIndexes.at(iRowIndex));

    strDocumentText.Empty();

    switch (iColumnIndex) {
        case COLUMN_PROJECT:
            GetDocProjectName(m_iSortedIndexes.at(iRowIndex), strDocumentText);
            if (!strDocumentText.IsSameAs(work->m_strProjectName)) {
                work->m_strProjectName = strDocumentText;
                return true;
            }
            break;
        case COLUMN_APPLICATION:
            GetDocApplicationName(m_iSortedIndexes.at(iRowIndex), strDocumentText);
            if (!strDocumentText.IsSameAs(work->m_strApplicationName)) {
                work->m_strApplicationName = strDocumentText;
                return true;
            }
            break;
        case COLUMN_NAME:
            GetDocName(m_iSortedIndexes.at(iRowIndex), strDocumentText);
            if (!strDocumentText.IsSameAs(work->m_strName)) {
                work->m_strName = strDocumentText;
                return true;
            }
            break;
        case COLUMN_CPUTIME:
            GetDocCPUTime(m_iSortedIndexes.at(iRowIndex), fDocumentFloat);
            if (fDocumentFloat != work->m_fCPUTime) {
                work->m_fCPUTime = fDocumentFloat;
                FormatCPUTime(fDocumentFloat, work->m_strCPUTime);
                return true;
            }
            break;
        case COLUMN_PROGRESS:
            GetDocProgress(m_iSortedIndexes.at(iRowIndex), fDocumentFloat);
            if (fDocumentFloat != work->m_fProgress) {
                work->m_fProgress = fDocumentFloat;
                FormatProgress(fDocumentFloat, work->m_strProgress);
                return true;
            }
            break;
        case COLUMN_TOCOMPLETION:
            GetDocTimeToCompletion(m_iSortedIndexes.at(iRowIndex), fDocumentFloat);
            if (fDocumentFloat != work->m_fTimeToCompletion) {
                work->m_fTimeToCompletion = fDocumentFloat;
                FormatTimeToCompletion(fDocumentFloat, work->m_strTimeToCompletion);
                return true;
            }
            break;
        case COLUMN_REPORTDEADLINE:
            GetDocReportDeadline(m_iSortedIndexes.at(iRowIndex), tDocumentTime);
            if (tDocumentTime != work->m_tReportDeadline) {
                work->m_tReportDeadline = tDocumentTime;
                FormatReportDeadline(tDocumentTime, work->m_strReportDeadline);
                return true;
            }
            break;
        case COLUMN_STATUS:
            GetDocStatus(m_iSortedIndexes.at(iRowIndex), strDocumentText);
            if (!strDocumentText.IsSameAs(work->m_strStatus)) {
                work->m_strStatus = strDocumentText;
                return true;
            }
            break;
    }
    return false;
}

void CViewWork::GetDocProjectName(size_t item, wxString& strBuffer) const {
    CMainDocument* doc = wxGetApp().GetDocument();
    RESULT* result = wxGetApp().GetDocument()->result(item);
    PROJECT* state_project = NULL;
    std::string project_name;

    wxASSERT(doc);
    wxASSERT(wxDynamicCast(doc, CMainDocument));

    if (result) {
        state_project = doc->state.lookup_project(result->project_url);
        if (state_project) {
            state_project->get_name(project_name);
            strBuffer = HtmlEntityDecode(wxString(project_name.c_str(), wxConvUTF8));
         } else {
            doc->ForceCacheUpdate();
        }
    }
}

void CViewWork::GetDocApplicationName(size_t item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    RESULT* result = 0;
    if (pDoc) {
        result = pDoc->result(item);
    }

    if (result) {
        RESULT* state_result = pDoc->state.lookup_result(result->project_url, result->name);
        if (!state_result) {
            pDoc->ForceCacheUpdate();
            state_result = pDoc->state.lookup_result(result->project_url, result->name);
        }
        if (!state_result) {
            return;
        }
        WORKUNIT* wup = state_result->wup;
        if (!wup) {
            return;
        }
        APP* app = wup->app;
        if (!app) {
            return;
        }

        wxString strLocalBuffer;
        wxString strLocale = wxString(setlocale(LC_NUMERIC, NULL), wxConvUTF8);
        setlocale(LC_NUMERIC, "C");
        if (!app->user_friendly_name.empty()) {
            strLocalBuffer = HtmlEntityDecode(wxString(state_result->app->user_friendly_name.c_str(), wxConvUTF8));
        } else {
            strLocalBuffer = HtmlEntityDecode(wxString(wup->avp->app_name.c_str(), wxConvUTF8));
        }
        APP_VERSION* avp = wup->avp;
        if (avp) {
            if (avp->plan_class.empty()) {
                strBuffer.Printf(wxT("%s %.2f"), strLocalBuffer.c_str(), avp->version_num / 100.0);
            } else {
                wxString planClass = wxString(avp->plan_class.c_str(), wxConvUTF8);
                strBuffer.Printf(wxT("%s %.2f (%s)"), strLocalBuffer.c_str(),
                                 avp->version_num / 100.0, planClass.c_str());
            }
        }
        setlocale(LC_NUMERIC, (const char*)strLocale.mb_str());
    }
}

void CViewWork::GetDocName(size_t item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    RESULT* result = 0;
    if (pDoc) {
        result = pDoc->result(item);
        if (result) {
            strBuffer = wxString(result->name.c_str(), wxConvUTF8);
        }
    }
}

void CViewWork::GetDocCPUTime(size_t item, float& fBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    RESULT* result = 0;
    if (pDoc) {
        result = pDoc->result(item);
    }

    fBuffer = 0;
    if (result) {
        if (result->active_task) {
            fBuffer = result->current_cpu_time;
        } else {
            if(result->state < RESULT_COMPUTE_ERROR) {
                fBuffer = 0;
            } else {
                fBuffer = result->final_cpu_time;
            }
        }
    }
}


wxInt32 CViewWork::FormatCPUTime(float fBuffer, wxString& strBuffer) const {
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;
    
    if (0 == fBuffer) {
        strBuffer = wxT("---");
    } else {
        iHour = (wxInt32)(fBuffer / (60 * 60));
        iMin  = (wxInt32)(fBuffer / 60) % 60;
        iSec  = (wxInt32)(fBuffer) % 60;

        ts = wxTimeSpan(iHour, iMin, iSec);

        strBuffer = ts.Format();
    }

    return 0;
}

void CViewWork::GetDocProgress(size_t item, float& fBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    RESULT* result = 0;
    if (pDoc) {
        result = pDoc->result(item);
    }

    fBuffer = 0;
    if (result) {
        if (result->active_task) {
            fBuffer = floor(result->fraction_done * 100000)/1000;
        } else {
            if(result->state < RESULT_COMPUTE_ERROR) {
                fBuffer = 0.0;
            } else {
                fBuffer = 100.0;
            }
        }
    }
}

wxInt32 CViewWork::FormatProgress(float fBuffer, wxString& strBuffer) const {
    strBuffer.Printf(wxT("%.3f%%"), fBuffer);
    return 0;
}

void CViewWork::GetDocTimeToCompletion(size_t item, float& fBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    RESULT* result = 0;
    if (pDoc) {
        result = pDoc->result(item);
    }

    fBuffer = 0;
    if (result) {
        fBuffer = result->estimated_cpu_time_remaining;
    }
}

wxInt32 CViewWork::FormatTimeToCompletion(float fBuffer, wxString& strBuffer) const {
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;

    if (fBuffer > 86400.0 * 365.0 * 10.0) {
        fBuffer = 86400.0 * 365.0 * 10.0;
    }
    if (0 >= fBuffer) {
        strBuffer = wxT("---");
    } else {
        iHour = (wxInt32)(fBuffer / (60 * 60));
        iMin  = (wxInt32)(fBuffer / 60) % 60;
        iSec  = (wxInt32)(fBuffer) % 60;

        ts = wxTimeSpan(iHour, iMin, iSec);

        strBuffer = ts.Format();
    }

    return 0;
}

void CViewWork::GetDocReportDeadline(size_t item, time_t& time) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    RESULT* result = 0;
    if (pDoc) {
        result = pDoc->result(item);
        if (result) {
            time = result->report_deadline;
        } else {
            time = (time_t)0;
        }
    }
}

wxInt32 CViewWork::FormatReportDeadline(time_t deadline, wxString& strBuffer) const {
    wxDateTime     dtTemp;
    dtTemp.Set(deadline);
    strBuffer = dtTemp.Format();
    return 0;
}

void CViewWork::GetDocStatus(size_t item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    RESULT* result = 0;
    if (pDoc) {
        result = pDoc->result(item);
    }
    CC_STATUS status;
    int retval = pDoc->GetCoreClientStatus(status);

    if ((retval) || (!result)) {
        strBuffer.Clear();
        return;
    }
    int throttled = status.task_suspend_reason & SUSPEND_REASON_CPU_USAGE_LIMIT;
    switch(result->state) {
    case RESULT_NEW:
        strBuffer = _("New"); 
        break;
    case RESULT_FILES_DOWNLOADING:
        if (result->ready_to_report) {
            strBuffer = _("Download failed");
        } else {
            strBuffer = _("Downloading");
        }
        break;
    case RESULT_FILES_DOWNLOADED:
        if (result->project_suspended_via_gui) {
            strBuffer = _("Project suspended by user");
        } else if (result->suspended_via_gui) {
            strBuffer = _("Task suspended by user");
        } else if (status.task_suspend_reason && !throttled) {
            strBuffer = _("Suspended");
            if (status.task_suspend_reason & SUSPEND_REASON_BATTERIES) {
                strBuffer += _(" - on batteries");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_USER_ACTIVE) {
                strBuffer += _(" - user active");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_USER_REQ) {
                strBuffer += _(" - computation suspended");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_TIME_OF_DAY) {
                strBuffer += _(" - time of day");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_BENCHMARKS) {
                strBuffer += _(" - CPU benchmarks");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_DISK_SIZE) {
                strBuffer += _(" - need disk space");
            }
        } else if (result->active_task) {
            if (result->too_large) {
                strBuffer = _("Waiting for memory");
            } else if (result->needs_shmem) {
                strBuffer = _("Waiting for shared memory");
            } else if (result->scheduler_state == CPU_SCHED_SCHEDULED) {
                if (result->edf_scheduled) {
                    strBuffer = _("Running, high priority");
                } else {
                    strBuffer = _("Running");
                }
#if 0
                // doesn't work - result pointer not there
                if (result->project->non_cpu_intensive) {
                    strBuffer += _(" (non-CPU-intensive)");
                }
#endif
            } else if (result->scheduler_state == CPU_SCHED_PREEMPTED) {
                strBuffer = _("Waiting to run");
            } else if (result->scheduler_state == CPU_SCHED_UNINITIALIZED) {
                strBuffer = _("Ready to start");
            }
        } else {
            strBuffer = _("Ready to start");
        }
        break;
    case RESULT_COMPUTE_ERROR:
        strBuffer = _("Computation error");
        break;
    case RESULT_FILES_UPLOADING:
        if (result->ready_to_report) {
            strBuffer = _("Upload failed");
        } else {
            strBuffer = _("Uploading");
        }
        break;
    case RESULT_ABORTED:
        switch(result->exit_status) {
        case ERR_ABORTED_VIA_GUI:
            strBuffer = _("Aborted by user");
            break;
        case ERR_ABORTED_BY_PROJECT:
            strBuffer = _("Aborted by project");
            break;
        default:
            strBuffer = _("Aborted");
        }
        break;
    default:
        if (result->got_server_ack) {
            strBuffer = _("Acknowledged");
        } else if (result->ready_to_report) {
            strBuffer = _("Ready to report");
        } else {
            strBuffer.Format(_("Error: invalid state '%d'"), result->state);
        }
        break;
    }
}

wxInt32 CViewWork::FormatStatus(wxInt32 item, wxString& strBuffer) const {
    CWork*          work = m_WorkCache.at(m_iSortedIndexes.at(item));
    strBuffer = work->m_strStatus;
    return 0;
}

double CViewWork::GetProgressValue(long item) {
    float          fBuffer = 0;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    RESULT* result = 0;
    if (pDoc) {
        result = pDoc->result(m_iSortedIndexes[item]);
    }

    if (result) {
        if (result->active_task) {
            fBuffer = result->fraction_done;

            // Make sure that the progress value is between 0% and 100%:
            if (fBuffer > 1.0) {
                fBuffer = 1.0;
            } else if (fBuffer < 0.0) {
                fBuffer = 0.0;
            }
        } else {
            if(result->state < RESULT_COMPUTE_ERROR) {
                fBuffer = 0.0;
            } else {
                fBuffer = 1.0;
            }
        }
    }

    return fBuffer;
}
