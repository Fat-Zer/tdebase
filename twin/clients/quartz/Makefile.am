
INCLUDES = -I$(srcdir)/../../lib $(all_includes)

SUBDIRS = . config

kde_module_LTLIBRARIES = twin3_quartz.la

twin3_quartz_la_SOURCES = quartz.cpp
twin3_quartz_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
twin3_quartz_la_LIBADD  = $(LIB_TDEUI)  ../../lib/libtdecorations.la

METASOURCES = AUTO
noinst_HEADERS = quartz.h

lnkdir = $(kde_datadir)/twin/
lnk_DATA = quartz.desktop

EXTRA_DIST = $(lnk_DATA)


###KMAKE-start (don't edit or delete this block)

###KMAKE-end
