
INCLUDES = -I$(srcdir)/../../lib $(all_includes)

SUBDIRS = config

kde_module_LTLIBRARIES = twin3_modernsys.la

twin3_modernsys_la_SOURCES = modernsys.cpp
twin3_modernsys_la_LIBADD = ../../lib/libtdecorations.la
twin3_modernsys_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module

METASOURCES = AUTO
noinst_HEADERS = modernsys.h

lnkdir = $(kde_datadir)/twin/
lnk_DATA = modernsystem.desktop

EXTRA_DIST = $(lnk_DATA)

