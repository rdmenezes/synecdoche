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

#include "ViewProjects.h"

#include "stdwx.h"

#include "AdvancedFrame.h"
#include "BOINCBaseFrame.h"
#include "BOINCGUIApp.h"
#include "BOINCListCtrl.h"
#include "BOINCTaskCtrl.h"
#include "Events.h"
#include "hyperlink.h"
#include "MainDocument.h"
#include "str_util.h"

#include "res/proj.xpm"

#define COLUMN_PROJECT              0
#define COLUMN_ACCOUNTNAME          1
#define COLUMN_TEAMNAME             2
#define COLUMN_TOTALCREDIT          3
#define COLUMN_AVGCREDIT            4
#define COLUMN_RESOURCESHARE        5
#define COLUMN_STATUS               6

// groups that contain buttons
#define GRP_TASKS    0
#define GRP_WEBSITES 1

// buttons in the "tasks" area
#define BTN_UPDATE       0
#define BTN_SUSPEND      1
#define BTN_RESUME       2
#define BTN_NOWORK       3
#define BTN_ALLOWWORK    4
#define BTN_RESET        5
#define BTN_DETACH       6

CProject::CProject() {
    m_fTotalCredit     = -1.0f;
    m_fAVGCredit       = -1.0f;
    m_fResourceShare   = -1.0f;
    m_fResourcePercent = -1.0f;
}

IMPLEMENT_DYNAMIC_CLASS(CViewProjects, CTaskViewBase)

BEGIN_EVENT_TABLE (CViewProjects, CTaskViewBase)
    EVT_BUTTON(ID_TASK_PROJECT_UPDATE, CViewProjects::OnProjectUpdate)
    EVT_BUTTON(ID_TASK_PROJECT_SUSPEND, CViewProjects::OnProjectSuspend)
    EVT_BUTTON(ID_TASK_PROJECT_RESUME, CViewProjects::OnProjectResume)
    EVT_BUTTON(ID_TASK_PROJECT_NONEWWORK, CViewProjects::OnProjectNoNewWork)
    EVT_BUTTON(ID_TASK_PROJECT_ALLOWNEWWORK, CViewProjects::OnProjectAllowNewWork)
    EVT_BUTTON(ID_TASK_PROJECT_RESET, CViewProjects::OnProjectReset)
    EVT_BUTTON(ID_TASK_PROJECT_DETACH, CViewProjects::OnProjectDetach)
    EVT_CUSTOM_RANGE(wxEVT_COMMAND_BUTTON_CLICKED, ID_TASK_PROJECT_WEB_PROJDEF_MIN, ID_TASK_PROJECT_WEB_PROJDEF_MAX, CViewProjects::OnProjectWebsiteClicked)
    EVT_LIST_ITEM_FOCUSED(ID_LIST_PROJECTSVIEW, CViewProjects::OnListSelected)
    EVT_LIST_ITEM_SELECTED(ID_LIST_PROJECTSVIEW, CViewProjects::OnListSelected)
    EVT_LIST_COL_CLICK(ID_LIST_PROJECTSVIEW, CViewProjects::OnColClick)
    EVT_LIST_CACHE_HINT(ID_LIST_PROJECTSVIEW, CViewProjects::OnCacheHint)
END_EVENT_TABLE ()

static CViewProjects* myCViewProjects;

static bool CompareViewProjectsItems(size_t iRowIndex1, size_t iRowIndex2) {
    CProject*  project1 = myCViewProjects->m_ProjectCache.at(iRowIndex1);
    CProject*  project2 = myCViewProjects->m_ProjectCache.at(iRowIndex2);
    int        result = 0;
    
    switch (myCViewProjects->m_iSortColumn) {
        case COLUMN_PROJECT:
            result = project1->m_strProjectName.CmpNoCase(project2->m_strProjectName);
            break;
        case COLUMN_ACCOUNTNAME:
            result = project1->m_strAccountName.CmpNoCase(project2->m_strAccountName);
            break;
        case COLUMN_TEAMNAME:
            result = project1->m_strTeamName.CmpNoCase(project2->m_strTeamName);
            break;
        case COLUMN_TOTALCREDIT:
            if (project1->m_fTotalCredit < project2->m_fTotalCredit) {
                result = -1;
            } else if (project1->m_fTotalCredit > project2->m_fTotalCredit) {
                result = 1;
            }
            break;
        case COLUMN_AVGCREDIT:
            if (project1->m_fAVGCredit < project2->m_fAVGCredit) {
                result = -1;
            } else if (project1->m_fAVGCredit > project2->m_fAVGCredit) {
                result = 1;
            }
            break;
        case COLUMN_RESOURCESHARE:
            if (project1->m_fResourceShare < project2->m_fResourceShare) {
                result = -1;
            } else if (project1->m_fResourceShare > project2->m_fResourceShare) {
                result = 1;
            }
            break;
        case COLUMN_STATUS:
            result = project1->m_strStatus.CmpNoCase(project2->m_strStatus);
            break;
    }

    // Always return false for equality (result == 0).
    return ((myCViewProjects->m_bReverseSort) ? (result > 0) : (result < 0));
}

