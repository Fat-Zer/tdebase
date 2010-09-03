METASOURCES = AUTO
INCLUDES= -I$(top_srcdir)/src $(all_includes) 

# The library containing the plugin base class
lib_LTLIBRARIES =  libkickoffsearch_interfaces.la
libkickoffsearch_interfaces_la_SOURCES = kickoff-search-plugin.cpp kickoffsearchinterface.cpp
libkickoffsearch_interfaces_la_LDFLAGS = $(all_libraries) -version-info 0:0:0

kickoffsearchincludedir = $(includedir)
kickoffsearchinclude_HEADERS = kickoff-search-plugin.h kickoffsearchinterface.h

kde_servicetypes_DATA = kickoffsearchplugin.desktop
