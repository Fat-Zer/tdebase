kde_module_LTLIBRARIES = kcm_clock.la

kcm_clock_la_SOURCES = dtime.cpp tzone.cpp main.cpp 

kcm_clock_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_clock_la_LIBADD = $(LIB_TDEIO)

AM_CPPFLAGS = $(all_includes)
 
METASOURCES = AUTO

messages:
	$(XGETTEXT) $(kcm_clock_la_SOURCES) -o $(podir)/kcmkclock.pot

xdg_apps_DATA = clock.desktop