CViewProjects::CViewProjects() {
}

CViewProjects::CViewProjects(wxNotebook* pNotebook) : CTaskViewBase(pNotebook) {
}

CViewProjects::~CViewProjects() {
    EmptyCache();
}

void CViewProjects::DemandLoadView() {

    wxASSERT(!m_bViewLoaded);

    CTaskViewBase::DemandLoadView(ID_TASK_PROJECTSVIEW, DEFAULT_TASK_FLAGS, ID_LIST_PROJECTSVIEW, DEFAULT_LIST_MULTI_SEL_FLAGS);

    CTaskItemGroup* pGroup = NULL;
    CTaskItem*      pItem = NULL;

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    // Setup View
    pGroup = new CTaskItemGroup( _("Commands") );
    m_TaskGroups.push_back( pGroup );

    pItem = new CTaskItem(_("Update"),
                            _("Report all completed tasks, get latest credit, "
                              "get latest preferences, and possibly get more tasks."),
                            ID_TASK_PROJECT_UPDATE);
    pGroup->m_Tasks.push_back(pItem);

    pItem = new CTaskItem(_("Suspend"), _("Suspend tasks for this project."),
                            ID_TASK_PROJECT_SUSPEND);
    pGroup->m_Tasks.push_back(pItem);

    pItem = new CTaskItem(_("Resume"), _("Resume tasks for this project."),
                            ID_TASK_PROJECT_RESUME);
    pGroup->m_Tasks.push_back(pItem);

    pItem = new CTaskItem(_("No new tasks"), _("Don't get new tasks for this project."),
                            ID_TASK_PROJECT_NONEWWORK);
    pGroup->m_Tasks.push_back(pItem);

    pItem = new CTaskItem(_("Allow new tasks"), _("Allow fetching new tasks for this project."),
                            ID_TASK_PROJECT_ALLOWNEWWORK);
    pGroup->m_Tasks.push_back(pItem);

    pItem = new CTaskItem(_("Reset project"),
                            _("Delete all files and tasks associated with this project, "
                              "and get new tasks.  "
                              "You can update the project "
                              "first to report any completed tasks."),
                            ID_TASK_PROJECT_RESET);
    pGroup->m_Tasks.push_back(pItem);

    pItem = new CTaskItem(_("Detach"),
                            _("Detach computer from this project.  "
                              "Tasks in progress will be lost "
                              "(use 'Update' first to report any completed tasks)."),
                            ID_TASK_PROJECT_DETACH);
    pGroup->m_Tasks.push_back(pItem);

    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

    // Create List Pane Items
    AddColumn(COLUMN_PROJECT,       _T("Project"), wxLIST_FORMAT_LEFT, 150);
    AddColumn(COLUMN_ACCOUNTNAME,   _T("Account"), wxLIST_FORMAT_LEFT, 80);
    AddColumn(COLUMN_TEAMNAME,      _T("Team"), wxLIST_FORMAT_LEFT, 80);
    AddColumn(COLUMN_TOTALCREDIT,   _T("Work done"), wxLIST_FORMAT_RIGHT, 80);
    AddColumn(COLUMN_AVGCREDIT,     _T("Avg. work done"), wxLIST_FORMAT_RIGHT, 80);
    AddColumn(COLUMN_RESOURCESHARE, _T("Resource share"), wxLIST_FORMAT_CENTRE, 85);
    AddColumn(COLUMN_STATUS,        _T("Status"), wxLIST_FORMAT_LEFT, 150);

    m_iProgressColumn = COLUMN_RESOURCESHARE;

    // Needed by static sort routine;
    myCViewProjects = this;
    m_funcSortCompare = CompareViewProjectsItems;

    RestoreState();

    UpdateSelection();
}

