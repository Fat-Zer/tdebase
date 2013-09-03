kde_module_LTLIBRARIES = kcm_nic.la

kcm_nic_la_SOURCES = nic.cpp

kcm_nic_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_nic_la_LIBADD = -ltdeui $(LIB_TDEIO) $(LIBSOCKET)

AM_CPPFLAGS= $(all_includes)

METASOURCES = AUTO

messages:
	$(XGETTEXT) $(kcm_nic_la_SOURCES) -o $(podir)/kcmnic.pot

xdg_apps_DATA = nic.desktop
