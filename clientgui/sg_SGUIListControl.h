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

#ifndef SGUILISTCTRL_H
#define SGUILISTCTRL_H

#include <wx/colour.h>
#include <wx/string.h>
#include <wx/listctrl.h>

#define DEFAULT_LIST_MULTI_SEL_FLAGS   wxLC_REPORT | wxLC_VIRTUAL

class CPanelMessages;
class wxListItemAttr;

class CSGUIListCtrl : public wxListView {
    DECLARE_DYNAMIC_CLASS(CSGUIListCtrl)

public:
    CSGUIListCtrl();
    CSGUIListCtrl(CPanelMessages* pView, wxWindowID iListWindowID, wxInt32 iListWindowFlags);

private:
    
    virtual wxString        OnGetItemText(long item, long column) const;
    virtual int             OnGetItemImage(long item) const;
    virtual wxListItemAttr* OnGetItemAttr(long item) const;
    virtual wxColour        GetBackgroundColour() const;

    bool                    m_bIsSingleSelection;

    CPanelMessages*         m_pParentView;
};

#endif // SGUILISTCTRL_H
