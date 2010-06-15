// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2010 Nicolas Alvarez
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

#include <UnitTest++.h>

#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>

#include "clientgui/BuildLayout.h"

class MainWindow: public wxFrame {
public:
    MainWindow(): wxFrame(NULL, wxID_ANY, wxT("test")) {}
};

/// This test simply checks of MoveBeforeInTabOrder works as I expect it to work.
/// It doesn't test any of our code.
TEST(WxTabOrder)
{
    MainWindow window;
    wxBoxSizer* dialogSizer = new wxBoxSizer( wxHORIZONTAL );

    wxCheckBox* chkFoo = new wxCheckBox(&window, wxID_ANY, wxT("Foo"));
    wxCheckBox* chkBar = new wxCheckBox(&window, wxID_ANY, wxT("Bar"));
    dialogSizer->Add(chkFoo);
    dialogSizer->Add(chkBar);
    chkBar->MoveBeforeInTabOrder(chkFoo);
    window.SetSizer(dialogSizer);

    // controls get order changed in the list of children of the parent window
    // according to the modified tab order
    const wxWindowList& windowList = window.GetChildren();
    CHECK_EQUAL(2, windowList.size());
    CHECK_EQUAL(chkBar, windowList[0]);
    CHECK_EQUAL(chkFoo, windowList[1]);

    // but they keep their visual order in the sizer
    const wxSizerItemList& sizerList = dialogSizer->GetChildren();
    CHECK_EQUAL(2, sizerList.size());
    CHECK_EQUAL(chkFoo, sizerList[0]->GetWindow());
    CHECK_EQUAL(chkBar, sizerList[1]->GetWindow());
}
SUITE(TestBuildLayout)
{
    TEST(SimpleCase) {
        MainWindow window;
        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        wxTextCtrl* textBox = new wxTextCtrl(&window, wxID_ANY);

        buildLayout(&window, sizer, wxT("Use up to %1 threads"), textBox);

        const wxSizerItemList& sizerList = sizer->GetChildren();
        CHECK_EQUAL(3, sizerList.size());

        // left label
        wxStaticText* label1 = wxDynamicCast(sizerList[0]->GetWindow(), wxStaticText);
        CHECK(label1);
        CHECK_EQUAL("Use up to", (const char*)label1->GetLabel().ToAscii());

        // text field

        // note: this cast is necessary for the assertion failure message to be correct;
        // wxTextCtrl derives from streambuf(?!) and streams have an overload for operator<<(streambuf*),
        // so wxTextCtrl* x; stream<<x; has unexpected behavior.
        CHECK_EQUAL(static_cast<wxWindow*>(textBox), sizerList[1]->GetWindow());

        // right label
        wxStaticText* label2 = wxDynamicCast(sizerList[2]->GetWindow(), wxStaticText);
        CHECK(label2);
        CHECK_EQUAL("threads", (const char*)label2->GetLabel().ToAscii());
    }
    void checkTwoControls(MainWindow& window, wxSizer* sizer,
                          const char* labelLeftText, const wxTextCtrl* ctrl1,
                          const char* labelMidText, const wxTextCtrl* ctrl2,
                          const char* labelRightText)
    {
        const wxSizerItemList& sizerList = sizer->GetChildren();
        CHECK_EQUAL(5, sizerList.size());

        // left label
        wxStaticText* label1 = wxDynamicCast(sizerList[0]->GetWindow(), wxStaticText);
        CHECK(label1);
        CHECK_EQUAL(labelLeftText, (const char*)label1->GetLabel().ToAscii());

        // first field
        CHECK_EQUAL(static_cast<const wxWindow*>(ctrl1), sizerList[1]->GetWindow());

        // mid label
        wxStaticText* label2 = wxDynamicCast(sizerList[2]->GetWindow(), wxStaticText);
        CHECK(label2);
        CHECK_EQUAL(labelMidText, (const char*)label2->GetLabel().ToAscii());
        
        // second field
        CHECK_EQUAL(static_cast<const wxWindow*>(ctrl2), sizerList[3]->GetWindow());

        // right label
        wxStaticText* label3 = wxDynamicCast(sizerList[4]->GetWindow(), wxStaticText);
        CHECK(label3);
        CHECK_EQUAL(labelRightText, (const char*)label3->GetLabel().ToAscii());
    }
    TEST(TwoControls) {
        MainWindow window;
        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        wxTextCtrl* textMBs = new wxTextCtrl(&window, wxID_ANY);
        wxTextCtrl* textDays = new wxTextCtrl(&window, wxID_ANY);

        buildLayout(&window, sizer, wxT("At most %1 megabytes every %2 days"), textMBs, textDays);

        checkTwoControls(window, sizer, "At most", textMBs, "megabytes every", textDays, "days");
    }
    TEST(TwoControlsSwapped) {
        MainWindow window;
        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        wxTextCtrl* textMBs = new wxTextCtrl(&window, wxID_ANY);
        wxTextCtrl* textDays = new wxTextCtrl(&window, wxID_ANY);

        buildLayout(&window, sizer, wxT("Every %2 days use at most %1 megabytes"), textMBs, textDays);

        checkTwoControls(window, sizer, "Every", textDays, "days use at most", textMBs, "megabytes");
    }
}
