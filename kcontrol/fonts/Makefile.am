noinst_LTLIBRARIES = libkxftconfig.la

libkxftconfig_la_SOURCES = kxftconfig.cpp
libkxftconfig_la_LIBADD = $(LIBFONTCONFIG_LIBS)
libkxftconfig_la_LDFLAGS = $(LIBFONTCONFIG_RPATH)

AM_CPPFLAGS  = $(all_includes) $(LIBFONTCONFIG_CFLAGS) $(FREETYPE2_CFLAGS) -D_LARGEFILE64_SOURCE

kde_module_LTLIBRARIES = kcm_fonts.la

kcm_fonts_la_SOURCES = fonts.cpp
kcm_fonts_la_LDFLAGS = $(KDE_RPATH) $(LIBFONTCONFIG_RPATH) $(FREETYPE2_LDFLAGS) -module -avoid-version $(all_libraries) -no-undefined
kcm_fonts_la_LIBADD = libkxftconfig.la $(top_builddir)/kcontrol/krdb/libkrdb.la $(LIB_TDEUI) $(LIBFONTCONFIG_LIBS)
METASOURCES = AUTO

noinst_HEADERS = fonts.h kxftconfig.h

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmfonts.pot

xdg_apps_DATA = fonts.desktop
