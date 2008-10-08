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

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "PrefTreeBook.h"
#include "PrefNodeBase.h"
#include "PrefNodeGeneral.h"
#include "PrefNodeProcessor.h"
#include "Events.h"

DEFINE_EVENT_TYPE(PREF_EVT_HELP_CMD)

IMPLEMENT_DYNAMIC_CLASS(PrefTreeBook, wxPanel)

BEGIN_EVENT_TABLE(PrefTreeBook, wxPanel)
    EVT_TREE_SEL_CHANGING(wxID_ANY, PrefTreeBook::OnTreeSelectionChanging)
    PREF_EVT_HELP(wxID_ANY, PrefTreeBook::OnHelp)
END_EVENT_TABLE()


PrefTreeBook::PrefTreeBook(wxWindow* parent) : wxPanel(parent) {

    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);

    pDoc->rpc.get_global_prefs_working_struct(m_preferences);
    bool haveOverride = (pDoc->rpc.get_global_prefs_override_struct(m_override) == 0);

    m_helpSource = 0;

    // Treeview preferences - ratio 1:2
    wxBoxSizer* contentRow = new wxBoxSizer(wxHORIZONTAL);

    // Content placeholder
    m_content = new wxPanel(this);

    wxPanel* helpPanel = new wxPanel(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxBORDER_THEME);
    helpPanel->SetAutoLayout(true);
    helpPanel->SetMinSize(wxSize(-1, 100));

    helpPanel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
    helpPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));

    m_helpTitleCtrl = new wxStaticText(helpPanel, wxID_ANY, wxEmptyString);
    wxFont font = m_helpTitleCtrl->GetFont();
    font.SetWeight(wxBOLD);
    m_helpTitleCtrl->SetFont(font);

    m_helpTextCtrl = new wxStaticText(helpPanel, wxID_ANY, wxEmptyString);

    m_helpDefaultCtrl = new wxStaticText(helpPanel, wxID_ANY, wxEmptyString);

    wxBoxSizer* helpBox = new wxBoxSizer(wxVERTICAL);

    helpBox->Add(m_helpTitleCtrl, 0, wxALL | wxEXPAND, 2);
    helpBox->Add(m_helpTextCtrl, 0, wxALL | wxEXPAND, 2);
    helpBox->Add(m_helpDefaultCtrl, 0, wxALL | wxEXPAND, 2);

    helpPanel->SetSizer(helpBox);

    wxBoxSizer* contentBox = new wxBoxSizer(wxVERTICAL);

    contentBox->Add(m_content, 4, wxEXPAND);
    contentBox->Add(helpPanel, 0, wxEXPAND | wxTOP, 8 );

    m_tree = new wxTreeCtrl(
        this,
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT
    );
    wxTreeItemId root = m_tree->AddRoot(_("Preferences"));

    wxTreeItemId globalRoot = m_tree->AppendItem(
        root, _("Global Preferences"), -1, -1, new PrefNodeItemData(Presets, &m_preferences));
    BuildGlobalNode(globalRoot, &m_preferences);

    if (haveOverride) {
        wxTreeItemId overrideRoot = m_tree->AppendItem(
            root, _("Override Preferences"), -1, -1, new PrefNodeItemData(Presets, &m_override));
        BuildGlobalNode(overrideRoot, &m_override);
    }

    wxTreeItemId localRoot = m_tree->AppendItem(root, _("Local Preferences"), -1, -1, new PrefNodeItemData(Presets, 0));
    m_tree->AppendItem(localRoot, _("Manager"), -1, -1, new PrefNodeItemData(Presets, 0));
    m_tree->AppendItem(localRoot, _("Connection"), -1, -1, new PrefNodeItemData(Presets, 0));
    m_tree->AppendItem(localRoot, _("Proxy"), -1, -1, new PrefNodeItemData(Presets, 0));

    contentRow->Add(m_tree, 1, wxALL | wxEXPAND, 4);
    contentRow->Add(contentBox, 2, wxALL | wxEXPAND, 4);

    SetSizer(contentRow);

    RestoreState();

}


PrefTreeBook::~PrefTreeBook() {
    SaveState();
}


