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

#ifndef CLIENTSTATEINDICATOR_H
#define CLIENTSTATEINDICATOR_H

#include <vector>

#include <wx/panel.h>

class wxPaintEvent;
class wxTimer;
class wxTimerEvent;

class CSimplePanel;
class ImageLoader;

class ClientStateIndicator : public wxPanel {
    DECLARE_DYNAMIC_CLASS(ClientStateIndicator)
public:
    int connIndicatorWidth;
    int connIndicatorHeight;
    int numOfIndic;
    int rightPosition;
    int topPosition;
    int indexIndVis;
    int numOfProgressInd;
    ImageLoader *i_indBg;
    ImageLoader *i_errorInd;
    std::vector<ImageLoader*> m_connIndV;
    wxTimer *m_connRenderTimer;
    wxString stateMessage;
    int clientState;
    /// Constructors
    ClientStateIndicator();
    ClientStateIndicator(CSimplePanel* parent, wxPoint coord);
    ~ClientStateIndicator();
    void CreateComponent();
    void ReskinInterface();
    void DeletePreviousState();
    void DisplayState();

private:

    void SetActionState(wxString message);
    void SetNoActionState(wxString message);
    void SetPausedState(wxString message);
    bool DownloadingResults();
    bool Suspended();
    bool ProjectUpdateScheduled();
    time_t error_time;

    void OnEraseBackground(wxEraseEvent& event);
    void OnPaint(wxPaintEvent& event);
    void RunConnectionAnimation(wxTimerEvent& event );

    DECLARE_EVENT_TABLE()
};

#define CLIENT_STATE_NONE 0
#define CLIENT_STATE_ACTION 1
#define CLIENT_STATE_PAUSED 2
#define CLIENT_STATE_ERROR 3

#endif

