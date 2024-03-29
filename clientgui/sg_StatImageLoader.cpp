// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
// Copyright (C) 2009 University of California
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

#include "sg_StatImageLoader.h" 

#include "stdwx.h"

#include "app_ipc.h"
#include "BOINCBaseFrame.h"
#include "BOINCGUIApp.h"
#include "hyperlink.h"
#include "MainDocument.h"
#include "sg_ProjectsComponent.h" 
#include "SkinManager.h"

enum{
    WEBSITE_URL_MENU_ID = 34500,
    WEBSITE_URL_MENU_ID_REMOVE_PROJECT = 34550,
    WEBSITE_URL_MENU_ID_HOMEPAGE = 34551,
};


BEGIN_EVENT_TABLE(StatImageLoader, wxWindow) 
    EVT_PAINT(StatImageLoader::OnPaint) 
    EVT_LEFT_DOWN(StatImageLoader::PopUpMenu)
    EVT_MENU(WEBSITE_URL_MENU_ID,StatImageLoader::OnMenuLinkClicked)
    EVT_MENU(WEBSITE_URL_MENU_ID_REMOVE_PROJECT,StatImageLoader::OnMenuLinkClicked)
END_EVENT_TABLE() 

StatImageLoader::StatImageLoader(wxWindow* parent, std::string url) : 
    wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(40,40), wxNO_BORDER) 
{
    m_prjUrl = url;
    project_files_downloaded_time = 1;
    project_last_rpc_time = 1;
    BuildUserStatToolTip();
    statPopUpMenu = new wxMenu();
    AddMenuItems();
}


StatImageLoader::~StatImageLoader() {
    delete statPopUpMenu;
}


#ifdef __WXMAC__
void StatImageLoader::PopUpMenu(wxMouseEvent& event) 
{ 
    // pop up menu: move menu down a bit so tooltip does not obscure it
    PopupMenu(statPopUpMenu, event.m_x-10, event.m_y+40);
}
#else
void StatImageLoader::PopUpMenu(wxMouseEvent& WXUNUSED(event)) 
{ 
    // pop up menu
    PopupMenu(statPopUpMenu);
}
#endif

void StatImageLoader::RebuildMenu() {
    for (size_t i = statPopUpMenu->GetMenuItemCount(); i > 0; --i){
        wxMenuItem* item = statPopUpMenu->FindItemByPosition(i - 1);
        statPopUpMenu->Delete(item);
    }
    AddMenuItems();
}


void StatImageLoader::BuildUserStatToolTip() {
    wxString strBuffer = wxEmptyString;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    const PROJECT* project = pDoc->state.lookup_project(m_prjUrl);

    strBuffer.Printf(
        _("%s. Work done by %s: %0.2f"),
        wxString(project->project_name.c_str(), wxConvUTF8).c_str(),
        wxString(project->user_name.c_str(), wxConvUTF8).c_str(),
        project->user_total_credit
    );

    SetToolTip(new wxToolTip(strBuffer));
}


void StatImageLoader::AddMenuItems() 
{ 
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    const PROJECT* project = pDoc->state.lookup_project(m_prjUrl);
    urlCount = project->gui_urls.size();

    // Add the home page link
    wxMenuItem *urlItem = new wxMenuItem(statPopUpMenu, WEBSITE_URL_MENU_ID_HOMEPAGE,wxString(project->project_name.c_str(), wxConvUTF8));
    Connect( WEBSITE_URL_MENU_ID_HOMEPAGE,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(StatImageLoader::OnMenuLinkClicked) );
    statPopUpMenu->Append(urlItem);


    // Add any GUI urls
    for(unsigned int i = 0; i < urlCount; i++){
        urlItem = new wxMenuItem(statPopUpMenu, WEBSITE_URL_MENU_ID + i, wxGetTranslation(wxString(project->gui_urls[i].name.c_str(), wxConvUTF8)));
        Connect( WEBSITE_URL_MENU_ID + i,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(StatImageLoader::OnMenuLinkClicked) );
 
        statPopUpMenu->Append(urlItem);
    }
    
    //  Add the 'remove project' option
    if (!project->attached_via_acct_mgr) {
        statPopUpMenu->AppendSeparator();
        wxMenuItemList menuList = statPopUpMenu->GetMenuItems();

        urlItem = new wxMenuItem(statPopUpMenu, WEBSITE_URL_MENU_ID_REMOVE_PROJECT, _("Remove Project"));

        Connect( WEBSITE_URL_MENU_ID_REMOVE_PROJECT,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(StatImageLoader::OnMenuLinkClicked) );
        statPopUpMenu->Append(urlItem);
    }
}


