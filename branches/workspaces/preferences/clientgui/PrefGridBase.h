// Synecdoche
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _PREFGRIDBASE_H_
#define _PREFGRIDBASE_H_

#include "prefs.h"
#include "PrefNodeBase.h"
#include "ValidateYesNo.h"
#include "ValidateNumber.h"
#include "ValidateTime.h"

class wxOwnerDrawnComboBox;

class PrefGridBase : public PrefNodeBase {

    DECLARE_DYNAMIC_CLASS(PrefGridBase)

public:
    PrefGridBase(wxWindow* parent = NULL, GLOBAL_PREFS* preferences = NULL);
    virtual ~PrefGridBase();

protected:
    class PrefGroup;
    class PrefValueBase;

    PrefGroup*      AddGroup(const wxString& title);
    void            AddPreference(PrefValueBase* pref);

private:
    int GetTotalSize() const;

    wxGridBagSizer*             m_groupSizer;
    std::vector<PrefGroup*>     m_groupList;
    std::vector<PrefValueBase*> m_prefList;

    PrefValueBase* m_selected;

// Nested utility classes:
protected:

    // Group box containing pref widgets.
    class PrefGroup : public wxObject {
    public:
        PrefGroup(wxWindow* win, const wxString& title);
        ~PrefGroup();

        void AddPreference(PrefValueBase* pref);

    private:
        PrefGridBase* m_page;

    };

    // Base for individual preference widgets. Abstract.
    class PrefValueBase : public wxEvtHandler {

        DECLARE_DYNAMIC_CLASS(PrefValueBase)
        //DECLARE_EVENT_TABLE()

    public:
        PrefValueBase(PrefGridBase* parent = NULL);

        PrefValueBase(
            PrefGridBase* parent,
            const wxString& label,
            const wxString& helpText,
            const wxString& helpDefault,
            const wxValidator& val
        );

        virtual wxPanel* CreateControls();
        void OnClick(wxMouseEvent& event);
        void OnFocus(wxFocusEvent& event);
    protected:

        void Select();
        void Deselect();



        PrefGridBase*   m_grid;
        wxString        m_label;
        wxString        m_helpText;
        wxString        m_default;
        wxValidator*    m_validator;

        wxPanel*        m_labelPanel;
        wxStaticText*   m_labelCtrl;
        wxPanel*        m_controlPanel;

    };


    class PrefValueText : public PrefValueBase {

        DECLARE_DYNAMIC_CLASS(PrefValueText)

    public:
        PrefValueText(PrefGridBase* parent = NULL);

        PrefValueText(
            PrefGridBase* parent,
            const wxString& label,
            const wxString& helpText,
            const wxString& default,
            const wxTextValidator& val
        );

        virtual wxPanel* CreateControls();

    protected:
        wxTextCtrl*       m_text;
    };


    class PrefValueBool : public PrefValueBase {

        DECLARE_DYNAMIC_CLASS(PrefValueBool)

    public:
        PrefValueBool(PrefGridBase* parent = NULL);

        PrefValueBool(
            PrefGridBase* parent,
            const wxString& label,
            const wxString& helpText,
            const wxString& default,
            const ValidateYesNo& val
        );

        virtual wxPanel* CreateControls();

    protected:
        wxOwnerDrawnComboBox* m_combo;

    };


private:

    // End of nested classes
};


#endif // _PREFGRIDBASE_H_
