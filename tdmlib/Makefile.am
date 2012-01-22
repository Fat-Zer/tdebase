AM_CPPFLAGS = -I$(top_srcdir)/tdm/kfrontend $(all_includes)

kde_module_LTLIBRARIES = kgreet_classic.la kgreet_pam.la kgreet_winbind.la

kgreet_classic_la_SOURCES = kgreet_classic.cpp
kgreet_classic_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kgreet_classic_la_LIBADD = $(LIB_TDEUI)

kgreet_pam_la_SOURCES = kgreet_pam.cpp
kgreet_pam_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kgreet_pam_la_LIBADD = $(LIB_TDEUI)

kgreet_winbind_la_SOURCES = kgreet_winbind.cpp
kgreet_winbind_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kgreet_winbind_la_LIBADD = $(LIB_TDEUI)

noinst_LTLIBRARIES = libdmctl.la
libdmctl_la_SOURCES = dmctl.cpp
libdmctl_la_LDFLAGS = $(all_libraries) -no-undefined
libdmctl_la_LIBADD = $(LIB_TDECORE) -lXau

METASOURCES = AUTO

noinst_HEADERS = dmctl.h kgreet_classic.h kgreet_winbind.h
include_HEADERS = kgreeterplugin.h

messages:
	$(XGETTEXT) $(kgreet_classic_la_SOURCES) -o $(podir)/kgreet_classic.pot
	$(XGETTEXT) $(kgreet_winbind_la_SOURCES) -o $(podir)/kgreet_winbind.pot
	$(XGETTEXT) $(libdmctl_la_SOURCES) -o $(podir)/libdmctl.pot