void StatImageLoader::OnMenuLinkClicked(wxCommandEvent& event) 
{ 
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    int menuIDevt =  event.GetId();

    if(menuIDevt == WEBSITE_URL_MENU_ID_REMOVE_PROJECT){
        //call detach project function
        OnProjectDetach();
    } else if (menuIDevt == WEBSITE_URL_MENU_ID_HOMEPAGE ) {
        HyperLink::ExecuteLink(wxString(m_prjUrl.c_str(),wxConvUTF8));
    } else{
        int menuId = menuIDevt - WEBSITE_URL_MENU_ID;
        PROJECT* project = pDoc->state.lookup_project(m_prjUrl);
        project->gui_urls[menuId].name.c_str();

        HyperLink::ExecuteLink(wxString(project->gui_urls[menuId].url.c_str(),wxConvUTF8));
    }
} 


void StatImageLoader::OnProjectDetach() {
    wxInt32  iAnswer        = 0; 
    std::string strProjectName;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    CProjectsComponent* pComp      = wxDynamicCast(GetParent(), CProjectsComponent);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pComp);

    if (!pDoc->IsUserAuthorized())
        return;

    size_t indexOfProj = 0;
    size_t prjCount = pDoc->GetProjectCount();
    for(size_t m = 0; m < prjCount; m++){
        const PROJECT* project = pDoc->project(m);
        project->get_name(strProjectName);
        if(project->master_url == m_prjUrl){
            indexOfProj = m;
            break;
        }
    }
    strMessage.Printf(_("Are you sure you want to detach from project '%s'?"), strProjectName.c_str());

    iAnswer = ::wxMessageBox(strMessage, _("Detach from Project"), wxYES_NO | wxICON_QUESTION, this);

    if (wxYES == iAnswer) {
        pDoc->ProjectDetach(indexOfProj);
        pComp->UpdateProjectArray();
    }
}


void StatImageLoader::LoadStatIcon(wxBitmap& image) {
    int width = image.GetWidth(); 
    int height = image.GetHeight(); 
    Bitmap = wxBitmap(image); 
    SetSize(width, height); 
}

/// Retrieve the location of the project icon.
///
/// \return A string containing the path to the project icon.
std::string StatImageLoader::GetProjectIconLoc() {
    const PROJECT* project = wxGetApp().GetDocument()->state.lookup_project(m_prjUrl);
    std::string urlDirectory = url_to_project_dir(project->master_url);
    return urlDirectory + std::string("/stat_icon");
}


void StatImageLoader::LoadImage() {
    std::string dirProjectGraphic;

    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    char defaultIcnPath[256];
    if(boinc_resolve_filename(GetProjectIconLoc().c_str(), defaultIcnPath, sizeof(defaultIcnPath)) == 0){
        wxBitmap* btmpStatIcn = new wxBitmap();
        if ( btmpStatIcn->LoadFile(wxString(defaultIcnPath,wxConvUTF8), wxBITMAP_TYPE_ANY) ) {
            LoadStatIcon(*btmpStatIcn);
        } else {
            LoadStatIcon(*pSkinSimple->GetProjectImage()->GetBitmap());
        }
        delete btmpStatIcn;
    }else{
        LoadStatIcon(*pSkinSimple->GetProjectImage()->GetBitmap());
    }
}


void StatImageLoader::ReloadProjectSpecificIcon() {
    char defaultIcnPath[256];
    // Only update if it is project specific is found
    if(boinc_resolve_filename(GetProjectIconLoc().c_str(), defaultIcnPath, sizeof(defaultIcnPath)) == 0){
        wxBitmap* btmpStatIcn = new wxBitmap();
        if ( btmpStatIcn->LoadFile(wxString(defaultIcnPath,wxConvUTF8), wxBITMAP_TYPE_ANY) ) {
            LoadStatIcon(*btmpStatIcn);
            RebuildMenu();
            Refresh();
            Update();
        }
        delete btmpStatIcn;
    }
}


void StatImageLoader::UpdateInterface() {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    const PROJECT* project = pDoc->state.lookup_project(m_prjUrl);

    // Check to see if we need to reload the stat icon
    if ( project != NULL && project->project_files_downloaded_time > project_files_downloaded_time ) {
        ReloadProjectSpecificIcon();
        project_files_downloaded_time = project->project_files_downloaded_time;
    }

    // Check to see if we need to rebuild the hoover and menu
    if ( project != NULL && project->last_rpc_time > project_last_rpc_time ) {
        RebuildMenu();
        BuildUserStatToolTip();
        project_last_rpc_time = project->last_rpc_time;
    }
}


void StatImageLoader::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{
    wxPaintDC dc(this);
    dc.SetBackgroundMode(wxTRANSPARENT);
    if(Bitmap.Ok()) {
        dc.DrawBitmap(Bitmap, 0, 0);
    }
}
