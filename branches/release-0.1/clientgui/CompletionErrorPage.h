// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Peter Kortschack
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
#ifndef WIZ_COMPLETIONERRORPAGE_H
#define WIZ_COMPLETIONERRORPAGE_H

#include <wx/wizard.h>

class wxStaticText;
class wxStaticBox;
class wxStaticBoxSizer;
class CBOINCBaseWizard;

class CCompletionErrorPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS(CCompletionErrorPage)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCompletionErrorPage();

    CCompletionErrorPage(CBOINCBaseWizard* parent);

    /// Creation
    bool Create(CBOINCBaseWizard* parent);

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONERRORPAGE
    void OnPageChanged(wxWizardEvent& event);

    /// wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONERRORPAGE
    void OnCancel(wxWizardEvent& event);

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource(const wxString& name);

    /// Retrieves icon resources
    wxIcon GetIconResource(const wxString& name);

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Get the current error message.
    wxString GetErrorMessage() const;

    /// Set the current error message.
    void SetErrorMessage(const wxString& msg);

private:
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDirectionsStaticCtrl;
    wxStaticBox* m_pServerMessagesDescriptionCtrl;
    wxStaticBoxSizer* m_pServerMessagesStaticBoxSizerCtrl;
    wxStaticText* m_pServerMessagesCtrl;
};

#endif // WIZ_COMPLETIONERRORPAGE_H
