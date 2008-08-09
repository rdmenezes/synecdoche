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
#include "PrefNodeBase.h"
#include "PrefTreeBook.h"
#include "PrefNodePresets.h"
#include "PrefNodeGeneral.h"
#include "PrefNodeProcessor.h"
#include "PrefNodeProcessorTimes.h"
#include "PrefNodeNetwork.h"
#include "PrefNodeNetworkTimes.h"
#include "PrefNodeMemory.h"
#include "PrefNodeDisk.h"


DEFINE_EVENT_TYPE(PREF_EVT_CMD_UPDATE)

IMPLEMENT_DYNAMIC_CLASS(PrefNodeBase, wxPanel)


PrefNodeBase::PrefNodeBase(wxWindow* parent, GLOBAL_PREFS* preferences)
: wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME),
m_preferences(preferences) {

    SetScrollRate(10, 10);

    m_groupSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(m_groupSizer);
}


PrefNodeBase::~PrefNodeBase() {}


PrefNodeBase::PrefGroup* PrefNodeBase::AddGroup(const wxString& title) {

    PrefGroup* group = new PrefGroup(this, title);
    m_groupSizer->Add(group, 0, wxEXPAND | wxALL, 4);
    m_groupList.push_back(group);
    Fit();
    return group;
}


void PrefNodeBase::AddPreference(PrefValueBase* pref) {
    m_prefList.push_back(pref);
}


wxWindow* PrefNodeBase::GetHelpAtPoint(const wxPoint& p) {

    wxPoint p1 = ScreenToClient(p);
    wxWindow* source = 0;
    wxRect r;

    std::vector<PrefGroup*>::iterator i = m_groupList.begin();
    while (i != m_groupList.end()) {

        PrefGroup* group = *i;

        r = group->GetStaticBox()->GetRect();

        if (r.Contains(p1)) {
            source = this;
        }
        i++;
    }

    // Iterate children....
    // Lots of room for optimisation. Start with a naive implementation.
    
    std::vector<PrefValueBase*>::iterator j = m_prefList.begin();
    while (j != m_prefList.end()) {

        wxWindow* win = *j;
        r = win->GetRect();

        if (win->IsEnabled() && r.Contains(p1)) {
            return win;
        }
        j++;
    }
    return source;
}


// Class factory pattern.... Yay! Design patterns!
PrefNodeBase* PrefNodeBase::Create(PrefNodeType nodeType, wxWindow* parent, GLOBAL_PREFS* preferences) {

    switch(nodeType) {
    case Presets:
        return new PrefNodePresets(parent, preferences);
    case General:
        return new PrefNodeGeneral(parent, preferences);
    case Processor:
        return new PrefNodeProcessor(parent, preferences);
    case ProcessorTimes:
        return new PrefNodeProcessorTimes(parent, preferences);
    case Network:
        return new PrefNodeNetwork(parent, preferences);
    case NetworkTimes:
        return new PrefNodeNetworkTimes(parent, preferences);
    case Memory:
        return new PrefNodeMemory(parent, preferences);
    case Disk:
        return new PrefNodeDisk(parent, preferences);
    default:
        break;
    }
    return NULL;
}


// Nested classes

// PrefGroup implementation

PrefNodeBase::PrefGroup::PrefGroup(wxWindow* win, const wxString& title)
: wxStaticBoxSizer(wxVERTICAL, win, title) {

    wxASSERT(win->IsKindOf(CLASSINFO(PrefNodeBase)));
    m_page = (PrefNodeBase*) win;
}

PrefNodeBase::PrefGroup::~PrefGroup() {}


void PrefNodeBase::PrefGroup::AddPreference(PrefValueBase* pref) {

    Add(pref, 0, wxEXPAND | wxALL, 2);
    m_page->AddPreference(pref);
}


// PrefValueBase implementation

IMPLEMENT_DYNAMIC_CLASS(PrefNodeBase::PrefValueBase, wxPanel)

BEGIN_EVENT_TABLE(PrefNodeBase::PrefValueBase, wxPanel)
    EVT_ENTER_WINDOW(PrefNodeBase::PrefValueBase::OnMouseLeave)
    EVT_LEAVE_WINDOW(PrefNodeBase::PrefValueBase::OnMouseLeave)
    EVT_CHILD_FOCUS(PrefNodeBase::PrefValueBase::OnFocus)
END_EVENT_TABLE()

PrefNodeBase::PrefValueBase::PrefValueBase(
            wxWindow* parent) : wxPanel(parent), m_xmlElementName(wxEmptyString)
{

}

