AM_CPPFLAGS  = $(all_includes)
kde_module_LTLIBRARIES = kcm_iccconfig.la

kcm_iccconfig_la_SOURCES = iccconfig.cpp iccconfigbase.ui iccconfig.skel

kcm_iccconfig_la_LDFLAGS  = $(all_libraries) -ltderandr -module -avoid-version -no-undefined

kcm_iccconfig_la_LIBADD = -ltdeui $(LIB_KIO) $(LIB_XRANDR)

METASOURCES = AUTO

noinst_HEADERS = iccconfig.h

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmiccconfig.pot

xdg_apps_DATA = iccconfig.desktop
