kde_module_LTLIBRARIES = kded_mediamanager.la

if include_media_halbackend
HALBACKEND_INCS = $(HAL_INCS) $(DBUS_INCS) $(DBUSQT_INCS)
endif

METASOURCES = AUTO
INCLUDES = -I$(srcdir)/../libmediacommon -I../libmediacommon $(HALBACKEND_INCS) $(all_includes)

if include_media_halbackend
HALBACKEND_LIB = libhalbackend.la
libhalbackend_la_SOURCES = halbackend.cpp
libhalbackend_la_LDFLAGS = -avoid-version $(all_libraries) -no-undefined
libhalbackend_la_LIBADD = $(HAL_LIBS) $(DBUS_LIBS) $(DBUSQT_LIBS)
endif

if include_media_linuxcdpolling
LINUXCDPOLLING_LIB = liblinuxcdpolling.la
liblinuxcdpolling_la_SOURCES = linuxcdpolling.cpp
liblinuxcdpolling_la_LDFLAGS = -avoid-version $(all_libraries) -no-undefined
endif

noinst_LTLIBRARIES = $(LINUXCDPOLLING_LIB) $(HALBACKEND_LIB)

kded_mediamanager_la_SOURCES = mediamanager.cpp mediamanager.skel medialist.cpp backendbase.cpp fstabbackend.cpp removablebackend.cpp mediadirnotify.cpp mediadirnotify.skel
kded_mediamanager_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_mediamanager_la_LIBADD = $(LIB_TDESYCOCA) ../libmediacommon/libmediacommon.la $(HALBACKEND_LIB) $(LINUXCDPOLLING_LIB)


servicesdir = $(kde_servicesdir)/kded
services_DATA = mediamanager.desktop

