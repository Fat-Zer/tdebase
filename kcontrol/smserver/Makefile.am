kde_module_LTLIBRARIES = kcm_smserver.la

kcm_smserver_la_SOURCES = kcmsmserver.cpp smserverconfigdlg.ui smserverconfigimpl.cpp
kcm_smserver_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_smserver_la_LIBADD = -ltdeui
AM_CPPFLAGS= $(all_includes) 

METASOURCES = AUTO

noinst_HEADERS = kcmsmserver.h smserverconfigimpl.h

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmsmserver.pot

xdg_apps_DATA = kcmsmserver.desktop

EXTRA_DIST = $(xdg_apps_DATA)
