INCLUDES = $(all_includes)

noinst_HEADERS = keramitdeconf.h
kde_module_LTLIBRARIES = kstyle_keramik_config.la
kstyle_keramik_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kstyle_keramik_config_la_LIBADD  = $(LIB_TDEUI)
kstyle_keramik_config_la_SOURCES = keramitdeconf.cpp
METASOURCES = AUTO 


messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kstyle_keramik_config.pot
