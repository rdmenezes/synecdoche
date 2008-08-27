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
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCBaseView.h"
#include "TaskViewBase.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "Events.h"

#include "res/boinc.xpm"
#include "res/sortascending.xpm"
#include "res/sortdescending.xpm"

IMPLEMENT_DYNAMIC_CLASS(CTaskViewBase, CBOINCBaseView)


CTaskViewBase::CTaskViewBase() {}

CTaskViewBase::CTaskViewBase(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook)
{
    wxASSERT(pNotebook);

    m_bTaskPaneVisible = false;

    m_pTaskPane = NULL;
}

CTaskViewBase::~CTaskViewBase() {
    EmptyTasks();
}

bool CTaskViewBase::OnSaveState(wxConfigBase* pConfig) {
    bool bReturnValue = true;

    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);

    bReturnValue = CBOINCBaseView::OnSaveState(pConfig);

    if (!m_pTaskPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    return bReturnValue;
}


bool CTaskViewBase::OnRestoreState(wxConfigBase* pConfig) {
    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);

    if (!m_pTaskPane->OnRestoreState(pConfig)) {
        return false;
    }

    return CBOINCBaseView::OnRestoreState(pConfig);
}


void CTaskViewBase::EmptyTasks() {
    unsigned int i;
    unsigned int j;
    for (i=0; i<m_TaskGroups.size(); i++) {
        for (j=0; j<m_TaskGroups[i]->m_Tasks.size(); j++) {
            delete m_TaskGroups[i]->m_Tasks[j];
        }
        m_TaskGroups[i]->m_Tasks.clear();
        delete m_TaskGroups[i];
    }
    m_TaskGroups.clear();
}


void CTaskViewBase::PreUpdateSelection(){

    CBOINCBaseView::PreUpdateSelection();

    wxASSERT(m_pTaskPane);
    m_pTaskPane->Freeze();
}


void CTaskViewBase::UpdateSelection(){
}


void CTaskViewBase::PostUpdateSelection(){

    wxASSERT(m_pTaskPane);
    m_pTaskPane->UpdateControls();
    m_pTaskPane->Thaw();

    CBOINCBaseView::PostUpdateSelection();
}


void CTaskViewBase::UpdateWebsiteSelection(long lControlGroup, PROJECT* project){
    unsigned int        i;
    CTaskItemGroup*     pGroup = NULL;
    CTaskItem*          pItem = NULL;

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    m_pTaskPane->Freeze();
    // Update the websites list
    //
    if (m_bForceUpdateSelection) {
        if (m_TaskGroups.size() > 1) {

            // Delete task group, objects, and controls.
            pGroup = m_TaskGroups[lControlGroup];

            m_pTaskPane->DeleteTaskGroupAndTasks(pGroup);
            for (i=0; i<pGroup->m_Tasks.size(); i++) {
                delete pGroup->m_Tasks[i];
            }
            pGroup->m_Tasks.clear();
            delete pGroup;

            pGroup = NULL;

            m_TaskGroups.erase( m_TaskGroups.begin() + 1 );
        }

        // If something is selected create the tasks and controls
        if (m_pListPane->GetSelectedItemCount()) {
            if (project) {
                // Create the web sites task group
                pGroup = new CTaskItemGroup( _("Web sites") );
                m_TaskGroups.push_back( pGroup );

                // Default project url
                pItem = new CTaskItem(
                    wxString(project->project_name.c_str(), wxConvUTF8), 
                    wxT(""), 
                    wxString(project->master_url.c_str(), wxConvUTF8),
                    ID_TASK_PROJECT_WEB_PROJDEF_MIN
                );
                pGroup->m_Tasks.push_back(pItem);


                // Project defined urls
                for (i=0;(i<project->gui_urls.size())&&(i<=ID_TASK_PROJECT_WEB_PROJDEF_MAX);i++) {
                    pItem = new CTaskItem(
                        wxGetTranslation(wxString(project->gui_urls[i].name.c_str(), wxConvUTF8)),
                        wxGetTranslation(wxString(project->gui_urls[i].description.c_str(), wxConvUTF8)),
                        wxString(project->gui_urls[i].url.c_str(), wxConvUTF8),
                        ID_TASK_PROJECT_WEB_PROJDEF_MIN + 1 + i
                    );
                    pGroup->m_Tasks.push_back(pItem);
                }
            }
        }

        m_bForceUpdateSelection = false;
    }
    m_pTaskPane->Thaw();

}


void CTaskViewBase::DemandLoadView(wxWindowID iTaskWindowID, int iTaskWindowFlags,
    wxWindowID iListWindowID, int iListWindowFlags) {

    wxASSERT(!m_bViewLoaded);

    //CBOINCBaseView::DemandLoadView();

    wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(2, 0, 0);
    wxASSERT(itemFlexGridSizer);

    itemFlexGridSizer->AddGrowableRow(0);
    itemFlexGridSizer->AddGrowableCol(1);
    
    m_pTaskPane = new CBOINCTaskCtrl(this, iTaskWindowID, iTaskWindowFlags);
    wxASSERT(m_pTaskPane);

    m_pListPane = new CBOINCListCtrl(this, iListWindowID, iListWindowFlags);
    wxASSERT(m_pListPane);

    itemFlexGridSizer->Add(m_pTaskPane, 1, wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_pListPane, 1, wxGROW|wxALL, 1);

    SetSizer(itemFlexGridSizer);

    Layout();
}
