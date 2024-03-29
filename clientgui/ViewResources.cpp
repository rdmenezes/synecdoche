// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
// Copyright (C) 2008 University of California
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

#include "ViewResources.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "Events.h"
#include "SkinManager.h"
#include <wx/arrimpl.cpp>
#include "res/usage.xpm"

WX_DEFINE_OBJARRAY(wxArrayColour);

IMPLEMENT_DYNAMIC_CLASS(CViewResources, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewResources, CBOINCBaseView)
END_EVENT_TABLE ()


CViewResources::CViewResources()
{}


CViewResources::CViewResources(wxNotebook* pNotebook) :
CBOINCBaseView(pNotebook) {}


CViewResources::~CViewResources() {}


void CViewResources::DemandLoadView() {

    wxASSERT(!m_bViewLoaded);
    m_BOINCwasEmpty=false;

    wxGridSizer* itemGridSizer = new wxGridSizer(2, 0, 3);
    wxASSERT(itemGridSizer);

    // create pie chart ctrl for total disk usage
    m_pieCtrlTotal = new wxPieCtrl(this, ID_LIST_RESOURCEUTILIZATIONVIEWTOTAL, wxDefaultPosition, wxSize(-1,-1));
    wxASSERT(m_pieCtrlTotal);
    // setup the legend
    m_pieCtrlTotal->SetTransparent(true);
    m_pieCtrlTotal->SetHorLegendBorder(10);
    m_pieCtrlTotal->SetLabelFont(*wxSWISS_FONT);
    m_pieCtrlTotal->SetLabelColour(wxColour(0,0,0));
    m_pieCtrlTotal->SetLabel(_("Total disk usage"));

    // create pie chart ctrl for BOINC disk usage
    m_pieCtrlBOINC = new wxPieCtrl(this, ID_LIST_RESOURCEUTILIZATIONVIEW, wxDefaultPosition, wxSize(-1,-1));
    wxASSERT(m_pieCtrlBOINC);
    //setup the legend
    m_pieCtrlBOINC->SetTransparent(true);
    m_pieCtrlBOINC->SetHorLegendBorder(10);
    m_pieCtrlBOINC->SetLabelFont(*wxSWISS_FONT);
    m_pieCtrlBOINC->SetLabelColour(wxColour(0,0,0));
    m_pieCtrlBOINC->SetLabel(_("Disk usage by projects"));
    //init the flexGrid
    itemGridSizer->Add(m_pieCtrlTotal,1,wxGROW|wxALL,1);
    itemGridSizer->Add(m_pieCtrlBOINC,1, wxGROW|wxALL,1);

    SetSizer(itemGridSizer);

    Layout();

    RestoreState();

    UpdateSelection();
}

const wxString& CViewResources::GetViewName() {
    static wxString strViewName(wxT("Disk"));
    return strViewName;
}

const wxString& CViewResources::GetViewDisplayName() {
    static wxString strViewName(_("Disk"));
    return strViewName;
}


const char** CViewResources::GetViewIcon() {
    return usage_xpm;
}

void CViewResources::UpdateSelection() {
    CBOINCBaseView::PreUpdateSelection();
    CBOINCBaseView::PostUpdateSelection();
}


#ifdef __WXMAC__
int CViewResources::GetViewRefreshRate() {
    return 10;
}
#endif


wxInt32 CViewResources::FormatProjectName(const PROJECT* project, wxString& strBuffer) const {
    CMainDocument* doc = wxGetApp().GetDocument();
    std::string project_name;

    wxASSERT(doc);
    wxASSERT(wxDynamicCast(doc, CMainDocument));

    if (project) {
        PROJECT* state_project = doc->state.lookup_project(project->master_url);
        if (state_project) {
            state_project->get_name(project_name);
            strBuffer = HtmlEntityDecode(wxString(project_name.c_str(), wxConvUTF8));
        }
    }

    return 0;
}


bool CViewResources::OnSaveState(wxConfigBase* /*pConfig*/) {
    return true;/*bool bReturnValue = true;

    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);

    if (!m_pTaskPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    return bReturnValue;*/
}

bool CViewResources::OnRestoreState(wxConfigBase* /*pConfig*/) {
    return true;/*wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);

    if (!m_pTaskPane->OnRestoreState(pConfig)) {
        return false;
    }

    return true;*/
}

