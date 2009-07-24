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

#ifndef BOINCTASKCTRL_H
#define BOINCTASKCTRL_H

#include <wx/scrolwin.h>

class CTaskItem;
class CTaskItemGroup;
class CTaskViewBase;

class wxConfigBase;
class wxBoxSizer;

class CBOINCTaskCtrl : public wxScrolledWindow {
    DECLARE_DYNAMIC_CLASS( CBOINCTaskCtrl )

public:
    CBOINCTaskCtrl();
    CBOINCTaskCtrl( CTaskViewBase* pView, wxWindowID iTaskWindowID, wxInt32 iTaskWindowFlags );

    ~CBOINCTaskCtrl();

    wxInt32 DeleteTaskGroupAndTasks( CTaskItemGroup* pGroup );
    wxInt32 DisableTaskGroupTasks( CTaskItemGroup* pGroup );
    wxInt32 EnableTaskGroupTasks( CTaskItemGroup* pGroup );

    wxInt32 DeleteTask( CTaskItemGroup* pGroup, CTaskItem* pItem );
    wxInt32 DisableTask( CTaskItem* pItem );
    wxInt32 EnableTask( CTaskItem* pItem );
    wxInt32 UpdateTask( CTaskItem* pItem, wxString strName, wxString strDescription );

    wxInt32 UpdateControls();

    virtual bool OnSaveState( wxConfigBase* pConfig );
    virtual bool OnRestoreState( wxConfigBase* pConfig );

private:

    CTaskViewBase*  m_pParent;

    wxBoxSizer*      m_pSizer;
};


#endif
