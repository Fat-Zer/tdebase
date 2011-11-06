AM_CPPFLAGS  = $(all_includes)
kde_module_LTLIBRARIES = kcm_displayconfig.la

kcm_displayconfig_la_SOURCES = displayconfig.cpp displayconfigbase.ui displayconfig.skel

kcm_displayconfig_la_LDFLAGS  = $(all_libraries) -lkrandr -module -avoid-version -no-undefined

kcm_displayconfig_la_LIBADD = -ltdeui $(LIB_KIO) $(LIB_XRANDR)

METASOURCES = AUTO

noinst_HEADERS = displayconfig.h

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmdisplayconfig.pot

xdg_apps_DATA = displayconfig.desktop
