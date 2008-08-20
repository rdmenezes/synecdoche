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

#include "stdwx.h"
#include "PrefGridBase.h"
#include "PrefTreeBook.h"



IMPLEMENT_DYNAMIC_CLASS(PrefGridBase, PrefNodeBase)


PrefGridBase::PrefGridBase(wxWindow* parent, GLOBAL_PREFS* preferences)
: PrefNodeBase(parent, preferences) {

    m_selected = 0;

    // PrefGridBase is derived from wxScrolledWindow. Allow vertical scroll only.
    SetScrollRate(0, 10);

    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    SetSizer(s);

    m_gridPanel = new wxPanel(this);
    s->Add(m_gridPanel, 1, wxEXPAND);

    m_groupSizer = new wxGridBagSizer(1, 1);

    // The label column is growable.
    m_groupSizer->AddGrowableCol(0, 1);

    m_gridPanel->SetSizer(m_groupSizer);
}


PrefGridBase::~PrefGridBase() {}

/// Creates a new group, and adds it to the grid. The group is owned by the
/// grid object.
/// \param[in] title The heading for the group.
/// \return A pointer to the newly created group.
PrefGridBase::PrefGroup* PrefGridBase::AddGroup(const wxString& title) {

    PrefGroup* group = new PrefGroup(this, title);

    wxStaticText* groupLabel = new wxStaticText(m_gridPanel, wxID_ANY, title);
    wxFont font = groupLabel->GetFont();
    font.SetWeight(wxBOLD);
    groupLabel->SetFont(font);

    m_groupSizer->Add(groupLabel, wxGBPosition(GetTotalSize(), 0), wxGBSpan(1, 2), wxALL, 3);
    m_groupList.push_back(group);

    return group;
}


void PrefGridBase::AddPreference(PrefValueBase* pref) {

    pref->CreateControls();

    m_prefList.push_back(pref);
}


int PrefGridBase::GetTotalSize() const {
    return (int) (m_prefList.size() + m_groupList.size());
}


// Nested classes

// PrefGroup implementation

PrefGridBase::PrefGroup::PrefGroup(wxWindow* win, const wxString& title) {

    wxASSERT(win->IsKindOf(CLASSINFO(PrefGridBase)));
    m_page = (PrefGridBase*) win;
}

PrefGridBase::PrefGroup::~PrefGroup() {}


void PrefGridBase::PrefGroup::AddPreference(PrefValueBase* pref) {

    m_page->AddPreference(pref);
}


// PrefValueBase implementation

IMPLEMENT_DYNAMIC_CLASS(PrefGridBase::PrefValueBase, wxEvtHandler)

PrefGridBase::PrefValueBase::PrefValueBase(
            PrefGridBase* parent) : wxEvtHandler(), m_grid(parent)
{

}


/// The child windows are not created until the preference is added to a
/// property grid with PrefGroup::AddPreference.
PrefGridBase::PrefValueBase::PrefValueBase(
    PrefGridBase* parent,
    const wxString& label,
    const wxString& helpText,
    const wxString& helpDefault,
    const wxValidator& val
    ) : wxEvtHandler(), m_grid(parent), m_label(label), m_helpText(helpText), m_default(helpDefault)
{
    m_validator = (wxValidator*) val.Clone();
    m_enabled = true;

    m_labelPanel = 0;
    m_controlPanel = 0;

    m_valueCtrl = 0;
    m_valueStaticCtrl = 0;
}