PrefNodeBase::PrefValueBase::PrefValueBase(
            wxWindow* parent,
            const wxString& xmlElementName,
            const wxString& helpText
            ) : wxPanel(parent), m_xmlElementName(xmlElementName)
{
    SetHelpText(helpText);
}

// TODO: Enter/Leave events are unreliable. Augment events with mouse polling.
// We keep the events because they make the response more snappy, the 90% of the time when they work.
void PrefNodeBase::PrefValueBase::OnMouseLeave(wxMouseEvent& WXUNUSED(event)) {

    // One method does the heavy lifting for both enter and leave events. We determine whether
    // to treat it as an enter or leave depending on the mouse position, NOT the event type.
    // This is because we don't get events from the child windows, but entering a child window
    // generates a leave event for the parent. Daft, but what can you do?

    //wxWindow* parent = GetParent();
    //wxASSERT(parent->IsKindOf(CLASSINFO(PrefNodeBase)));
    //PrefNodeBase* page = wxDynamicCast(parent, PrefNodeBase);

    //wxWindow* source = page->GetHelpAtPoint(wxGetMousePosition());

    //PrefHelpEvent e(PREF_EVT_HELP_CMD, GetId());
    //e.SetTrigger(PrefHelpEvent::Mouse);
    //e.SetEventObject(source);
    //GetEventHandler()->ProcessEvent(e);
}


void PrefNodeBase::PrefValueBase::OnFocus(wxChildFocusEvent& WXUNUSED(event)) {

    //PrefHelpEvent e(PREF_EVT_HELP_CMD, GetId());
    //e.SetTrigger(PrefHelpEvent::Focus);
    //e.SetEventObject(this);
    //GetEventHandler()->ProcessEvent(e);
}

// PrefValueText implementation

IMPLEMENT_DYNAMIC_CLASS(PrefNodeBase::PrefValueText, PrefNodeBase::PrefValueBase)

PrefNodeBase::PrefValueText::PrefValueText(
            wxWindow* parent) : PrefNodeBase::PrefValueBase(parent)
{

}


PrefNodeBase::PrefValueText::PrefValueText(
            wxWindow* parent,
            const wxString& xmlElementName,
            const wxString& prompt,
            const wxString& units,
            const wxString& helpText,
            const wxTextValidator& val
            ) : PrefNodeBase::PrefValueBase(
            parent, xmlElementName, helpText)
{
    wxBoxSizer* line = new wxBoxSizer(wxHORIZONTAL);

    m_prompt = new wxStaticText(this, wxID_ANY, prompt);
    wxTextCtrl* inputCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RIGHT);
    // Set aspect ratio of edit control:
    inputCtrl->SetMinSize(wxSize(inputCtrl->GetSize().y * 2, -1));
    m_units = new wxStaticText(this, wxID_ANY, units);

    inputCtrl->SetValidator(val);

    line->Add(m_prompt, 0, wxALL | wxCENTER, 2);
    line->Add(inputCtrl, 0, wxALL | wxCENTER, 2);
    line->Add(m_units, 0, wxALL | wxCENTER, 2);

    SetSizer(line);

}

// PrefValueBool implementation

IMPLEMENT_DYNAMIC_CLASS(PrefNodeBase::PrefValueBool, PrefNodeBase::PrefValueBase)

PrefNodeBase::PrefValueBool::PrefValueBool(
            wxWindow* parent) : PrefNodeBase::PrefValueBase(parent)
{

}

PrefNodeBase::PrefValueBool::PrefValueBool(
            wxWindow* parent,
            const wxString& xmlElementName,
            const wxString& prompt,
            const wxString& helpText,
            const CValidateBool& val
            ) : PrefNodeBase::PrefValueBase(
            parent, xmlElementName, helpText)
{
    wxBoxSizer* line = new wxBoxSizer(wxHORIZONTAL);
    m_check = new wxCheckBox(this, wxID_ANY, prompt);
    m_check->SetValidator(val);

    line->Add(m_check, 0, wxALL | wxCENTER, 2);
    SetSizer(line);

}


// PrefValueButton implementation

IMPLEMENT_DYNAMIC_CLASS(PrefNodeBase::PrefValueButton, PrefNodeBase::PrefValueBase)

PrefNodeBase::PrefValueButton::PrefValueButton(
            wxWindow* parent) : PrefNodeBase::PrefValueBase(parent)
{

}

