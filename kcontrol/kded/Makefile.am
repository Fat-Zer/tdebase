kde_module_LTLIBRARIES = kcm_kded.la

kcm_kded_la_SOURCES = kcmkded.cpp

kcm_kded_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_kded_la_LIBADD =  $(LIB_TDEIO)

AM_CPPFLAGS= $(all_includes)

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmkded.pot

xdg_apps_DATA = kcmkded.desktop
