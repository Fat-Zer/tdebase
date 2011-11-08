AUTOMAKE_OPTIONS = foreign

SUBDIRS = config

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = -I$(srcdir)/../../lib $(all_includes)

twindir = $(kde_datadir)/twin/
twin_DATA = plastik.desktop

kde_module_LTLIBRARIES = twin3_plastik.la
twin3_plastik_la_SOURCES = plastik.cpp plastikclient.cpp plastikbutton.cpp misc.cpp
twin3_plastik_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
twin3_plastik_la_LIBADD = $(LIB_TDEUI) ../../lib/libtdecorations.la
twin3_plastik_la_METASOURCES = AUTO

DISTCLEANFILES = $(twin3_plastik_la_METASOURCES)

