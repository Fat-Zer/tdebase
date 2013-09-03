INCLUDES = $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES =  kcm_tdednssd.la
kcm_tdednssd_la_LDFLAGS = -avoid-version -module -no-undefined $(all_libraries)
kcm_tdednssd_la_SOURCES = configdialog.ui kcmdnssd.cpp kcmdnssd.h

xdg_apps_DATA = kcm_tdednssd.desktop
kcm_tdednssd_la_LIBADD = $(LIB_TDEDNSSD) $(LIB_TDEUI)

messages: rc.cpp
	$(EXTRACTRC) *.ui *.rc >> rc.cpp
	$(XGETTEXT) $(kcm_tdednssd_la_SOURCES) rc.cpp -o $(podir)/kcmtdednssd.pot
