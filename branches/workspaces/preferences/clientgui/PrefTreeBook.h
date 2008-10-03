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

#ifndef PREFTREEBOOK_H
#define PREFTREEBOOK_H

#include "PrefNodeBase.h"

/// Data class to attach NodeType to TreeItem.
class PrefNodeItemData : public wxTreeItemData {
public:
    PrefNodeItemData(PrefNodeType nodeType) : wxTreeItemData(), m_nodeType(nodeType) {};
    inline PrefNodeType GetNodeType() { return m_nodeType; };

private:
    PrefNodeType m_nodeType;
};


// Event class for extended help events.
class PrefHelpEvent: public wxCommandEvent {
public:
    PrefHelpEvent(wxEventType commandType = wxEVT_NULL, int id = 0) : wxCommandEvent(commandType, id) {}
    PrefHelpEvent(const PrefHelpEvent& event)
        : wxCommandEvent(event), m_title(event.m_title), m_default(event.m_default) {}

    /// Gets the title for the help. This is typically the name of the preference.
    /// \return The help title.
    wxString GetTitle() { return m_title; }

    /// Sets the title for the help. This is typically the name of the preference.
    /// \param[in] title The help title.
    void SetTitle(const wxString& title) { m_title = title; }

    /// Gets the default value for the preference, as a string.
    /// \return The default for the preference.

    wxString GetDefault() { return m_default; }
    /// Sets the default value displayed for the preference, as a string.
    /// \param[in] helpDefault The default for the preference.
    void SetDefault(const wxString& helpDefault) { m_default = helpDefault; }

    // Clone required for sending with wxPostEvent()
    wxEvent* Clone() const { return new PrefHelpEvent(*this); }

private:
    wxString m_title;       ///< Field for Title property.
    wxString m_default;     ///< Field for Default property.
};

DECLARE_EVENT_TYPE(PREF_EVT_HELP_CMD, -1)

typedef void (wxEvtHandler::*PrefHelpEventFunction)(PrefHelpEvent&);

#define PREF_EVT_HELP(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY(PREF_EVT_HELP_CMD, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
    wxStaticCastEvent(PrefHelpEventFunction, &fn), (wxObject *) NULL),




// Not using wxTreeBook for extra flexibility.

class PrefTreeBook : public wxPanel {

    DECLARE_DYNAMIC_CLASS(PrefTreeBook)
    DECLARE_EVENT_TABLE()

public:
    PrefTreeBook(wxWindow* parent = NULL);
    virtual ~PrefTreeBook();

    void SavePreferences();

protected:
    bool SaveState();
    bool RestoreState();
    void OnTreeSelectionChanging(wxTreeEvent& event);
    void OnHelp(PrefHelpEvent& event);

private:
    bool Find(const wxTreeItemId& root, wxTreeItemId& result, PrefNodeType nodeType);

    wxTreeCtrl*     m_tree;

    wxStaticText*   m_helpTitleCtrl;
    wxStaticText*   m_helpTextCtrl;
    wxStaticText*   m_helpDefaultCtrl;

    wxWindow*       m_helpSource;

    wxWindow*       m_content;
    GLOBAL_PREFS    m_preferences;

};

#endif // PREFTREEBOOK_H
