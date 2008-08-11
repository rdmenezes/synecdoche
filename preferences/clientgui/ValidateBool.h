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

#ifndef _VALIDATEBOOL_H_
#define _VALIDATEBOOL_H_

// wxGenericValidator does all we need for a boolean (checkbox) validator.
// Rename it for convenience and clarity:
typedef wxGenericValidator ValidateBool;

// ValidateBoolInverse reverses the checkbox meaning. Checked == false, Not checked == true.
// Useful when the sense of the backing variable does not match the prompt.
class ValidateBoolInverse : public ValidateBool {

    DECLARE_CLASS(ValidateBool)

public:

    ValidateBoolInverse(bool* val) : ValidateBool(val) {};
    ValidateBoolInverse(const ValidateBoolInverse& val): ValidateBool(val) {};

    ~ValidateBoolInverse() {};

    virtual wxObject* Clone() const { return new ValidateBoolInverse(*this); }
    //virtual bool      Copy(const ValidateBoolInverse& val);

    virtual bool      TransferToWindow();
    virtual bool      TransferFromWindow();
};


#endif // _VALIDATEBOOL_H_
