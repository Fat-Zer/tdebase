lib_LTLIBRARIES = libtdefontinst.la

libtdefontinst_la_SOURCES = \
Misc.cpp \
FcEngine.cpp

noinst_HEADERS = \
Misc.h \
FcEngine.h \
KfiConstants.h

libtdefontinst_la_LIBADD = $(LIB_TDECORE) $(LIBFONTCONFIG_LIBS) $(LIBFREETYPE_LIBS) $(LIB_TDEIO) $(LIBXFT_LIB)
libtdefontinst_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBFONTCONFIG_RPATH) $(LIBFREETYPE_RPATH) -no-undefined
AM_CPPFLAGS= $(all_includes) $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS) -D_LARGEFILE64_SOURCE
