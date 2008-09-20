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
#ifndef WIZ_COMPLETIONPAGE_H
#define WIZ_COMPLETIONPAGE_H

#include <wx/wizard.h>

class CBOINCBaseWizard;

class CCompletionPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS(CCompletionPage)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCompletionPage();
    CCompletionPage(CBOINCBaseWizard* parent);

    /// Creation
    bool Create(CBOINCBaseWizard* parent);

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONPAGE
    void OnPageChanged(wxWizardEvent& event);

    /// wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONPAGE
    void OnCancel(wxWizardEvent& event);

    /// wxEVT_WIZARD_FINISHED event handler for ID_COMPLETIONPAGE
    void OnFinished(wxWizardEvent& event);

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

private:
    wxStaticText* m_pCompletionTitle;
    wxStaticText* m_pCompletionWelcome;
    wxStaticText* m_pCompletionBrandedMessage;
    wxStaticText* m_pCompletionMessage;
};

#endif // WIZ_COMPLETIONPAGE_H
