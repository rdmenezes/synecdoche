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

#ifndef _VALIDATEPERCENT_H_
#define _VALIDATEPERCENT_H_

#include "ValidateNumber.h"

// In addition to the ValidateNumber requirements, T must cast to various integer values.
template<class T>
class ValidatePercent : public ValidateNumber<T> {
public:
    ValidatePercent(T* val) : ValidateNumber<T>(val, (T)0, (T)1) {}
    ValidatePercent(const ValidateNumber<T>& val) { Copy(val); }
    ~ValidatePercent() {}

    virtual wxObject*   Clone() const { return new ValidatePercent<T>(*this); }

protected:
    virtual T           TranslateTo(T val) { return val * (T)100; }
    virtual T           TranslateFrom(T val) { return val / (T)100; }
};

#endif // _VALIDATEPERCENT_H_
