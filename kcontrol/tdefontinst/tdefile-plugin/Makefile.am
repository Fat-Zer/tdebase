kde_module_LTLIBRARIES = tdefile_font.la

tdefile_font_la_SOURCES = KFileFont.cpp
tdefile_font_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBFREETYPE_RPATH) -module $(KDE_PLUGIN)
tdefile_font_la_LIBADD = $(LIB_TDEIO) $(LIBFREETYPE_LIBS) ../lib/libtdefontinst.la

kdelnkdir = $(kde_servicesdir)
kde_services_DATA = tdefile_font.desktop

AM_CPPFLAGS = -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS)

METASOURCES = AUTO
noinst_HEADERS = KFileFont.h