PrefNodeBase::PrefValueButton::PrefValueButton(
            wxWindow* parent,
            const wxString& xmlElementName,
            const wxString& prompt,
            const wxString& title,
            const wxString& helpText,
            wxWindowID id
            ) : PrefNodeBase::PrefValueBase(
            parent, xmlElementName, helpText)
{
    wxBoxSizer* lines = new wxBoxSizer(wxVERTICAL);
    wxStaticText* promptTxt = new wxStaticText(this, wxID_ANY, prompt);
    promptTxt->Wrap(400);
    wxButton* button = new wxButton(this, id, title);

    lines->Add(promptTxt, 0, wxEXPAND | wxALL, 2);
    lines->Add(button, 0, wxALL, 2);

    SetSizer(lines);
}


// PrefValueTime implementation

IMPLEMENT_DYNAMIC_CLASS(PrefNodeBase::PrefValueTime, PrefNodeBase::PrefValueBase)

BEGIN_EVENT_TABLE(PrefNodeBase::PrefValueTime, PrefNodeBase::PrefValueBase)
    EVT_RADIOBUTTON(wxID_ANY, PrefNodeBase::PrefValueTime::OnBetweenChanged)
    EVT_TEXT(wxID_ANY, PrefNodeBase::PrefValueTime::OnTextChanged)
END_EVENT_TABLE()

PrefNodeBase::PrefValueTime::PrefValueTime(
            wxWindow* parent) : PrefNodeBase::PrefValueBase(parent)
{

}


PrefNodeBase::PrefValueTime::PrefValueTime(
            wxWindow* parent,
            const wxString& prompt,
            const wxString& helpText,
            TIME_SPAN* time
            ) : PrefNodeBase::PrefValueBase(
            parent, wxEmptyString, helpText), m_time(time)
{
    wxBoxSizer* lines = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* betweenLine = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* promptText = new wxStaticText(this, wxID_ANY, prompt);

    m_rbAlways = new wxRadioButton(this, TIME_SPAN::Always, _("Always"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_rbNever = new wxRadioButton(this, TIME_SPAN::Never, _("Never"));
    m_rbBetween = new wxRadioButton(this, TIME_SPAN::Between, _("Between"));

    lines->Add(promptText, 0, wxALL, 2);
    lines->Add(m_rbAlways, 1, wxALL, 2);
    lines->Add(m_rbNever, 1, wxALL, 2);

    m_startCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTER);
    wxStaticText* andPrompt = new wxStaticText(this, wxID_ANY, _("and"));
    m_endCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTER);
    // Set aspect ratio of edit controls:
    m_startCtrl->SetMinSize(wxSize(m_startCtrl->GetSize().y * 3, -1));
    m_endCtrl->SetMinSize(wxSize(m_endCtrl->GetSize().y * 3, -1));

    m_startCtrl->SetValidator(CValidateTime(&m_time->start_hour));
    m_endCtrl->SetValidator(CValidateTime(&m_time->end_hour));

    betweenLine->Add(m_rbBetween, 0, wxALL | wxCENTER, 2);
    betweenLine->Add(m_startCtrl, 0, wxALL | wxCENTER, 2);
    betweenLine->Add(andPrompt, 0, wxALL | wxCENTER, 2);
    betweenLine->Add(m_endCtrl, 0, wxALL | wxCENTER, 2);

    lines->Add(betweenLine, 1);

    SetSizer(lines);

    Update();
}


void PrefNodeBase::PrefValueTime::OnBetweenChanged(wxCommandEvent& event) {

    SetState((TIME_SPAN::TimeMode) event.GetId());
}


void PrefNodeBase::PrefValueTime::SetState(TIME_SPAN::TimeMode mode) {

    switch (mode) {
    case TIME_SPAN::Always:
        m_startCtrl->SetValue(_("00:00"));
        m_endCtrl->SetValue(_("24:00"));
        break;
    case TIME_SPAN::Never:
        m_startCtrl->SetValue(_("24:00"));
        m_endCtrl->SetValue(_("00:00"));
        break;
    default:
        break;
    }

    m_startCtrl->Enable(mode == TIME_SPAN::Between);
    m_endCtrl->Enable(mode == TIME_SPAN::Between);
}

void PrefNodeBase::PrefValueTime::Update() {

    m_startCtrl->GetValidator()->TransferToWindow();
    m_endCtrl->GetValidator()->TransferToWindow();

    TIME_SPAN::TimeMode mode = m_time->mode();

    switch (mode) {
    case TIME_SPAN::Always:
        m_rbAlways->SetValue(true);
        break;
    case TIME_SPAN::Never:
        m_rbNever->SetValue(true);
        break;
    default:
        m_rbBetween->SetValue(true);
        break;
    }

    m_startCtrl->Enable(mode == TIME_SPAN::Between);
    m_endCtrl->Enable(mode == TIME_SPAN::Between);
}