/// The property label is created in the left column, and a panel for the
/// input control in the right column. Derived classes add a suitable control
/// to this panel.
/// \return A pointer to the input control panel.
wxPanel* PrefGridBase::PrefValueBase::CreateControls() {

    m_labelPanel = new wxPanel(m_grid->m_gridPanel);
    m_controlPanel = new wxPanel(m_grid->m_gridPanel);

    m_labelCtrl = new wxStaticText(m_labelPanel, wxID_ANY, m_label);
    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    s->Add(m_labelCtrl, 0, wxALL, 3);
    m_labelPanel->SetSizer(s);

    m_labelPanel->SetHelpText(m_helpText);

    m_valueStaticCtrl = new wxStaticText(m_controlPanel, wxID_ANY, wxEmptyString);
    m_valueStaticCtrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    s = new wxBoxSizer(wxVERTICAL);
    s->Add(m_valueStaticCtrl, 0, wxALL | wxEXPAND, 3);
    m_controlPanel->SetSizer(s);

    m_valueStaticCtrl->Show(!m_enabled);

    m_grid->m_groupSizer->Add(m_labelPanel, wxGBPosition(m_grid->GetTotalSize(), 0), wxDefaultSpan, wxEXPAND);
    m_grid->m_groupSizer->Add(m_controlPanel, wxGBPosition(m_grid->GetTotalSize(), 1), wxDefaultSpan, wxEXPAND);

    m_labelPanel->Connect(wxEVT_LEFT_DOWN,
        wxMouseEventHandler(PrefGridBase::PrefValueBase::OnClick), 0, this);
    m_labelCtrl->Connect(wxEVT_LEFT_DOWN,
        wxMouseEventHandler(PrefGridBase::PrefValueBase::OnClick), 0, this);
    m_controlPanel->Connect(wxEVT_LEFT_DOWN,
        wxMouseEventHandler(PrefGridBase::PrefValueBase::OnClick), 0, this);
    m_valueStaticCtrl->Connect(wxEVT_LEFT_DOWN,
        wxMouseEventHandler(PrefGridBase::PrefValueBase::OnClick), 0, this);

    UpdateColours();

    return m_controlPanel;
}


/// Highlights the property row, and sets the focus to the control if the
/// property is enabled. Disabled properties can be selected, but don't get
/// focus. Selecting a row will automatically deselect the previous selection.
///
/// Selecting a property fires a PrefHelpEvent.
void PrefGridBase::PrefValueBase::Select() {
    if (this != m_grid->m_selected) {

        if (m_grid->m_selected) {
            m_grid->m_selected->Deselect();
        }

        m_labelPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
        m_labelCtrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
        m_labelPanel->Refresh();

        m_grid->m_selected = this;

        PrefHelpEvent e(PREF_EVT_HELP_CMD, m_grid->GetId());
        e.SetTitle(m_label);
        e.SetDefault(m_default);
        e.SetEventObject(m_labelPanel);
        m_grid->GetEventHandler()->ProcessEvent(e);
    }
}


/// Clears the selected state.
void PrefGridBase::PrefValueBase::Deselect() {

    UpdateColours();
    m_labelPanel->Refresh();

    m_grid->m_selected = 0;
}


void PrefGridBase::PrefValueBase::OnClick(wxMouseEvent& event) {

    Select();
    m_controlPanel->SetFocus();
    event.Skip();
}


void PrefGridBase::PrefValueBase::OnFocus(wxFocusEvent& event) {

    Select();
    event.Skip();
}


bool PrefGridBase::PrefValueBase::Enable(bool enable) {

    if (m_enabled != enable) {
        m_enabled = enable;
        if (m_valueCtrl) {
            m_valueCtrl->Show(enable);
            m_valueStaticCtrl->Show(!enable);
        }
        if (m_controlPanel) {
            m_controlPanel->Layout();
        }
        UpdateColours();
        return true;
    }
    return false;
}


void PrefGridBase::PrefValueBase::UpdateColours() {

    if (m_labelPanel) {
        m_labelPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
        if (m_enabled) {
            m_labelCtrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
        } else {
            m_labelCtrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
        }
        m_labelPanel->Refresh();
    }

    if (m_controlPanel) {
        m_controlPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
        if (m_enabled) {
            m_controlPanel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
        } else {
            m_controlPanel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
        }
        m_controlPanel->Refresh();
    }
}


// PrefValueText implementation

