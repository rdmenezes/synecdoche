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

#ifndef _VALIDATEMULTIPLIED_H_
#define _VALIDATEMULTIPLIED_H_

#include "ValidateNumber.h"

template<class T>
class ValidateMultiplied : public ValidateNumber<T> {
public:
    ValidateMultiplied(T* val, T multiplier) : ValidateNumber<T>(val),
    m_multiplier(multiplier) {}
    ValidateMultiplied(const ValidateMultiplied<T>& val) { Copy(val); }
    ~ValidateMultiplied() {}

    virtual wxObject*   Clone() const { return new ValidateMultiplied<T>(*this); }
    virtual bool        Copy(const ValidateMultiplied& val);

protected:
    virtual T           TranslateTo(T val) { return val * m_multiplier; }
    virtual T           TranslateFrom(T val) { return val / m_multiplier; }

    T                   m_multiplier;
};


template<class T>
bool ValidateMultiplied<T>::Copy(const ValidateMultiplied& val) {

    ValidateNumber<T>::Copy(val);
    m_multiplier = val.m_multiplier;
    return true;
}

#endif // _VALIDATEMULTIPLIED_H_
