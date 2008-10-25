## -*- mode: makefile; tab-width: 4 -*-
## $Id: Makefile.am 15375 2008-06-08 22:18:52Z davea $

include $(top_srcdir)/Makefile.incl

LIBS += @CLIENTLIBS@


if OS_DARWIN
   LIBS += -framework IOKit -framework Foundation -framework ScreenSaver -framework Cocoa -framework Security
   mac_sources = mac/SystemMenu.m \
                 mac/mac_saver_module.cpp \
                 mac/Mac_Saver_ModuleView.m \
                 mac/MacSysMenu.cpp \
                 mac/Mac_GUI.cpp\
                 mac/browser_safari.mm

    mac_headers = mac/*.h
else
   mac_headers =
   mac_sources = 
endif

bin_PROGRAMS = synecmgr

# keep the following alphabetic

synecmgr_SOURCES = \
    AccountInfoPage.cpp \
    AccountKeyPage.cpp \
    AccountManagerInfoPage.cpp \
    AccountManagerProcessingPage.cpp \
    AccountManagerPropertiesPage.cpp \
    AdvancedFrame.cpp \
    AlreadyAttachedPage.cpp \
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
    common/wxPieCtrl.cpp \
    common/wxFlatNotebook.cpp \
    DlgAbout.cpp 		\
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
    ValidateAccountKey.cpp \
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
    $(mac_sources)

EXTRA_DIST = *.h \
    res \
    msw \
    ../lib/error_numbers.h \
    locale $(mac_headers)

CXXCOMPILE = $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
        $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_CXXFLAGS) $(WX_CXXFLAGS) $(CLIENTGUIFLAGS)

if PRECOMPILE_HEADERS
BUILT_SOURCES = stdwx.h.gch
endif

synecmgr_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) $(CLIENTGUIFLAGS)
synecmgr_CXXFLAGS = $(AM_CXXFLAGS) $(WX_CXXFLAGS) $(CLIENTGUIFLAGS)
synecmgr_LDADD = $(LIBBOINC) $(CLIENTGUILIBS)

stdwx.h.gch: stdwx.h
	-rm -f stdwx.h.gch
	$(CXXCOMPILE) -o stdwx.h.gch -xc++-header stdwx.h

all-local: synecmgr

win_config.h: $(top_srcdir)/config.h
	grep '#define.*BOINC.*VERSION' $^ > $@

version: win_config.h

clean-local:
	-rm -f stdwx.h.gch

.PHONY: version
