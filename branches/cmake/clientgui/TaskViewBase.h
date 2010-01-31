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
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef _TASKVIEWBASE_H_
#define _TASKVIEWBASE_H_

#include <vector>
#include <wx/string.h>

#include "BOINCBaseView.h"

#define DEFAULT_TASK_FLAGS             wxTAB_TRAVERSAL | wxADJUST_MINSIZE
#define DEFAULT_LIST_MULTI_SEL_FLAGS   wxLC_REPORT | wxLC_VIRTUAL

class wxButton;
class wxStaticBox;
class wxStaticBoxSizer;

class CBOINCTaskCtrl;
class CBOINCListCtrl;
class PROJECT;


class CTaskItem : wxObject {
public:
    CTaskItem();
    CTaskItem( wxString strName, wxString strDescription, wxInt32 iEventID ) :
        m_strName(strName), m_strDescription(strDescription), m_iEventID(iEventID),
        m_pButton(NULL), m_strWebSiteLink(wxT("")) {};
    CTaskItem( wxString strName, wxString strDescription, wxString strWebSiteLink, wxInt32 iEventID ) :
        m_strName(strName), m_strDescription(strDescription), m_iEventID(iEventID),  
        m_pButton(NULL), m_strWebSiteLink(strWebSiteLink) {};
    ~CTaskItem() {};

    wxString                m_strName;
    wxString                m_strDescription;
    wxInt32                 m_iEventID;

    wxButton*               m_pButton;
    wxString                m_strWebSiteLink;
};


class CTaskItemGroup : wxObject {
public:
    CTaskItemGroup();
    CTaskItemGroup( wxString strName ) :
        m_strName(strName), m_pStaticBox(NULL), m_pStaticBoxSizer(NULL) { m_Tasks.clear(); };
    ~CTaskItemGroup() {};
    wxButton* button(int i) {return m_Tasks[i]->m_pButton;}

    wxString                m_strName;

    wxStaticBox*            m_pStaticBox;
    wxStaticBoxSizer*       m_pStaticBoxSizer;

    std::vector<CTaskItem*> m_Tasks;
};

// Base for all views which include a task pane.
class CTaskViewBase : public CBOINCBaseView {
    DECLARE_DYNAMIC_CLASS( CTaskViewBase )

public:
    CTaskViewBase();
    CTaskViewBase(wxNotebook* pNotebook);

    virtual ~CTaskViewBase();

    bool                    FireOnSaveState( wxConfigBase* pConfig );
    bool                    FireOnRestoreState( wxConfigBase* pConfig );

    std::vector<CTaskItemGroup*> m_TaskGroups;

protected:
    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );

    virtual void            EmptyTasks();

    virtual void            PreUpdateSelection();
    virtual void            UpdateSelection();
    virtual void            PostUpdateSelection();

    virtual void            UpdateWebsiteSelection(long lControlGroup, const PROJECT* project);

    virtual void            DemandLoadView(
                                wxWindowID iTaskWindowID,
                                int iTaskWindowFlags,
                                wxWindowID iListWindowID,
                                int iListWindowFlags
                            );

    CBOINCTaskCtrl*         m_pTaskPane;

    bool                    m_bTaskPaneVisible;
};


#endif
