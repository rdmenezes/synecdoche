// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2005 University of California
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
//

#ifndef _DLGGENERICMESSAGE_H_
#define _DLGGENERICMESSAGE_H_

#include <wx/dialog.h>

class wxStaticText;
class wxCheckBox;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10000
#define SYMBOL_CDLGGENERICMESSAGE_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CDLGGENERICMESSAGE_TITLE _T("")
#define SYMBOL_CDLGGENERICMESSAGE_IDNAME ID_DIALOG
#define SYMBOL_CDLGGENERICMESSAGE_SIZE wxSize(400, 300)
#define SYMBOL_CDLGGENERICMESSAGE_POSITION wxDefaultPosition
#define ID_DISABLEDIALOG 10017
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * CDlgGenericMessage class declaration
 */

class CDlgGenericMessage: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CDlgGenericMessage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgGenericMessage( );
    CDlgGenericMessage( wxWindow* parent, wxWindowID id = SYMBOL_CDLGGENERICMESSAGE_IDNAME, const wxString& caption = SYMBOL_CDLGGENERICMESSAGE_TITLE, const wxPoint& pos = SYMBOL_CDLGGENERICMESSAGE_POSITION, const wxSize& size = SYMBOL_CDLGGENERICMESSAGE_SIZE, long style = SYMBOL_CDLGGENERICMESSAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGGENERICMESSAGE_IDNAME, const wxString& caption = SYMBOL_CDLGGENERICMESSAGE_TITLE, const wxPoint& pos = SYMBOL_CDLGGENERICMESSAGE_POSITION, const wxSize& size = SYMBOL_CDLGGENERICMESSAGE_SIZE, long style = SYMBOL_CDLGGENERICMESSAGE_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CDlgGenericMessage event handler declarations

////@end CDlgGenericMessage event handler declarations

////@begin CDlgGenericMessage member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDlgGenericMessage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDlgGenericMessage member variables
    wxStaticText* m_DialogMessage;
    wxCheckBox* m_DialogDisableMessage;
////@end CDlgGenericMessage member variables
};

#endif
    // _DLGGENERICMESSAGE_H_