const wxString& CViewProjects::GetViewName() {
    static wxString strViewName(wxT("Projects"));
    return strViewName;
}

const wxString& CViewProjects::GetViewDisplayName() {
    static wxString strViewName(_("Projects"));
    return strViewName;
}

const char** CViewProjects::GetViewIcon() {
    return proj_xpm;
}


void CViewProjects::OnProjectUpdate( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectUpdate - Function Begin"));

    CMainDocument*  pDoc   = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    pFrame->UpdateStatusText(_("Updating project..."));
    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        pDoc->ProjectUpdate(m_iSortedIndexes[row]);
    }
    pFrame->UpdateStatusText(wxT(""));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->ResetReminderTimers();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectUpdate - Function End"));
}

void CViewProjects::OnProjectSuspend( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectSuspend - Function Begin"));

    CMainDocument*  pDoc   = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;

        PROJECT* project = pDoc->project(m_iSortedIndexes[row]);
        if (project) {
            if (!project->suspended_via_gui) {
                pFrame->UpdateStatusText(_("Suspending project..."));
                pDoc->ProjectSuspend(m_iSortedIndexes[row]);
            }
        }
    }
    pFrame->UpdateStatusText(wxT(""));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectSuspend - Function End"));
}

void CViewProjects::OnProjectResume( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectResume - Function Begin"));

    CMainDocument*  pDoc   = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;

        PROJECT* project = pDoc->project(m_iSortedIndexes[row]);
        if (project) {
            if (project->suspended_via_gui) {
                pFrame->UpdateStatusText(_("Resuming project..."));
                pDoc->ProjectResume(m_iSortedIndexes[row]);
            }
        }
    }
    pFrame->UpdateStatusText(wxT(""));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectResume - Function End"));
}

void CViewProjects::OnProjectNoNewWork( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectNoNewWork - Function Begin"));

    CMainDocument*  pDoc   = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;

        PROJECT* project = pDoc->project(m_iSortedIndexes[row]);
        if (project) {
            if (!project->dont_request_more_work) {
                pFrame->UpdateStatusText(_("Telling project to not fetch any additional tasks..."));
                pDoc->ProjectNoMoreWork(m_iSortedIndexes[row]);
            }
        }
    }
    pFrame->UpdateStatusText(wxT(""));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectNoNewWork - Function End"));
}

void CViewProjects::OnProjectAllowNewWork( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectAllowNewWork - Function Begin"));

    CMainDocument*  pDoc   = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;

        PROJECT* project = pDoc->project(m_iSortedIndexes[row]);
        if (project) {
            if (project->dont_request_more_work) {
                pFrame->UpdateStatusText(_("Telling project to allow additional task downloads..."));
                pDoc->ProjectAllowMoreWork(m_iSortedIndexes[row]);
            }
        }
    }
    pFrame->UpdateStatusText(wxT(""));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectAllowNewWork - Function End"));
}

void CViewProjects::OnProjectReset( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectReset - Function Begin"));

    wxInt32         iAnswer        = 0; 
    wxString        strMessage     = wxEmptyString;
    CMainDocument*  pDoc           = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame         = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    CProject*       pProject       = NULL;
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Resetting project..."));

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        pProject = m_ProjectCache.at(m_iSortedIndexes[row]);

        strMessage.Printf(
            _("Are you sure you want to reset project '%s'?"), 
            pProject->m_strProjectName.c_str()
        );

        iAnswer = ::wxMessageBox(
            strMessage,
            _("Reset Project"),
            wxYES_NO | wxICON_QUESTION,
            this
        );

        if (wxYES == iAnswer) {
            pDoc->ProjectReset(m_iSortedIndexes[row]);
        }
    }
    
    pFrame->UpdateStatusText(wxT(""));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectReset - Function End"));
}

