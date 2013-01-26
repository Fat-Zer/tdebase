bin_PROGRAMS = kio_media_mounthelper

INCLUDES = -I$(srcdir)/../libmediacommon $(all_includes)
AM_LDFLAGS = $(all_libraries)

kio_media_mounthelper_SOURCES = kio_media_mounthelper.cpp decryptdialog.ui dialog.cpp

kio_media_mounthelper_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -ltdetexteditor
kio_media_mounthelper_LDADD = ../libmediacommon/libmediacommon.la $(LIB_KIO) $(LIB_TDEUI)

METASOURCES = AUTO