void CViewResources::OnListRender( wxTimerEvent& WXUNUSED(event) ) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString diskspace;
    static double project_total=0.0;
    unsigned int i;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    //get data for BOINC projects disk usage
    pDoc->CachedDiskUsageUpdate();
    pDoc->CachedStateUpdate();
    bool refreshBOINC=false;
    if (pDoc->disk_usage.projects.size()>0) {
        m_BOINCwasEmpty=false;
        //check for changes worth a refresh
        if(pDoc->disk_usage.projects.size() != m_pieCtrlBOINC->m_Series.size()) {
            refreshBOINC=true;
        } else {
            for (i=0; i<pDoc->disk_usage.projects.size(); i++) {
                wxString oldValue;
                wxString newValue;
                FormatDiskSpace(pDoc->DiskUsageProject(i)->disk_usage, newValue);
                FormatDiskSpace(m_pieCtrlBOINC->m_Series.Item(i).GetValue(), oldValue);
                if(newValue.Cmp(oldValue)!=0) {
                    refreshBOINC=true;
                    break;
                }
            }
        }
        //only refresh when worthy changes
        if(refreshBOINC) {
            m_pieCtrlBOINC->m_Series.Clear();
            project_total = 0;
            for (i=0; i<pDoc->disk_usage.projects.size(); i++) {
                //update data for boinc projects pie chart
                PROJECT* project = pDoc->DiskUsageProject(i);
                wxString projectname;
                FormatProjectName(project, projectname);
                FormatDiskSpace(project->disk_usage, diskspace);
                double usage = project->disk_usage;
                project_total += usage;
                wxPiePart part;
                part.SetLabel(projectname + wxT(": ") + diskspace);
                part.SetValue(usage);
                unsigned char r=128+(rand()&127);
                unsigned char g=128+(rand()&127);
                unsigned char b=128+(rand()&127);
                part.SetColour(wxColour(r, g, b));
                m_pieCtrlBOINC->m_Series.Add(part);
            }
            m_pieCtrlBOINC->Refresh();
        }
    }
    //data for pie chart 2 (total disk usage)
    //
    // good source of color palettes:
    // http://www.siteprocentral.com/cgi-bin/feed/feed.cgi
    //
    bool refreshTotal=false;
    double free = pDoc->disk_usage.d_free;
    double total = pDoc->disk_usage.d_total;
    if(m_pieCtrlTotal->m_Series.size()>0) {
        wxString oldFree;
        wxString newFree;
        FormatDiskSpace(free, newFree);
        FormatDiskSpace(m_pieCtrlTotal->m_Series.Item(0).GetValue(), oldFree);
        if(oldFree.Cmp(newFree)!=0) {
            refreshTotal=true;
        }
    } else {
        refreshTotal=true;
    }
    if(refreshBOINC || refreshTotal) {
        m_pieCtrlTotal->m_Series.Clear();
        wxPiePart part;
        wxString label;

        // used by BOINC
        double boinc_total = project_total + pDoc->disk_usage.d_boinc;
        FormatDiskSpace(boinc_total, diskspace);
        label.Printf(_("used by %s: "), pSkinAdvanced->GetApplicationName().c_str());
        part.SetLabel(label + diskspace);
        part.SetValue(boinc_total);
        part.SetColour(wxColour(0,0,0));
        m_pieCtrlTotal->m_Series.Add(part);

        if (pDoc->disk_usage.d_allowed > 0.0) {
            double avail = pDoc->disk_usage.d_allowed - boinc_total;
            if (avail > 0.0) {
                if (avail > free) {
                    avail = free;
                }
                FormatDiskSpace(avail, diskspace);
                label.Printf(_("free, available to %s: "), pSkinAdvanced->GetApplicationName().c_str());
                part.SetLabel(label + diskspace);
                part.SetValue(avail);
                part.SetColour(wxColour(128, 128, 128));
                m_pieCtrlTotal->m_Series.Add(part);
            } else {
                avail = 0;
            }

            // free disk space
            double not_avail = free - avail;
            if (not_avail > 0.0) {
                FormatDiskSpace(not_avail, diskspace);
                label.Printf(_("free, not available to %s: "), pSkinAdvanced->GetApplicationName().c_str());
                part.SetLabel(label + diskspace);
                part.SetValue(not_avail);
                part.SetColour(wxColour(238, 238, 238));
                m_pieCtrlTotal->m_Series.Add(part);
            }
        } else {
            // If d_allowed is zero we must be talking to a pre-6.3 client.
            // Just show free space in this case.
            FormatDiskSpace(free, diskspace);
            label.Printf(_("free: ") + diskspace);
            part.SetLabel(label);
            part.SetValue(free);
            part.SetColour(wxColour(238, 238, 238));
            m_pieCtrlTotal->m_Series.Add(part);
        }

        // used by others
        double used_by_others = total-boinc_total-free;
        FormatDiskSpace(used_by_others, diskspace);
        part.SetLabel(_("used by other programs: ") + diskspace);
        part.SetValue(used_by_others);
        part.SetColour(wxColour(192,192,192));
        m_pieCtrlTotal->m_Series.Add(part);
        m_pieCtrlTotal->Refresh();
    }
}

wxInt32 CViewResources::FormatDiskSpace(double bytes, wxString& strBuffer) const {
    double         xTera = 1099511627776.0;
    double         xGiga = 1073741824.0;
    double         xMega = 1048576.0;
    double         xKilo = 1024.0;

    if (bytes >= xTera) {
        strBuffer.Printf(wxT("%0.2f TB"), bytes/xTera);
    } else if (bytes >= xGiga) {
        strBuffer.Printf(wxT("%0.2f GB"), bytes/xGiga);
    } else if (bytes >= xMega) {
        strBuffer.Printf(wxT("%0.2f MB"), bytes/xMega);
    } else if (bytes >= xKilo) {
        strBuffer.Printf(wxT("%0.2f KB"), bytes/xKilo);
    } else {
        strBuffer.Printf(wxT("%0.0f bytes"), bytes);
    }

    return 0;
}