void CViewProjects::OnProjectDetach( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectDetach - Function Begin"));

    wxString        strMessage     = wxEmptyString;
    CMainDocument*  pDoc           = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame         = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Detaching from project..."));

    std::vector<size_t> selectedProjects;
    int row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) {
            break;
        }
        
        selectedProjects.push_back(m_iSortedIndexes[row]);
    }

    for (size_t i = 0; i< selectedProjects.size(); ++i) {
        const size_t projectIndex = selectedProjects[i];

        strMessage.Printf(_("Are you sure you want to detach from project '%s'?"), 
                pDoc->project(projectIndex)->project_name.c_str());

        if (::wxMessageBox(strMessage, _("Detach from Project"),
                wxYES_NO | wxICON_QUESTION, this) == wxYES) {
            pDoc->ProjectDetach(projectIndex);
        }
    }

    pFrame->UpdateStatusText(wxT(""));

    m_bForceUpdateSelection = true;
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectDetach - Function End"));
}

void CViewProjects::OnProjectWebsiteClicked( wxEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectWebsiteClicked - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    pFrame->UpdateStatusText(_("Launching browser..."));

    int website_task_index = event.GetId() - ID_TASK_PROJECT_WEB_PROJDEF_MIN;
    HyperLink::ExecuteLink(m_TaskGroups[1]->m_Tasks[website_task_index]->m_strWebSiteLink);
    
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectWebsiteClicked - Function End"));
}

wxInt32 CViewProjects::GetDocCount() {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    if (pDoc) {
        return static_cast<wxInt32>(pDoc->GetProjectCount());
    } else {
        return -1;
    }
}

wxString CViewProjects::OnListGetItemText(long item, long column) const {
    wxString  strBuffer = wxEmptyString;

    CProject* project   = m_ProjectCache.at(m_iSortedIndexes[item]);

    switch (column) {
        case COLUMN_PROJECT:
            strBuffer = project->m_strProjectName;
            break;
        case COLUMN_ACCOUNTNAME:
            strBuffer = project->m_strAccountName;
            break;
        case COLUMN_TEAMNAME:
            strBuffer = project->m_strTeamName;
            break;
        case COLUMN_TOTALCREDIT:
            strBuffer = project->m_strTotalCredit;
            break;
        case COLUMN_AVGCREDIT:
            strBuffer = project->m_strAVGCredit;
            break;
        case COLUMN_RESOURCESHARE:
            strBuffer = project->m_strResourceShare;
            break;
        case COLUMN_STATUS:
            strBuffer = project->m_strStatus;
        break;
    }
    return strBuffer;
}

wxInt32 CViewProjects::AddCacheElement() {
    CProject* pItem = new CProject();
    wxASSERT(pItem);
    if (pItem) {
        m_ProjectCache.push_back(pItem);
        m_iSortedIndexes.push_back((int)m_ProjectCache.size()-1);
        return 0;
    }
    return -1;
}

wxInt32 CViewProjects::EmptyCache() {
    unsigned int i;
    for (i=0; i<m_ProjectCache.size(); i++) {
        delete m_ProjectCache[i];
    }
    m_ProjectCache.clear();
    m_iSortedIndexes.clear();
    return 0;
}

wxInt32 CViewProjects::GetCacheCount() {
    return (wxInt32)m_ProjectCache.size();
}

wxInt32 CViewProjects::RemoveCacheElement() {
    unsigned int i;
    delete m_ProjectCache.back();
    m_ProjectCache.erase(m_ProjectCache.end() - 1);
    m_iSortedIndexes.clear();
    for (i=0; i<m_ProjectCache.size(); i++) {
        m_iSortedIndexes.push_back(i);
    }
    return 0;
}

