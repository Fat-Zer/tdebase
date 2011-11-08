METASOURCES = AUTO
AM_CPPFLAGS = $(all_includes) -DKCMRULES
INCLUDES = -I$(top_srcdir)/twin/lib

bin_PROGRAMS =
lib_LTLIBRARIES =
tdeinit_LTLIBRARIES = twin_rules_dialog.la
kde_module_LTLIBRARIES = kcm_twinrules.la 
noinst_LTLIBRARIES = libtwinrules.la

libtwinrules_la_SOURCES = ruleswidget.cpp ruleslist.cpp twinsrc.cpp detectwidget.cpp \
        ruleswidgetbase.ui ruleslistbase.ui detectwidgetbase.ui editshortcutbase.ui
libtwinrules_la_LDFLAGS = $(all_libraries) -no-undefined
libtwinrules_la_LIBADD = $(LIB_TDEUI)

kcm_twinrules_la_SOURCES = kcm.cpp 
kcm_twinrules_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_twinrules_la_LIBADD = libtwinrules.la 
kcm_twinrules_la_COMPILE_FIRST = ruleslistbase.h ruleswidgetbase.h editshortcutbase.h

twin_rules_dialog_la_SOURCES = main.cpp 
twin_rules_dialog_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
twin_rules_dialog_la_LIBADD = libtwinrules.la 
twin_rules_dialog_la_COMPILE_FIRST = ruleslistbase.h ruleswidgetbase.h editshortcutbase.h

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmtwinrules.pot

xdg_apps_DATA = twinrules.desktop
