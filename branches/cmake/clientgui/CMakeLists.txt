# This file is part of Synecdoche.
# http://synecdoche.googlecode.com/
# Copyright (C) 2008 Nicolas Alvarez
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


FIND_PACKAGE(wxWidgets)

INCLUDE_DIRECTORIES(${wxWidgets_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

ADD_DEFINITIONS(${wxWidgets_DEFINITIONS})
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${wxWidgets_CXX_FLAGS}")

ADD_EXECUTABLE(synecmgr
AccountInfoPage.cpp
AccountKeyPage.cpp
AccountManagerInfoPage.cpp
AccountManagerProcessingPage.cpp
AccountManagerPropertiesPage.cpp
AdvancedFrame.cpp
AlreadyAttachedPage.cpp
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
CompletionErrorPage.cpp
CompletionPage.cpp
common/wxPieCtrl.cpp
common/wxFlatNotebook.cpp
DlgAbout.cpp
DlgAdvPreferences.cpp
DlgAdvPreferencesBase.cpp
DlgGenericMessage.cpp
DlgOptions.cpp
DlgSelectComputer.cpp
hyperlink.cpp
LogBOINC.cpp
MainDocument.cpp
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
ValidateAccountKey.cpp
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
)
TARGET_LINK_LIBRARIES(synecmgr boinc)
TARGET_LINK_LIBRARIES(synecmgr ${wxWidgets_LIBRARIES})