void CViewProjects::UpdateSelection() {
    CTaskItemGroup*     pGroup = NULL;
    PROJECT*            project = NULL;
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    int                 n, row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);


    CTaskViewBase::PreUpdateSelection();

    pGroup = m_TaskGroups[0];

    row = -1;
    n = m_pListPane->GetSelectedItemCount();
    if (n == 1) {
        // Single selection
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        project = pDoc->project(m_iSortedIndexes[row]);

        // Start with everything enabled.
        m_pTaskPane->EnableTaskGroupTasks(pGroup);

        // Disable Suspend / Resume button
        if (project->suspended_via_gui) {
            m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_SUSPEND]);
        } else {
            m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_RESUME]);
        }

        // Allow / no new work
        if (project->dont_request_more_work) {
            m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_NOWORK]);
        } else {
            m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_ALLOWWORK]);
        }

        if (project->attached_via_acct_mgr) {
            m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_DETACH]);
        }

    } else if (n > 1) {
        // Multiple selection
        // Allow everything.
        m_pTaskPane->EnableTaskGroupTasks(pGroup);

    } else {
        // No selection
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
    }

    UpdateWebsiteSelection(GRP_WEBSITES, project);

    CTaskViewBase::PostUpdateSelection();
}

bool CViewProjects::SynchronizeCacheItem(wxInt32 iRowIndex, wxInt32 iColumnIndex) {
    wxString    strDocumentText  = wxEmptyString;
    float       fDocumentFloat = 0.0;
    float       fDocumentPercent = 0.0;
    CProject*   project = m_ProjectCache.at(m_iSortedIndexes[iRowIndex]);
    bool        dirty = false;

    strDocumentText.Empty();

    switch (iColumnIndex) {
        case COLUMN_PROJECT:
            GetDocProjectName(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(project->m_strProjectName)) {
                project->m_strProjectName = strDocumentText;
                return true;
            }
            break;
        case COLUMN_ACCOUNTNAME:
            GetDocAccountName(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(project->m_strAccountName)) {
                project->m_strAccountName = strDocumentText;
                return true;
            }
           break;
        case COLUMN_TEAMNAME:
            GetDocTeamName(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(project->m_strTeamName)) {
                project->m_strTeamName = strDocumentText;
                return true;
            }
            break;
        case COLUMN_TOTALCREDIT:
            GetDocTotalCredit(m_iSortedIndexes[iRowIndex], fDocumentFloat);
            if (fDocumentFloat != project->m_fTotalCredit) {
                project->m_fTotalCredit = fDocumentFloat;
                FormatTotalCredit(fDocumentFloat, project->m_strTotalCredit);
                return true;
            }
            break;
        case COLUMN_AVGCREDIT:
            GetDocAVGCredit(m_iSortedIndexes[iRowIndex], fDocumentFloat);
            if (fDocumentFloat != project->m_fAVGCredit) {
                project->m_fAVGCredit = fDocumentFloat;
                FormatAVGCredit(fDocumentFloat, project->m_strAVGCredit);
                return true;
            }
            break;
        case COLUMN_RESOURCESHARE:
            GetDocResourceShare(m_iSortedIndexes[iRowIndex], fDocumentFloat);
            if (fDocumentFloat != project->m_fResourceShare) {
                project->m_fResourceShare = fDocumentFloat;
                dirty = true;
            }
            GetDocResourcePercent(m_iSortedIndexes[iRowIndex], fDocumentPercent);
            if (fDocumentPercent != project->m_fResourcePercent) {
                project->m_fResourcePercent = fDocumentPercent;
                dirty = true;
            }
            if (dirty) {
                FormatResourceShare(fDocumentFloat, fDocumentPercent, project->m_strResourceShare);
                return true;
            }
            break;
        case COLUMN_STATUS:
            GetDocStatus(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(project->m_strStatus)) {
                project->m_strStatus = strDocumentText;
                return true;
            }
            break;
    }

    return false;
}

void CViewProjects::GetDocProjectName(size_t item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    PROJECT* project = 0;
    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        std::string project_name;
        project->get_name(project_name);
        strBuffer = HtmlEntityDecode(wxString(project_name.c_str(), wxConvUTF8));
    } else {
        strBuffer = wxEmptyString;
    }
}

void CViewProjects::GetDocAccountName(size_t item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    PROJECT* project = 0;
    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        strBuffer = HtmlEntityDecode(wxString(project->user_name.c_str(), wxConvUTF8));
    } else {
        strBuffer = wxEmptyString;
    }
}

void CViewProjects::GetDocTeamName(size_t item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    PROJECT* project = 0;
    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        strBuffer = HtmlEntityDecode(wxString(project->team_name.c_str(), wxConvUTF8));
    } else {
        strBuffer = wxEmptyString;
    }
}

void CViewProjects::GetDocTotalCredit(size_t item, float& fBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    PROJECT* project = 0;
    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        fBuffer = project->user_total_credit;
    } else {
        fBuffer = 0.0;
    }
}

wxInt32 CViewProjects::FormatTotalCredit(float fBuffer, wxString& strBuffer) const {
    strBuffer.Printf(wxT("%0.2f"), fBuffer);
    return 0;
}

void CViewProjects::GetDocAVGCredit(size_t item, float& fBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    PROJECT* project = 0;
    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        fBuffer = project->user_expavg_credit;
    } else {
        fBuffer = 0.0;
    }
}

wxInt32 CViewProjects::FormatAVGCredit(float fBuffer, wxString& strBuffer) const {
    strBuffer.Printf(wxT("%0.2f"), fBuffer);
    return 0;
}

void CViewProjects::GetDocResourceShare(size_t item, float& fBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    PROJECT* project = 0;
    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        fBuffer = project->resource_share;
    } else {
        fBuffer = 0.0;
    }
}

void CViewProjects::GetDocResourcePercent(size_t item, float& fBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    PROJECT* project = 0;
    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project && pDoc) {
        fBuffer = (project->resource_share / pDoc->m_fProjectTotalResourceShare) * 100;
    } else {
        fBuffer = 0.0;
    }
}

wxInt32 CViewProjects::FormatResourceShare(float fBuffer, float fBufferPercent, wxString& strBuffer) const {
    strBuffer.Printf(wxT("%0.0f (%0.2f%%)"), fBuffer, fBufferPercent);
    return 0;
}

void CViewProjects::GetDocStatus(size_t item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    PROJECT* project = 0;
    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        if (project->suspended_via_gui) {
            AppendToStatus(strBuffer, _("Suspended by user"));
        }
        if (project->dont_request_more_work) {
            AppendToStatus(strBuffer, _("Won't get new tasks"));
        }
        if (project->ended) {
            AppendToStatus(strBuffer, _("Project ended - OK to detach"));
        }
        if (project->detach_when_done) {
            AppendToStatus(strBuffer, _("Will detach when tasks done"));
        }
        if (project->sched_rpc_pending) {
            AppendToStatus(strBuffer, _("Scheduler request pending"));
            AppendToStatus(strBuffer, wxString(rpc_reason_string(project->sched_rpc_pending), wxConvUTF8));
        }
        if (project->scheduler_rpc_in_progress) {
            AppendToStatus(strBuffer, _("Scheduler request in progress"));
        }
        wxDateTime dtNextRPC((time_t)project->min_rpc_time);
        wxDateTime dtNow(wxDateTime::Now());
        if (dtNextRPC > dtNow) {
            wxTimeSpan tsNextRPC(dtNextRPC - dtNow);
            AppendToStatus(strBuffer, _("Communication deferred ") + tsNextRPC.Format());
        }
    }
}

double CViewProjects::GetProgressValue(long item) {
    CProject* project = m_ProjectCache.at(m_iSortedIndexes[item]);

    if (project) {
        return project->m_fResourcePercent / 100.0;
    }

    return 0.0;
}

bool CViewProjects::IsWebsiteLink(const wxString& strLink) {
    bool bReturnValue = false;

    if (strLink.StartsWith(wxT("web:")))
        bReturnValue = true;

    return bReturnValue;
}

wxInt32 CViewProjects::ConvertWebsiteIndexToLink(wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strLink) {
    strLink.Printf(wxT("web:%d:%d"), iProjectIndex, iWebsiteIndex);
    return 0;
}

wxInt32 CViewProjects::ConvertLinkToWebsiteIndex(const wxString& strLink, wxInt32& iProjectIndex, wxInt32& iWebsiteIndex) {
    wxString strTemplate = strLink;
    wxString strBuffer = wxEmptyString;

    strTemplate.Replace(wxT("web:"), wxEmptyString);

    strBuffer = strTemplate;
    strBuffer.Remove(strBuffer.Find(wxT(":")));
    strBuffer.ToLong((long*) &iProjectIndex);

    strBuffer = strTemplate;
    strBuffer = strBuffer.Mid(strBuffer.Find(wxT(":")) + 1);
    strBuffer.ToLong((long*) &iWebsiteIndex);

    return 0;
}
