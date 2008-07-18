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

#ifndef _VALIDATETIME_H_
#define _VALIDATETIME_H_

#include "wx/valtext.h"
#include "prefs.h"


class CValidateTime : public wxTextValidator {

    DECLARE_CLASS(CValidateTime)

public:
    CValidateTime() : wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST) {}
    CValidateTime(double* val) : wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST), m_time(val) {
        const wxChar *numbers[] = {
           wxT("0"), wxT("1"), wxT("2"), wxT("3"), wxT("4"),
           wxT("5"), wxT("6"), wxT("7"), wxT("8"), wxT("9"),
           wxT(":")
        };
        SetIncludes(wxArrayString(11, numbers));
    }
    CValidateTime(const CValidateTime& val): wxTextValidator(val) { m_time = val.m_time; }
    ~CValidateTime() {};

    virtual wxObject*   Clone() const { return new CValidateTime(*this); }

    virtual bool        TransferToWindow();
    virtual bool        TransferFromWindow();
    virtual bool        Validate(wxWindow* parent);
    double              GetTime() const { return *m_time; }

    static bool         TimeStringToDouble(wxString timeStr, double& time);
    static wxString     DoubleToTimeString(double dt);

protected:
    double*             m_time;
};

class CValidateTimeSpan : public wxTextValidator {

    DECLARE_CLASS(CValidateTimeSpan)

public:
    CValidateTimeSpan(TIME_PREFS* val, int day) : m_prefs(val), m_day(day) {}
    CValidateTimeSpan(const CValidateTimeSpan& val): m_prefs(val.m_prefs), m_day(val.m_day) {}
    ~CValidateTimeSpan() {};

    virtual wxObject*   Clone() const { return new CValidateTimeSpan(*this); }

    virtual bool        TransferToWindow();
    virtual bool        TransferFromWindow();
    virtual bool        Validate(wxWindow* parent);

protected:
    bool                ParseTimeSpan(wxString timeStr, TIME_SPAN* span);

    TIME_PREFS*         m_prefs;
    int                 m_day;
};

#endif // _VALIDATETIME_H_
