kde_module_LTLIBRARIES = tdeio_fonts.la
tdeio_fonts_la_SOURCES = \
KioFonts.cpp

tdeio_fonts_la_LIBADD = ../../fonts/libkxftconfig.la $(LIBFONTCONFIG_LIBS) $(LIBFREETYPE_LIBS) $(LIB_TDEIO) -ltdesu ../lib/libtdefontinst.la
tdeio_fonts_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(KDE_RPATH) $(LIBFONTCONFIG_RPATH) $(LIBFREETYPE_RPATH) -module -avoid-version -no-undefined
AM_CPPFLAGS= -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS) -D_LARGEFILE64_SOURCE

# The kxftconfig stuf really belongs to tdebase/kcontrol/fonts - here only so that can distribute this as an archive.
noinst_HEADERS = \
KioFonts.h

servicesdir = $(kde_servicesdir)
services_DATA = fonts.protocol

vfsdata_DATA = fonts.desktop
vfsdatadir = $(kde_datadir)/konqsidebartng/virtual_folders/services

mimetype_DATA = folder.desktop system-folder.desktop package.desktop
mimetypedir = $(kde_mimedir)/fonts
