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

#ifndef _VALIDATENUMBER_H_
#define _VALIDATENUMBER_H_

#include "stdwx.h"

// Templated validator to handle any numeric type. T must
// support insertion and extraction operators or it will all fail horribly.
template<class T>
class CValidateNumber : public wxTextValidator {
public:
    CValidateNumber() {}
    CValidateNumber(T* val)
        : wxTextValidator(wxFILTER_NUMERIC),
        m_value(val),
        m_min(0),
        m_max(0),
        m_constrained(false),
        m_clamped(false) {}
    CValidateNumber(T* val, T min, T max, bool clamped = true)
        : wxTextValidator(wxFILTER_NUMERIC),
        m_value(val),
        m_min(min),
        m_max(max),
        m_constrained(true),
        m_clamped(clamped) {}
    CValidateNumber(const CValidateNumber<T>& val) { Copy(val); }
    ~CValidateNumber() {}

    virtual wxObject*   Clone() const { return new CValidateNumber<T>(*this); }
    virtual bool        Copy(const CValidateNumber& val);

    virtual bool        Validate(wxWindow *parent);
    virtual bool        TransferToWindow();
    virtual bool        TransferFromWindow();

protected:
    virtual T           TranslateTo(T val) { return val; }
    virtual T           TranslateFrom(T val) { return val; }
    bool                Parse(const wxString& value, T& result);

    T*                  m_value;
    T                   m_min;
    T                   m_max;
    bool                m_constrained;
    bool                m_clamped;
};


template<class T>
bool CValidateNumber<T>::Copy(const CValidateNumber& val) {

    wxTextValidator::Copy(val);

    m_value = val.m_value;
    m_min = val.m_min;
    m_max = val.m_max;
    m_constrained = val.m_constrained;
    m_clamped = val.m_clamped;

    return true;
}


template<class T>
bool CValidateNumber<T>::Validate(wxWindow* parent) {
 
    bool valid = wxTextValidator::Validate(parent);
    if (!valid || !m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl))) {

        wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
        T val = 0;
        if (m_value && Parse(pControl->GetValue(), val)) {

            if (m_constrained) {
                // TODO: msgbox
                return (val >= m_min && val <= m_max);
            }
            return true;
        }
    }
    return false;
}


template<class T>
bool CValidateNumber<T>::TransferToWindow() {

    if (!m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl))) {

        wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
        if (m_value)
        {
            wxString s;
            T val = TranslateTo(*m_value);
            s << val;
            pControl->SetValue(s);
            return true;
        }
    }
    return false;
}


template<class T>
bool CValidateNumber<T>::TransferFromWindow() {

    if (!m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl))) {
        wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
        T val = 0;
        if (m_value && Parse(pControl->GetValue(), val)) {

            *m_value = val;
            return true;
        }
    }
    return false;
}


template<class T>
bool CValidateNumber<T>::Parse(const wxString& value, T& result) {

    std::basic_istringstream<wxChar> iss(value.c_str());
    iss >> result;

    if (! iss.fail()) {
        result = TranslateFrom(result);

        if (m_clamped) {
            result = (result > m_min) ? result : m_min;
            result = (result < m_max) ? result : m_max;
        }
        return true;
    }
    return false;
}

#endif // _VALIDATENUMBER_H_
