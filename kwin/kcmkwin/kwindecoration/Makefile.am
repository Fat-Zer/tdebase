INCLUDES = -I$(srcdir)/../../lib $(all_includes)

kde_module_LTLIBRARIES = kcm_twindecoration.la

kcm_twindecoration_la_SOURCES = twindecoration.cpp buttons.cpp twindecorationIface.skel preview.cpp
noinst_HEADERS = twindecoration.h twindecorationIface.h buttons.h preview.h

kcm_twindecoration_la_LDFLAGS = \
  -module -avoid-version $(all_libraries) -no-undefined

kcm_twindecoration_la_LIBADD = $(LIB_TDEUI) ../../lib/libtdecorations.la

METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kcmtwindecoration.pot

xdg_apps_DATA = twindecoration.desktop
