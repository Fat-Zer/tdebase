INCLUDES                       = $(all_includes)

kde_module_LTLIBRARIES         = twin_keramik_config.la

twin_keramik_config_la_SOURCES = config.cpp keramikconfig.ui
twin_keramik_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
twin_keramik_config_la_LIBADD  = $(LIB_TDEUI)

METASOURCES                    = AUTO
noinst_HEADERS                 = config.h keramikconfig.h

lnkdir                         = $(kde_datadir)/twin

