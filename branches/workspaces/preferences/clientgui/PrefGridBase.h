// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
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

#ifndef PREFGRIDBASE_H
#define PREFGRIDBASE_H

#include "prefs.h"
#include "PrefNodeBase.h"
#include "ValidateYesNo.h"
#include "ValidateNumber.h"
#include "ValidateTime.h"

class wxOwnerDrawnComboBox;

/// Base class for property grid preferences.
/// Preferences are arranged in a grid, with group headers spanning both
/// columns. The selected row is shown highlighted, and selection changes
/// fire a PrefHelpEvent.
class PrefGridBase : public PrefNodeBase {

    DECLARE_DYNAMIC_CLASS(PrefGridBase)

public:
    PrefGridBase(wxWindow* parent = NULL, GLOBAL_PREFS* preferences = NULL);
    virtual ~PrefGridBase();

protected:
    class PrefGroup;
    class PrefValueBase;

    /// Adds a group to the grid.
    PrefGroup*      AddGroup(const wxString& title);

    /// Adds a single preference to the grid.
    void            AddPreference(PrefValueBase* pref);

private:
    int GetTotalSize() const;

    wxPanel*                    m_gridPanel;    ///< Panel filling the virtual scrolled window size.
    wxGridBagSizer*             m_groupSizer;   ///< Sizer for m_gridPanel.
    std::vector<PrefGroup*>     m_groupList;    ///< All groups added to the grid.
    std::vector<PrefValueBase*> m_prefList;     ///< All preferences added to the grid.

    PrefValueBase*              m_selected;     ///< Currently selected preference item. May be null.

// Nested utility classes:
protected:

    /// A group of preferences.
    /// In a property grid page, groups are rendered as a bold heading, spanning
    /// both columns.
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

        virtual ~PrefValueBase();

        /// Creates the standard child controls for a property row.
        virtual wxPanel* CreateControls();
        void OnClick(wxMouseEvent& event);
        void OnFocus(wxFocusEvent& event);

        bool Enable(bool enable = true);
        bool Disable() { return Enable(false); }

        /// Selects the property row.
        void Select();

        /// Deselects the property row.
        void Deselect();

    protected:
        /// Changes the child control colours to match the current state.
        void UpdateColours();

        PrefGridBase*   m_grid;
        wxString        m_label;
        wxString        m_helpText;
        wxString        m_default;
        wxValidator*    m_validator;

        wxPanel*        m_labelPanel;
        wxStaticText*   m_labelCtrl;
        wxPanel*        m_controlPanel;

        wxWindow*       m_valueCtrl;
        wxWindow*       m_valueStaticCtrl;

        bool            m_enabled;

    };


    /// A class for arbitrary text preferences.
    /// The validator passed to the constructor handles translating
    /// the text value to whatever format is required by the
    /// underlying preference.
    class PrefValueText : public PrefValueBase {

        DECLARE_DYNAMIC_CLASS(PrefValueText)

    public:
        PrefValueText(PrefGridBase* parent = NULL);

        PrefValueText(
            PrefGridBase* parent,
            const wxString& label,
            const wxString& helpText,
            const wxString& helpDefault,
            const wxTextValidator& val
        );

        virtual wxPanel* CreateControls();
        void OnChange(wxCommandEvent& event);
        void SetValue(const wxString& value);

    protected:
        wxTextCtrl*       m_text;
    };


    /// A class for boolean preferences.
    /// The preference is presented as a yes/no choice using a
    /// dropdown box.
    class PrefValueBool : public PrefValueBase {

        DECLARE_DYNAMIC_CLASS(PrefValueBool)

    public:
        PrefValueBool(PrefGridBase* parent = NULL);

        PrefValueBool(
            PrefGridBase* parent,
            const wxString& label,
            const wxString& helpText,
            const wxString& helpDefault,
            const ValidateYesNo& val
        );

        virtual ~PrefValueBool();

        virtual wxPanel* CreateControls();
        void OnChange(wxCommandEvent& event);

    protected:
        wxOwnerDrawnComboBox* m_combo;

    };


private:

    // End of nested classes
};


#endif // PREFGRIDBASE_H