IMPLEMENT_DYNAMIC_CLASS(PrefGridBase::PrefValueText, PrefGridBase::PrefValueBase)

PrefGridBase::PrefValueText::PrefValueText(
            PrefGridBase* parent) : PrefGridBase::PrefValueBase(parent)
{

}


PrefGridBase::PrefValueText::PrefValueText(
            PrefGridBase* parent,
            const wxString& label,
            const wxString& helpText,
            const wxString& helpDefault,
            const wxTextValidator& val
            ) : PrefGridBase::PrefValueBase(
            parent, label, helpText, helpDefault, val)
{

}

wxPanel* PrefGridBase::PrefValueText::CreateControls() {
    PrefValueBase::CreateControls();

    m_text = new wxTextCtrl(m_controlPanel, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, *m_validator);

    m_text->SetMinSize(wxSize(-1, m_labelPanel->GetSize().GetY() - 6));

    m_controlPanel->GetSizer()->Add(m_text, 0, wxALL | wxEXPAND, 3);
    m_text->Show(m_enabled);

    m_text->Connect(wxEVT_SET_FOCUS,
        wxFocusEventHandler(PrefGridBase::PrefValueBase::OnFocus), 0, this);

    m_text->Connect(wxEVT_COMMAND_TEXT_UPDATED,
        wxCommandEventHandler(PrefGridBase::PrefValueText::OnChange), 0, this);

    m_valueCtrl = m_text;

    return m_controlPanel;
}

void PrefGridBase::PrefValueText::OnChange(wxCommandEvent& WXUNUSED(event)) {

    m_valueStaticCtrl->SetLabel(m_text->GetValue());
}

void PrefGridBase::PrefValueText::SetValue(const wxString& value) {
    if (m_text) {
        m_text->SetValue(value);
    }
}

// PrefValueBool implementation

IMPLEMENT_DYNAMIC_CLASS(PrefGridBase::PrefValueBool, PrefGridBase::PrefValueBase)

PrefGridBase::PrefValueBool::PrefValueBool(
            PrefGridBase* parent) : PrefGridBase::PrefValueBase(parent)
{

}

PrefGridBase::PrefValueBool::PrefValueBool(
            PrefGridBase* parent,
            const wxString& label,
            const wxString& helpText,
            const wxString& helpDefault,
            const ValidateYesNo& val
            ) : PrefGridBase::PrefValueBase(
            parent, label, helpText, helpDefault, val)
{

}

wxPanel* PrefGridBase::PrefValueBool::CreateControls() {
    PrefValueBase::CreateControls();

    wxArrayString choices;
    choices.Add( _("Yes") );
    choices.Add( _("No") );

    m_combo = new wxOwnerDrawnComboBox (m_controlPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        choices, wxODCB_DCLICK_CYCLES | wxCB_READONLY | wxBORDER_NONE);

    // There is a bug in wxComboCtrl::SetValidator().
    m_combo->wxControl::SetValidator(*m_validator);

    m_combo->SetMinSize(wxSize(m_labelPanel->GetSize().GetY() * 3, m_labelPanel->GetSize().GetY()));

    m_controlPanel->GetSizer()->Add(m_combo, 0, wxALL | wxEXPAND, 0);
    m_combo->Show(m_enabled);
    m_combo->PushEventHandler(this);

    m_combo->Connect(wxEVT_LEFT_DOWN,
        wxMouseEventHandler(PrefGridBase::PrefValueBase::OnClick), 0, this);

    m_combo->Connect(wxEVT_SET_FOCUS,
        wxFocusEventHandler(PrefGridBase::PrefValueBase::OnFocus), 0, this);

    m_combo->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED,
        wxCommandEventHandler(PrefGridBase::PrefValueBool::OnChange), 0, this);

    m_valueCtrl = m_combo;

    return m_controlPanel;
}

void PrefGridBase::PrefValueBool::OnChange(wxCommandEvent& WXUNUSED(event)) {

    m_valueStaticCtrl->SetLabel(m_combo->GetValue());
}
