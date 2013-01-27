kde_module_LTLIBRARIES = tdefile_media.la

tdefile_media_la_SOURCES = tdefilemediaplugin.cpp
tdefile_media_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module $(KDE_PLUGIN)
tdefile_media_la_LIBADD = $(LIB_KIO) ../libmediacommon/libmediacommon.la

kdelnkdir = $(kde_servicesdir)
kde_services_DATA = tdefile_media.desktop

INCLUDES = -I$(srcdir)/../libmediacommon $(all_includes)

METASOURCES = AUTO
noinst_HEADERS = tdefilemediaplugin.h
