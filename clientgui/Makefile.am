## -*- mode: makefile; tab-width: 4 -*-
## $Id: Makefile.am 15375 2008-06-08 22:18:52Z davea $

include $(top_srcdir)/Makefile.incl

#LIBS += @CLIENTLIBS@


if OS_DARWIN
   LIBS += -framework IOKit -framework Foundation -framework ScreenSaver -framework Cocoa -framework Security
   mac_sources = mac/SystemMenu.m \
                 mac/MacSysMenu.cpp \
                 mac/Mac_GUI.cpp\
                 mac/browser_safari.mm

    mac_headers = mac/MacSysMenu.h
else
   mac_headers =
   mac_sources = 
endif

bin_PROGRAMS = synecmgr

# keep the list of sources alphabetically sorted

gui_headers = \
    AccountInfoPage.h \
    AccountManagerInfoPage.h \
    AccountManagerProcessingPage.h \
    AccountManagerPropertiesPage.h \
    AdvancedFrame.h \
    AlreadyExistsPage.h \
    BOINCBaseFrame.h \
    BOINCBaseView.h \
    BOINCBaseWizard.h \
    BOINCClientManager.h \
    BOINCDialupManager.h \
    BOINCGUIApp.h \
    BOINCListCtrl.h \
    BOINCTaskBar.h \
    BOINCTaskCtrl.h \
    BOINCWizards.h \
    browser.h \
    common/wxFNBDropTarget.h \
    common/wxFlatNotebook.h \
    common/wxFlatNotebookImages.h \
    common/wxPieCtrl.h \
    CompletionErrorPage.h \
    CompletionPage.h \
    DlgAbout.h \
    DlgAdvPreferencesBase.h \
    DlgAdvPreferences.h \
    DlgGenericMessage.h \
    DlgOptions.h \
    DlgSelectComputer.h \
    Events.h \
    hyperlink.h \
    LogBOINC.h \
    MainDocument.h \
    NoInternetConnectionPage.h \
    NotDetectedPage.h \
    NotFoundPage.h \
    ProjectInfoPage.h \
    ProjectListCtrl.h \
    ProjectProcessingPage.h \
    ProjectPropertiesPage.h \
    ProxyInfoPage.h \
    ProxyPage.h \
    sg_BoincSimpleGUI.h \
    sg_ClientStateIndicator.h \
    sg_CustomControls.h \
    sg_DlgMessages.h \
    sg_DlgPreferences.h \
    sg_ImageButton.h \
    sg_ImageLoader.h \
    sg_ProgressBar.h \
    sg_ProjectsComponent.h \
    sg_SGUIListControl.h \
    sg_StatImageLoader.h \
    sg_ViewTabPage.h \
    stdwx.h \
    SkinManager.h \
    TaskViewBase.h \
    UnavailablePage.h \
    ValidateEmailAddress.h \
    ValidateURL.h \
    ViewMessages.h \
    ViewProjects.h \
    ViewResources.h \
    ViewStatistics.h \
    ViewTransfers.h \
    ViewWork.h \
    WelcomePage.h \
    WizardAccountManager.h \
    WizardAttachProject.h

gui_xpms = \
    res/disconnected.xpm \
    res/externalweblink.xpm \
    res/mess.xpm \
    res/proj.xpm \
    res/result.xpm \
    res/snooze.xpm \
    res/sortascending.xpm \
    res/sortdescending.xpm \
    res/stats.xpm \
    res/synecdoche16x16x8.xpm \
    res/synecdoche32x32x8.xpm \
    res/synecdoche_logo.xpm \
    res/usage.xpm \
    res/warning.xpm \
    res/wizard_bitmap.xpm \
    res/wizprogress01.xpm \
    res/wizprogress02.xpm \
    res/wizprogress03.xpm \
    res/wizprogress04.xpm \
    res/wizprogress05.xpm \
    res/wizprogress06.xpm \
    res/wizprogress07.xpm \
    res/wizprogress08.xpm \
    res/wizprogress09.xpm \
    res/wizprogress10.xpm \
    res/wizprogress11.xpm \
    res/wizprogress12.xpm \
    res/xfer.xpm \
    res/skins/default/graphic/advanced_link_image.xpm \
    res/skins/default/graphic/attach_project_button.xpm \
    res/skins/default/graphic/attach_project_clicked_button.xpm \
    res/skins/default/graphic/background_image.xpm \
    res/skins/default/graphic/cancel_button.xpm \
    res/skins/default/graphic/cancel_clicked_button.xpm \
    res/skins/default/graphic/close_button.xpm \
    res/skins/default/graphic/close_clicked_button.xpm \
    res/skins/default/graphic/connecting_indicator_image.xpm \
    res/skins/default/graphic/copy_all_button.xpm \
    res/skins/default/graphic/copy_all_clicked_button.xpm \
    res/skins/default/graphic/copy_button.xpm \
    res/skins/default/graphic/copy_clicked_button.xpm \
    res/skins/default/graphic/dialog_background_image.xpm \
    res/skins/default/graphic/error_indicator_image.xpm \
    res/skins/default/graphic/help_button.xpm \
    res/skins/default/graphic/help_clicked_button.xpm \
    res/skins/default/graphic/left_arrow_button.xpm \
    res/skins/default/graphic/left_arrow_clicked_button.xpm \
    res/skins/default/graphic/messages_alert_link_image.xpm \
    res/skins/default/graphic/messages_link_image.xpm \
    res/skins/default/graphic/preferences_link_image.xpm \
    res/skins/default/graphic/project_area_background_image.xpm \
    res/skins/default/graphic/project_image.xpm \
    res/skins/default/graphic/resume_link_image.xpm \
    res/skins/default/graphic/right_arrow_button.xpm \
    res/skins/default/graphic/right_arrow_clicked_button.xpm \
    res/skins/default/graphic/save_button.xpm \
    res/skins/default/graphic/save_clicked_button.xpm \
    res/skins/default/graphic/spacer_image.xpm \
    res/skins/default/graphic/state_indicator_background_image.xpm \
    res/skins/default/graphic/suspend_link_image.xpm \
    res/skins/default/graphic/synchronize_button.xpm \
    res/skins/default/graphic/synchronize_clicked_button.xpm \
    res/skins/default/graphic/workunit_active_image.xpm \
    res/skins/default/graphic/workunit_animation_background_image.xpm \
    res/skins/default/graphic/workunit_animation_image.xpm \
    res/skins/default/graphic/workunit_area_background_image.xpm \
    res/skins/default/graphic/workunit_gauge_background_image.xpm \
    res/skins/default/graphic/workunit_gauge_progress_indicator_image.xpm \
    res/skins/default/graphic/workunit_suspended_image.xpm \
    res/skins/default/graphic/workunit_tab_area_background_image.xpm

