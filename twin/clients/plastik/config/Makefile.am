INCLUDES                     = $(all_includes)

kde_module_LTLIBRARIES       = twin_plastik_config.la

twin_plastik_config_la_SOURCES = config.cpp configdialog.ui
twin_plastik_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
twin_plastik_config_la_LIBADD  = $(LIB_TDEUI)

METASOURCES                  = AUTO
noinst_HEADERS               = config.h
DISTCLEANFILES               = $(METASOURCES)

lnkdir                       = $(kde_datadir)/twin

