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

#ifndef _VIEWRESOURCES_H_
#define _VIEWRESOURCES_H_

#include <wx/dynarray.h>

#include "BOINCBaseView.h"
#include "common/wxPieCtrl.h"

class PROJECT;

WX_DECLARE_OBJARRAY(wxColour, wxArrayColour);

class CViewResources : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewResources )
    DECLARE_EVENT_TABLE()

public:
    CViewResources();
    CViewResources(wxNotebook* pNotebook);

    ~CViewResources();

    virtual const wxString& GetViewName();
    virtual const wxString& GetViewDisplayName();
    virtual const char**    GetViewIcon();
#ifdef __WXMAC__
    virtual int             GetViewRefreshRate();
#endif

protected:

    wxPieCtrl*              m_pieCtrlBOINC;
    wxPieCtrl*              m_pieCtrlTotal;

    bool                    m_BOINCwasEmpty;

    virtual void            UpdateSelection();

    wxInt32                 FormatProjectName(const PROJECT*, wxString& strBuffer ) const;
    wxInt32                 FormatDiskSpace(double bytes, wxString& strBuffer) const;

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );
    virtual void            OnListRender( wxTimerEvent& event );

    virtual void            DemandLoadView();
};


#endif
