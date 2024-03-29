# This file is part of Synecdoche.
# http://synecdoche.googlecode.com/
# Copyright (C) 2009 Nicolas Alvarez
#
# Synecdoche is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Synecdoche is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

FIND_PACKAGE(wxWidgets COMPONENTS base core adv net)

IF(wxWidgets_FOUND)
INCLUDE(${wxWidgets_USE_FILE})

# .cpp files in common are including headers in the same directory
# including the 'common/' prefix, so we need to add the clientgui
# root dir to the search path
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

IF(WIN32)
    SET(PLATFORM_GUI_SOURCES msw/taskbarex.cpp)
ELSEIF(APPLE)
    SET(PLATFORM_GUI_SOURCES
        mac/browser_safari.mm
        mac/Mac_GUI.cpp
        mac/MacSysMenu.cpp
        mac/SystemMenu.m
    )
ENDIF(WIN32)

ADD_EXECUTABLE(synecmgr WIN32
# {{{ source list
    AccountInfoPage.cpp
    AccountManagerInfoPage.cpp
    AccountManagerProcessingPage.cpp
    AccountManagerPropertiesPage.cpp
    AdvancedFrame.cpp
    AlreadyExistsPage.cpp
    BOINCBaseFrame.cpp
    BOINCBaseView.cpp
    BOINCBaseWizard.cpp
    BOINCClientManager.cpp
    BOINCDialupManager.cpp
    BOINCGUIApp.cpp
    BOINCListCtrl.cpp
    BOINCTaskBar.cpp
    BOINCTaskCtrl.cpp
    browser.C
    BuildLayout.cpp
    CompletionErrorPage.cpp
    CompletionPage.cpp
    DlgAbout.cpp
    DlgAdvPreferencesBase.cpp
    DlgAdvPreferences.cpp
    DlgGenericMessage.cpp
    DlgMsgFilter.cpp
    DlgOptions.cpp
    DlgSelectComputer.cpp
    hyperlink.cpp
    LogBOINC.cpp
    MainDocument.cpp
    MsgFilterData.cpp
    NoInternetConnectionPage.cpp
    NotDetectedPage.cpp
    NotFoundPage.cpp
    ProjectInfoPage.cpp
    ProjectListCtrl.cpp
    ProjectProcessingPage.cpp
    ProjectPropertiesPage.cpp
    ProxyInfoPage.cpp
    ProxyPage.cpp
    sg_BoincSimpleGUI.cpp
    sg_ClientStateIndicator.cpp
    sg_CustomControls.cpp
    sg_DlgMessages.cpp
    sg_DlgPreferences.cpp
    sg_ImageButton.cpp
    sg_ImageLoader.cpp
    sg_ProgressBar.cpp
    sg_ProjectsComponent.cpp
    sg_SGUIListControl.cpp
    sg_StatImageLoader.cpp
    sg_ViewTabPage.cpp
    SkinManager.cpp
    stdwx.cpp
    TaskViewBase.cpp
    UnavailablePage.cpp
    UiFormatString.cpp
    ValidateEmailAddress.cpp
    ValidateURL.cpp
    ViewMessages.cpp
    ViewProjects.cpp
    ViewResources.cpp
    ViewStatistics.cpp
    ViewTransfers.cpp
    ViewWork.cpp
    WelcomePage.cpp
    WizardAccountManager.cpp
    WizardAttachProject.cpp

    common/wxPieCtrl.cpp
    common/wxFlatNotebook.cpp

    ${PLATFORM_GUI_SOURCES}
    ${CMAKE_BINARY_DIR}/svnversion.cpp
# }}}
)
TARGET_LINK_LIBRARIES(synecmgr boinc)
TARGET_LINK_LIBRARIES(synecmgr ${wxWidgets_LIBRARIES})

IF(WIN32)
    TARGET_LINK_LIBRARIES(synecmgr clientlib)
    TARGET_LINK_LIBRARIES(synecmgr wininet.lib)
ENDIF(WIN32)
IF(APPLE)
    FIND_LIBRARY(SECURITY_LIBRARY Security)
    TARGET_LINK_LIBRARIES(synecmgr ${SECURITY_LIBRARY})
ENDIF(APPLE)

install(TARGETS synecmgr RUNTIME DESTINATION bin)

add_subdirectory(tests)

ENDIF(wxWidgets_FOUND)
# vim: set fdm=marker:
