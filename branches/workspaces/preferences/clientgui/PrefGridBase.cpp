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

#include "stdwx.h"
#include "wx/treectrl.h"
#include "wx/valgen.h"
#include "wx/gbsizer.h"
#include "PrefGridBase.h"
#include "PrefTreeBook.h"
#include <wx/odcombo.h>


IMPLEMENT_DYNAMIC_CLASS(PrefGridBase, PrefNodeBase)


PrefGridBase::PrefGridBase(wxWindow* parent, GLOBAL_PREFS* preferences)
: PrefNodeBase(parent, preferences) {

    //SetAutoLayout(true);
    m_selected = 0;

    SetScrollRate(10, 10);

    m_groupSizer = new wxGridBagSizer(1, 1);

    m_groupSizer->AddGrowableCol(0, 1);
    //m_groupSizer->AddGrowableCol(1, 1);

    SetSizer(m_groupSizer);
}


PrefGridBase::~PrefGridBase() {}


PrefGridBase::PrefGroup* PrefGridBase::AddGroup(const wxString& title) {

    PrefGroup* group = new PrefGroup(this, title);

    wxStaticText* groupLabel = new wxStaticText(this, wxID_ANY, title);
    wxFont font = groupLabel->GetFont();
    font.SetWeight(wxBOLD);
    groupLabel->SetFont(font);

    m_groupSizer->Add(groupLabel, wxGBPosition(GetTotalSize(), 0), wxGBSpan(1, 2), wxALL, 3);
    m_groupList.push_back(group);
    //Fit();
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

//BEGIN_EVENT_TABLE(PrefGridBase::PrefValueBase, wxEvtHandler)
////    EVT_ENTER_WINDOW(PrefNodeBase::PrefValueBase::OnMouseLeave)
////    EVT_LEAVE_WINDOW(PrefNodeBase::PrefValueBase::OnMouseLeave)
//    EVT_COMMAND_LEFT_CLICK(wxID_ANY, PrefGridBase::PrefValueBase::OnClick)
//END_EVENT_TABLE()

PrefGridBase::PrefValueBase::PrefValueBase(
            PrefGridBase* parent) : wxEvtHandler(), m_grid(parent)
{

}

PrefGridBase::PrefValueBase::PrefValueBase(
    PrefGridBase* parent,
    const wxString& label,
    const wxString& helpText,
    const wxString& helpDefault,
    const wxValidator& val
    ) : wxEvtHandler(), m_grid(parent), m_label(label), m_helpText(helpText), m_default(helpDefault)
{
    m_validator = (wxValidator*) val.Clone();
}


wxPanel* PrefGridBase::PrefValueBase::CreateControls() {

    m_labelPanel = new wxPanel(m_grid);
    m_labelPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    m_labelPanel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));

    m_controlPanel = new wxPanel(m_grid);
    m_controlPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    m_controlPanel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));

    m_labelCtrl = new wxStaticText(m_labelPanel, wxID_ANY, m_label, wxDefaultPosition, wxDefaultSize);
    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    s->Add(m_labelCtrl, 0, wxALL, 3);
    m_labelPanel->SetSizer(s);

    m_labelPanel->SetHelpText(m_helpText);

    //inputCtrl->SetValidator(val);

    m_grid->m_groupSizer->Add(m_labelPanel, wxGBPosition(m_grid->GetTotalSize(), 0), wxDefaultSpan, wxEXPAND);
    m_grid->m_groupSizer->Add(m_controlPanel, wxGBPosition(m_grid->GetTotalSize(), 1), wxDefaultSpan, wxEXPAND);

    //m_labelPanel->PushEventHandler(this);
    m_labelPanel->Connect(wxEVT_LEFT_DOWN,
        wxMouseEventHandler(PrefGridBase::PrefValueBase::OnClick), 0, this);

    m_labelCtrl->Connect(wxEVT_LEFT_DOWN,
        wxMouseEventHandler(PrefGridBase::PrefValueBase::OnClick), 0, this);

    m_controlPanel->Connect(wxEVT_LEFT_DOWN,
        wxMouseEventHandler(PrefGridBase::PrefValueBase::OnClick), 0, this);

    return m_controlPanel;
}


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


void PrefGridBase::PrefValueBase::Deselect() {

    m_labelPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    m_labelCtrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));

    m_labelPanel->Refresh();

    m_grid->m_selected = 0;
}


void PrefGridBase::PrefValueBase::OnClick(wxMouseEvent& event) {

    m_controlPanel->SetFocus();
    event.Skip();
}


void PrefGridBase::PrefValueBase::OnFocus(wxFocusEvent& event) {

    Select();
    event.Skip();
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

    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    s->Add(m_text, 0, wxALL | wxEXPAND, 3);
    m_controlPanel->SetSizer(s);

    m_text->Connect(wxEVT_SET_FOCUS,
        wxFocusEventHandler(PrefGridBase::PrefValueBase::OnFocus), 0, this);

    return m_controlPanel;
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

    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    s->Add(m_combo, 0, wxALL | wxEXPAND, 0);
    m_controlPanel->SetSizer(s);


    m_combo->Connect(wxEVT_LEFT_DOWN,
        wxMouseEventHandler(PrefGridBase::PrefValueBase::OnClick), 0, this);

    m_combo->Connect(wxEVT_SET_FOCUS,
        wxFocusEventHandler(PrefGridBase::PrefValueBase::OnFocus), 0, this);

    return m_controlPanel;
}