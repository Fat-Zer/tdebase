INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = twin_quartz_config.la

twin_quartz_config_la_SOURCES = config.cpp
twin_quartz_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
twin_quartz_config_la_LIBADD = $(LIB_TDEUI)

METASOURCES = AUTO
noinst_HEADERS = config.h

lnkdir = $(kde_datadir)/twin/

###KMAKE-start (don't edit or delete this block)

###KMAKE-end
