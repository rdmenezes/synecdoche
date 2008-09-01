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

#ifndef _VALIDATENUMBER_H_
#define _VALIDATENUMBER_H_

// Templated validator to handle any numeric type. T must
// support insertion and extraction operators or it will all fail horribly.
template<class T>
class ValidateNumber : public wxTextValidator {
public:
    ValidateNumber() {}
    ValidateNumber(T* val)
        : wxTextValidator(wxFILTER_NUMERIC),
        m_value(val),
        m_min(0),
        m_max(0),
        m_constrained(false),
        m_clamped(false) {}
    ValidateNumber(T* val, T min, T max, bool clamped = true)
        : wxTextValidator(wxFILTER_NUMERIC),
        m_value(val),
        m_min(min),
        m_max(max),
        m_constrained(true),
        m_clamped(clamped) {}
    ValidateNumber(const ValidateNumber<T>& val) { Copy(val); }
    ~ValidateNumber() {}

    virtual wxObject*   Clone() const { return new ValidateNumber<T>(*this); }
    virtual bool        Copy(const ValidateNumber& val);

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
bool ValidateNumber<T>::Copy(const ValidateNumber& val) {

    wxTextValidator::Copy(val);

    m_value = val.m_value;
    m_min = val.m_min;
    m_max = val.m_max;
    m_constrained = val.m_constrained;
    m_clamped = val.m_clamped;

    return true;
}


template<class T>
bool ValidateNumber<T>::Validate(wxWindow* parent) {
 
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
bool ValidateNumber<T>::TransferToWindow() {

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
bool ValidateNumber<T>::TransferFromWindow() {

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
bool ValidateNumber<T>::Parse(const wxString& value, T& result) {

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
