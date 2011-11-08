
INCLUDES = -I$(srcdir)/../../lib $(all_includes)

kde_module_LTLIBRARIES = twin3_redmond.la

twin3_redmond_la_SOURCES = redmond.cpp
twin3_redmond_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
twin3_redmond_la_LIBADD = $(LIB_TDECORE) ../../lib/libtdecorations.la

METASOURCES = AUTO
noinst_HEADERS = redmond.h

lnkdir = $(kde_datadir)/twin
lnk_DATA = redmond.desktop

EXTRA_DIST = $(lnk_DATA)