void PrefTreeBook::BuildGlobalNode(const wxTreeItemId& parent, GLOBAL_PREFS* prefs) {

    m_tree->AppendItem(parent, _("Presets"), -1, -1,
        new PrefNodeItemData(Presets, prefs));

    m_tree->AppendItem(parent, _("General Options"), -1, -1,
        new PrefNodeItemData(General, prefs));

    wxTreeItemId proc = m_tree->AppendItem(parent, _("Processor Usage"), -1, -1,
        new PrefNodeItemData(Processor, prefs));
    m_tree->AppendItem(proc, _("Custom Times"), -1, -1,
        new PrefNodeItemData(ProcessorTimes, prefs));

    wxTreeItemId net = m_tree->AppendItem(parent, _("Network Usage"), -1, -1,
        new PrefNodeItemData(Network, prefs));
    m_tree->AppendItem(net, _("Custom Times"), -1, -1,
        new PrefNodeItemData(NetworkTimes, prefs));

    m_tree->AppendItem(parent, _("Memory Usage"), -1, -1,
        new PrefNodeItemData(Memory, prefs));

    m_tree->AppendItem(parent, _("Disk Usage"), -1, -1,
        new PrefNodeItemData(Disk, prefs));
}


void PrefTreeBook::OnTreeSelectionChanging(wxTreeEvent& event) {

    if (! Validate()) {
        event.Veto();
    }
    else {
        TransferDataFromWindow();

        // Create new page
        wxTreeItemId id = event.GetItem();
        if (id.IsOk()) {
            Freeze();
            PrefNodeItemData* nodeType = (PrefNodeItemData*) m_tree->GetItemData(id);
            PrefNodeBase* page = PrefNodeBase::Create(nodeType->GetNodeType(), this, nodeType->GetPreferences());

            if (page && m_content) {
                // Swap pages
                wxSizer* s = m_content->GetContainingSizer();
                s->Replace(m_content, page);
                m_content->Destroy();
                TransferDataToWindow();
                s->Layout();
                page->Layout();

                m_content = page;
            }
            Thaw();
        }

        PrefHelpEvent e(PREF_EVT_HELP_CMD, GetId());
        e.SetEventObject(0);
        GetEventHandler()->ProcessEvent(e);
    }
}


// Use a stack for help text, to allow mouse movement to override help derived from focus.
void PrefTreeBook::OnHelp(PrefHelpEvent& event) {

    wxObject* eo = event.GetEventObject();
    wxWindow* source = 0;

    if (eo) {
        wxASSERT(eo->IsKindOf(CLASSINFO(wxWindow)));
        source = wxDynamicCast(eo, wxWindow);
    }

    if (source) {
        if (m_helpSource != source) {

            m_helpTitleCtrl->SetLabel(event.GetTitle());
            m_helpDefaultCtrl->SetLabel(_("Default: ") + event.GetDefault());

            m_helpTextCtrl->SetLabel(source->GetHelpText());
            m_helpTextCtrl->GetParent()->Layout();
            m_helpTextCtrl->Wrap(m_helpTextCtrl->GetSize().GetWidth());
            m_helpTextCtrl->GetParent()->Layout();

            m_helpSource = source;
        }
    } else {
        m_helpTitleCtrl->SetLabel(wxEmptyString);
        m_helpTextCtrl->SetLabel(wxEmptyString);
        m_helpDefaultCtrl->SetLabel(wxEmptyString);
    }
}


bool PrefTreeBook::SaveState() {

    wxString        strBaseConfigLocation = wxString(wxT("/PrefTreeBook/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(false);

    wxASSERT(pConfig);
    if (!pConfig) return false;

    PrefNodeItemData* n = (PrefNodeItemData*) m_tree->GetItemData(m_tree->GetSelection());

    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("CurrentNode"), n->GetNodeType());

    return true;
}


bool PrefTreeBook::RestoreState() {

    wxString        strBaseConfigLocation = wxString(wxT("/PrefTreeBook/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(false);
    int             node;

    wxASSERT(pConfig);
    if (!pConfig) return false;

    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read(wxT("CurrentNode"), &node, General);
    
    PrefNodeType selNode = (PrefNodeType) node;

    wxTreeItemId root = m_tree->GetRootItem();
    wxTreeItemId result;

    if (Find(root, result, selNode)) {
        m_tree->SelectItem(result);
    }

    m_tree->SetFocus();

    return true;
}


// Recursively search for a node of the specified type.
bool PrefTreeBook::Find(const wxTreeItemId& root, wxTreeItemId& result, PrefNodeType nodeType) {

    wxTreeItemIdValue cookie;
    wxTreeItemId child = m_tree->GetFirstChild(root, cookie);
    while (child.IsOk()){

        PrefNodeItemData* n = (PrefNodeItemData*) m_tree->GetItemData(child);

        if (n->GetNodeType() == nodeType) {
            result = child;
            return true;
        }
        // Depth first traversal
        if (Find(child, result, nodeType)) {
            return true;
        }
        child = m_tree->GetNextChild(root, cookie);
    }
    return false;
}

void PrefTreeBook::SavePreferences() {

    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);

    pDoc->rpc.set_global_prefs_override_struct(m_preferences);
    pDoc->rpc.read_global_prefs_override();

}
