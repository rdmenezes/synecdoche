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

#ifndef _PREFNODEBASE_H_
#define _PREFNODEBASE_H_

#include "prefs.h"
#include "ValidateBool.h"
#include "ValidateNumber.h"
#include "ValidateTime.h"


enum PrefNodeType {
    Presets = 7000,
    General,
    Processor,
    ProcessorTimes,
    Network,
    NetworkTimes,
    Memory,
    Disk,
};


/// Days of the week.
/// Used by day-specific preferences.
enum DayOfWeek { 
    Sunday,
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
};

DECLARE_EVENT_TYPE(PREF_EVT_CMD_UPDATE, -1)

class PrefNodeBase;


/// Base class for all preference pages.
/// 
class PrefNodeBase : public wxScrolledWindow {

    DECLARE_DYNAMIC_CLASS(PrefNodeBase)

public:
    PrefNodeBase(wxWindow* parent = NULL, GLOBAL_PREFS* preferences = NULL);
    virtual ~PrefNodeBase();

    wxWindow* GetHelpAtPoint(const wxPoint& p);

    static PrefNodeBase* Create(PrefNodeType nodeType, wxWindow* parent, GLOBAL_PREFS* preferences);

protected:
    class PrefGroup;
    class PrefValueBase;

    PrefGroup*     AddGroup(const wxString& title);
    void            AddPreference(PrefValueBase* pref);

    GLOBAL_PREFS*   m_preferences;

private:
    class PrefValueTimeSpan;
    wxBoxSizer* m_groupSizer;
    std::vector<PrefGroup*> m_groupList;
    std::vector<PrefValueBase*> m_prefList;

// Nested utility classes:
protected:

    /// A group of preferences.
    /// In the base implementation, groups are rendered as a static box.
    class PrefGroup : public wxStaticBoxSizer {
    public:
        PrefGroup(wxWindow* win, const wxString& title);
        ~PrefGroup();

        void AddPreference(PrefValueBase* pref);

    private:
        PrefNodeBase* m_page;

    };

    /// Base class for individual preference widgets.
    /// Abstract.
    class PrefValueBase : public wxPanel {

        DECLARE_DYNAMIC_CLASS(PrefValueBase)
        DECLARE_EVENT_TABLE()

    public:
        PrefValueBase(wxWindow* parent = NULL);

        PrefValueBase(
            wxWindow* parent,
            const wxString& helpText
        );

    protected:

        virtual void    OnMouseLeave(wxMouseEvent& event);
        virtual void    OnFocus(wxChildFocusEvent& event);
        void            OnCreate(wxWindowCreateEvent& event);
    };


    class PrefValueText : public PrefValueBase {

        DECLARE_DYNAMIC_CLASS(PrefValueNumber)

    public:
        PrefValueText(wxWindow* parent = NULL);

        PrefValueText(
            wxWindow* parent,
            const wxString& prompt,
            const wxString& units,
            const wxString& helpText,
            const wxTextValidator& val
        );

    protected:
        wxStaticText*       m_prompt;
        wxStaticText*       m_units;
    };


    class PrefValueBool : public PrefValueBase {

        DECLARE_DYNAMIC_CLASS(PrefValueBool)

    public:
        PrefValueBool(wxWindow* parent = NULL);

        PrefValueBool(
            wxWindow* parent,
            const wxString& prompt,
            const wxString& helpText,
            const ValidateBool& val
        );

    protected:
        wxCheckBox* m_check;

    };

    class PrefValueButton : public PrefValueBase {

        DECLARE_DYNAMIC_CLASS(PrefValueButton)

    public:
        PrefValueButton(wxWindow* parent = NULL);

        PrefValueButton(
            wxWindow* parent,
            const wxString& prompt,
            const wxString& title,
            const wxString& helpText,
            wxWindowID id
        );
    };

    class PrefValueTime : public PrefValueBase {

        DECLARE_DYNAMIC_CLASS(PrefValueTime)
        DECLARE_EVENT_TABLE()

    public:
        PrefValueTime(wxWindow* parent = NULL);

        PrefValueTime(
            wxWindow* parent,
            const wxString& prompt,
            const wxString& helpText,
            TIME_SPAN* time
        );

        void Update();

    protected:
        void SetState(TIME_SPAN::TimeMode mode);
        void OnBetweenChanged(wxCommandEvent& event);
        void OnTextChanged(wxCommandEvent& event);

        TIME_SPAN* m_time;
        wxTextCtrl* m_startCtrl;
        wxTextCtrl* m_endCtrl;

        wxRadioButton* m_rbAlways;
        wxRadioButton* m_rbNever;
        wxRadioButton* m_rbBetween;
    };

    class PrefValueWeek : public PrefValueBase {

        DECLARE_DYNAMIC_CLASS(PrefValueWeek)

    public:
        PrefValueWeek(wxWindow* parent = NULL);

        PrefValueWeek(
            wxWindow* parent,
            const wxString& prompt,
            const wxString& helpText,
            TIME_PREFS* prefs
        );

        void Update();
        void                    OnUpdateUI(wxCommandEvent& event);

    protected:
        TIME_PREFS*             m_prefs;
        PrefValueTimeSpan*     m_dayWidgets[7];
    };


private:
    class PrefValueTimeSpan : public wxPanel {

        DECLARE_DYNAMIC_CLASS(PrefValueTimeSpan)
        DECLARE_EVENT_TABLE()

    public:
        PrefValueTimeSpan(wxWindow* parent = NULL);

        PrefValueTimeSpan(
            wxWindow* parent,
            DayOfWeek day,
            TIME_PREFS* prefs
        );

        void Update();
        void UpdateDefault();

    protected:
        void OnUseDefaultChanged(wxCommandEvent& event);

        wxCheckBox* m_check;
        wxTextCtrl*     m_timeText;
        DayOfWeek m_day;
        TIME_PREFS* m_prefs;
        
    };
    // End of nested classes
};


#endif // _PREFNODEBASE_H_
