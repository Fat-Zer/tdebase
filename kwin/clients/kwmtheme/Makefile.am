SUBDIRS=cli_installer
INCLUDES = $(all_includes) 

kde_module_LTLIBRARIES = twin3_kwmtheme.la

twin3_kwmtheme_la_SOURCES = kwmthemeclient.cpp
twin3_kwmtheme_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
twin3_kwmtheme_la_LIBADD  = -ltdecorations

METASOURCES = AUTO
noinst_HEADERS = kwmthemeclient.h

lnkdir = $(kde_datadir)/twin
lnk_DATA = kwmtheme.desktop