void PrefNodeBase::PrefValueTime::OnTextChanged(wxCommandEvent& event) {

    if (m_startCtrl->GetValidator()->TransferFromWindow()
        && m_endCtrl->GetValidator()->TransferFromWindow()) {

        wxCommandEvent e(PREF_EVT_CMD_UPDATE, GetId());
        e.SetEventObject(this);
        GetEventHandler()->ProcessEvent(e);
    }
    event.Skip();
}

// PrefValueWeek implementation

IMPLEMENT_DYNAMIC_CLASS(PrefNodeBase::PrefValueWeek, PrefNodeBase::PrefValueBase)

PrefNodeBase::PrefValueWeek::PrefValueWeek(
            wxWindow* parent) : PrefNodeBase::PrefValueBase(parent)
{

}


PrefNodeBase::PrefValueWeek::PrefValueWeek(
            wxWindow* parent,
            const wxString& prompt,
            const wxString& helpText,
            TIME_PREFS* prefs
            ) : PrefNodeBase::PrefValueBase(
            parent, wxEmptyString, helpText), m_prefs(prefs)
{

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* promptText = new wxStaticText(this, wxID_ANY, prompt);
    sizer->Add(promptText, 0, wxALL, 2);

    // Monday first.
    for (int i = 1; i < 8; i++) {
        
        DayOfWeek day = (DayOfWeek) (i % 7); // Wrap back to Sunday.
        PrefValueTimeSpan* spanWidget = new PrefValueTimeSpan(
            this, day, prefs);
        sizer->Add(spanWidget, 0, wxEXPAND);
        m_dayWidgets[day] = spanWidget;
    }

    SetSizer(sizer);

}

void PrefNodeBase::PrefValueWeek::Update() {

    for (int i = 0; i < 7; i++) {
        
        PrefValueTimeSpan* spanWidget = m_dayWidgets[i];
        spanWidget->Update();
    }
}

void PrefNodeBase::PrefValueWeek::OnUpdateUI(wxCommandEvent& WXUNUSED(event)) {

    for (int i = 0; i < 7; i++) {
        
        PrefValueTimeSpan* spanWidget = m_dayWidgets[i];
        spanWidget->UpdateDefault();
    }
}


// PrefValueTimeSpan implementation

IMPLEMENT_DYNAMIC_CLASS(PrefNodeBase::PrefValueTimeSpan, wxPanel)

BEGIN_EVENT_TABLE(PrefNodeBase::PrefValueTimeSpan, wxPanel)
    EVT_CHECKBOX(wxID_ANY, PrefNodeBase::PrefValueTimeSpan::OnUseDefaultChanged)
END_EVENT_TABLE()

PrefNodeBase::PrefValueTimeSpan::PrefValueTimeSpan(
            wxWindow* parent) : wxPanel(parent)
{

}


PrefNodeBase::PrefValueTimeSpan::PrefValueTimeSpan(
            wxWindow* parent,
            DayOfWeek day,
            TIME_PREFS* prefs
            ) : wxPanel(parent), m_day(day), m_prefs(prefs)
{

    const wxString dayName[] = {
           _("Sunday"), _("Monday"), _("Tuesday"), _("Wednesday"),
           _("Thursday"), _("Friday"), _("Saturday")
    };

    bool present = (prefs->week.get(day) != 0);

    wxBoxSizer* line = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* dayTxt = new wxStaticText(this, wxID_ANY, dayName[day], wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);

    m_check = new wxCheckBox(this, wxID_ANY, wxEmptyString);
    

    m_timeText = new wxTextCtrl(this, wxID_ANY);
    m_timeText->SetValidator(CValidateTimeSpan(prefs, day));
    m_timeText->Enable(present);

    line->Add(dayTxt, 1, wxALL | wxCENTER, 2);
    line->Add(m_check, 0, wxALL | wxCENTER, 2);
    line->Add(m_timeText, 3, wxALL | wxCENTER, 2);

    SetSizer(line);
    
    m_check->SetValue(present); 

}

void PrefNodeBase::PrefValueTimeSpan::OnUseDefaultChanged(wxCommandEvent& event) {
    
    m_timeText->Enable(event.IsChecked());
    m_timeText->GetValidator()->TransferFromWindow();
    UpdateDefault();
}

void PrefNodeBase::PrefValueTimeSpan::Update() {

    bool present = (m_prefs->week.get(m_day) != 0);

    m_timeText->GetValidator()->TransferToWindow();
    m_timeText->Enable(present);
    m_check->SetValue(present);
}

void PrefNodeBase::PrefValueTimeSpan::UpdateDefault() {

    if (!m_timeText->IsEnabled()) {
        
        m_timeText->GetValidator()->TransferToWindow();
    }
}