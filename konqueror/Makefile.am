AM_CPPFLAGS = -D_LARGEFILE64_SOURCE

INCLUDES = -I$(top_srcdir)/libkonq -I$(top_srcdir)/kcontrol/input $(all_includes)

SUBDIRS = . client iconview listview keditbookmarks shellcmdplugin about pics sidebar preloader quickprint remoteencodingplugin kttsplugin

bin_PROGRAMS =
lib_LTLIBRARIES =
tdeinit_LTLIBRARIES = konqueror.la

noinst_LTLIBRARIES = libkonqueror_intern.la
konqdatadir = $(kde_datadir)/konqueror
konqdata_DATA = konqueror.rc konq-simplebrowser.rc

kde_apps_DATA = konqueror.desktop

# servicetypedir = $(kde_servicetypesdir)
# servicetype_DATA = konq_metaview.desktop

METASOURCES = AUTO

include_HEADERS = KonquerorIface.h

libkonqueror_intern_la_SOURCES = konq_settingsxt.kcfgc
konqueror_la_SOURCES = konq_main.cc \
	    KonquerorIface.cc KonquerorIface.skel \
	    KonqMainWindowIface.cc KonqMainWindowIface.skel \
	    KonqViewIface.cc KonqViewIface.skel \
	    konq_guiclients.cc \
	    konq_run.cc konq_view.cc konq_viewmgr.cc \
	    konq_misc.cc \
	    konq_frame.cc \
	    konq_tabs.cc \
	    konq_actions.cc \
	    konq_profiledlg.cc \
	    konq_factory.cc \
	    konq_combo.cc	\
	    konq_browseriface.cc \
	    delayedinitializer.cc \
	    konq_mainwindow.cc  \
	    konq_extensionmanager.cc

konqueror_la_PCH = AUTO

noinst_HEADERS = KonqMainWindowIface.h KonqViewIface.h delayedinitializer.h \
	konq_actions.h konq_browseriface.h konq_combo.h konq_factory.h \
	konq_frame.h konq_tabs.h konq_guiclients.h konq_main.h konq_mainwindow.h \
	konq_misc.h konq_openurlrequest.h konq_profiledlg.h konq_run.h \
	konq_view.h konq_viewmgr.h konq_extensionmanager.h version.h

konqueror_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) -lconnectionmanager -lnetworkstatus
konqueror_la_LIBADD = ../libkonq/libkonq.la libkonqueror_intern.la $(LIBMALLOC) $(LIB_TDEUTILS) -lconnectionmanager -lnetworkstatus

# Hmm, this experiment of a static konq failed, don't trust it...
# (but feel free to fix it :)

# Note that you need to --enable-debug in tdelibs and tdebase,
# then make clean ; make install in tdelibs and libkonq,
# and make clean ; make konqueror_static in here.
#check_PROGRAMS = konqueror_static
#
#konqueror_static_SOURCES = dummy.cc
#konqueror_static_LDADD = konqueror.la *view/*.la dirtree/*.la $(libdir)/libtdehtml.la
#konqueror_static_LDFLAGS = $(all_libraries) -static

messages: rc.cpp
	$(EXTRACTRC) *.rc */*.rc >> rc.cpp
	$(EXTRACTRC) sidebar/trees/history_module/history_dlg.ui >> rc.cpp
	$(XGETTEXT) -kaliasLocal rc.cpp *.h *.cc *view/*h *view/*cc  kedit*/*.h kedit*/*.cpp about/*.h about/*.cc remoteencodingplugin/*.cpp remoteencodingplugin/*.h shellcmdplugin/*.cpp -o `find sidebar -name "*.cpp"` `find sidebar -name "*.h"` -o $(podir)/konqueror.pot

xdg_apps_DATA = kfmclient.desktop kfmclient_dir.desktop kfmclient_html.desktop \
	kfmclient_war.desktop konqbrowser.desktop konquerorsu.desktop Home.desktop

konq_data4_DATA = konqfilemgr.desktop
konq_data4dir = $(kde_appsdir)/.hidden

kde_kcfg_DATA = konqueror.kcfg

####### Build rules
KonquerorIface.lo: konq_settingsxt.h
konq_actions.lo: konq_settingsxt.h
konq_guiclients.lo: konq_settingsxt.h
konq_main.lo: konq_settingsxt.h
konq_mainwindow.lo: konq_settingsxt.h
konq_profiledlg.lo: konq_settingsxt.h
konq_settingsxt.lo: konq_settingsxt.h
konq_tabs.lo: konq_settingsxt.h
konq_view.lo: konq_settingsxt.h
konq_viewmgr.lo: konq_settingsxt.h

# Use a hand-made rule for profiles because they are renamed upon installation
profiledir = $(kde_datadir)/konqueror/profiles
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(profiledir)
	$(INSTALL_DATA) $(srcdir)/profile_webbrowsing.desktop $(DESTDIR)$(profiledir)/webbrowsing
	$(INSTALL_DATA) $(srcdir)/profile_filemanagement.desktop $(DESTDIR)$(profiledir)/filemanagement
	$(INSTALL_DATA) $(srcdir)/profile_midnightcommander.desktop $(DESTDIR)$(profiledir)/midnightcommander
	$(INSTALL_DATA) $(srcdir)/profile_tabbedbrowsing.desktop $(DESTDIR)$(profiledir)/tabbedbrowsing
	$(INSTALL_DATA) $(srcdir)/profile_kde_devel.desktop $(DESTDIR)$(profiledir)/kde_devel
	$(INSTALL_DATA) $(srcdir)/profile_simplebrowser.desktop $(DESTDIR)$(profiledir)/simplebrowser

#	$(INSTALL_DATA) $(srcdir)/profile_filepreview.desktop $(DESTDIR)$(profiledir)/filepreview

uninstall-local:
	-rm -f $(DESTDIR)$(profiledir)/webbrowsing $(DESTDIR)$(profiledir)/filemanagement $(DESTDIR)$(profiledir)/midnightcommander $(DESTDIR)$(profiledir)/filepreview
