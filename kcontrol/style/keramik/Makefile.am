INCLUDES = $(all_includes)

noinst_HEADERS = keramikconf.h
kde_module_LTLIBRARIES = tdestyle_keramik_config.la
tdestyle_keramik_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
tdestyle_keramik_config_la_LIBADD  = $(LIB_TDEUI)
tdestyle_keramik_config_la_SOURCES = keramikconf.cpp
METASOURCES = AUTO 


messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/tdestyle_keramik_config.pot
