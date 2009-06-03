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

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "sg_DlgMessages.h"
#include "sg_SGUIListControl.h"


IMPLEMENT_DYNAMIC_CLASS(CSGUIListCtrl, wxListView)


CSGUIListCtrl::CSGUIListCtrl() {}

CSGUIListCtrl::CSGUIListCtrl(CPanelMessages* pView, wxWindowID iListWindowID, wxInt32 iListWindowFlags)
    :wxListView(pView, iListWindowID, wxDefaultPosition, wxSize(640,480), iListWindowFlags) 
{
    m_pParentView = pView;

    m_bIsSingleSelection = (iListWindowFlags & wxLC_SINGLE_SEL) ? true : false ;
}


wxString CSGUIListCtrl::OnGetItemText(long item, long column) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CPanelMessages));

    return m_pParentView->OnListGetItemText(item, column);
}


int CSGUIListCtrl::OnGetItemImage(long /* item */) const {
    return 1;
}


wxListItemAttr* CSGUIListCtrl::OnGetItemAttr(long item) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CPanelMessages));

    return m_pParentView->OnListGetItemAttr(item);
}
