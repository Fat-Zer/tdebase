bin_PROGRAMS = kpersonalizer
kpersonalizer_SOURCES = stylepreview.ui krefinepage.cpp \
      tdestylepage.cpp keyecandypage.cpp kospage.cpp kcountrypage.cpp kpersonalizer.cpp \
      main.cpp kfindlanguage.cpp \
      kcountrypagedlg.ui kospagedlg.ui keyecandypagedlg.ui tdestylepagedlg.ui \
      krefinepagedlg.ui  ksysinfo.cpp

kpersonalizer_LDADD   = $(LIB_TDEIO) 

EXTRA_DIST = main.cpp kpersonalizer.cpp kpersonalizer.h kpersonalizer.desktop kcountrypage.cpp kcountrypage.h kospage.cpp kospage.h keyecandypage.cpp keyecandypage.h tdestylepage.cpp tdestylepage.h krefinepage.cpp krefinepage.h cr16-app-kpersonalizer.png cr32-app-kpersonalizer.png README kcountrypagedlg.ui kospage.ui keyecandypagedlg.ui tdestylepagedlg.ui krefinepagedlg.ui kfindlanguage.cpp kfindlanguage.h

xdg_apps_DATA = kpersonalizer.desktop

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/System
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/System/kpersonalizer.desktop

####### tdevelop will overwrite this part!!! (end)############
SUBDIRS		= pics

# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -I$(top_srcdir)

METASOURCES = AUTO

KDE_ICON= AUTO
# the library search path. 
kpersonalizer_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE�s XML GUI builing
#rcdir = $(kde_datadir)/kpersonalizer
#rc_DATA = kpersonalizerui.rc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kpersonalizer.pot; \
	fi