synecmgr_SOURCES = \
    AccountInfoPage.cpp \
    AccountManagerInfoPage.cpp \
    AccountManagerProcessingPage.cpp \
    AccountManagerPropertiesPage.cpp \
    AdvancedFrame.cpp \
    AlreadyExistsPage.cpp \
    BOINCBaseFrame.cpp \
    BOINCBaseView.cpp \
    BOINCBaseWizard.cpp \
    BOINCClientManager.cpp \
    BOINCDialupManager.cpp \
    BOINCGUIApp.cpp \
    BOINCListCtrl.cpp \
    BOINCTaskBar.cpp \
    BOINCTaskCtrl.cpp \
    browser.C \
    CompletionErrorPage.cpp \
    CompletionPage.cpp \
    DlgAbout.cpp \
    DlgAdvPreferences.cpp \
    DlgAdvPreferencesBase.cpp \
    DlgGenericMessage.cpp \
    DlgOptions.cpp \
    DlgSelectComputer.cpp \
    hyperlink.cpp \
    LogBOINC.cpp \
    MainDocument.cpp \
    NoInternetConnectionPage.cpp \
    NotDetectedPage.cpp \
    NotFoundPage.cpp \
    ProjectInfoPage.cpp \
    ProjectListCtrl.cpp \
    ProjectProcessingPage.cpp \
    ProjectPropertiesPage.cpp \
    ProxyInfoPage.cpp \
    ProxyPage.cpp \
    sg_BoincSimpleGUI.cpp \
    sg_ClientStateIndicator.cpp \
    sg_CustomControls.cpp \
    sg_DlgMessages.cpp \
    sg_DlgPreferences.cpp \
    sg_ImageButton.cpp \
    sg_ImageLoader.cpp \
    sg_ProgressBar.cpp \
    sg_ProjectsComponent.cpp \
    sg_SGUIListControl.cpp \
    sg_StatImageLoader.cpp \
    sg_ViewTabPage.cpp \
    SkinManager.cpp \
    stdwx.cpp \
    TaskViewBase.cpp \
    UnavailablePage.cpp \
    ValidateEmailAddress.cpp \
    ValidateURL.cpp \
    ViewMessages.cpp \
    ViewProjects.cpp \
    ViewResources.cpp \
    ViewStatistics.cpp \
    ViewTransfers.cpp \
    ViewWork.cpp \
    WelcomePage.cpp \
    WizardAccountManager.cpp \
    WizardAttachProject.cpp \
    common/wxFlatNotebook.cpp \
    common/wxPieCtrl.cpp \
    $(gui_headers) \
    $(gui_xpms) \
    $(mac_sources) $(mac_headers)

nodist_synecmgr_SOURCES = ../svnversion.cpp

EXTRA_DIST = \
    msw

CXXCOMPILE = $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
        $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_CXXFLAGS) $(WX_CXXFLAGS) $(CLIENTGUIFLAGS)

if PRECOMPILE_HEADERS
BUILT_SOURCES = stdwx.h.gch
endif

synecmgr_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) $(CLIENTGUIFLAGS)
synecmgr_CXXFLAGS = $(AM_CXXFLAGS) $(WX_CXXFLAGS) $(CLIENTGUIFLAGS)
synecmgr_LDADD = $(LIBBOINC) $(CLIENTGUILIBS)

stdwx.h.gch: stdwx.h
	-rm -f $@
	$(CXXCOMPILE) -o $@ -xc++-header $(srcdir)/stdwx.h

all-local: synecmgr

win_config.h: $(top_srcdir)/config.h
	grep '#define.*BOINC.*VERSION' $^ > $@

version: win_config.h

CLEANFILES=stdwx.h.gch

.PHONY: version
