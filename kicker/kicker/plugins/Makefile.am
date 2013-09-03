INCLUDES    = -I$(top_srcdir)/interfaces $(all_includes) $(LIBBEAGLE_CFLAGS) $(GLIB_CFLAGS)
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kickoffsearch_beagle.la

# Srcs for the plugin
kickoffsearch_beagle_la_SOURCES = kickoff-beagle-plugin.cpp beaglesearch.cpp

# Libs needed by the plugin
kickoffsearch_beagle_la_LIBADD = $(LIB_TDEPARTS) ../interfaces/libkickoffsearch_interfaces.la \
	$(LIBBEAGLE_LIBADD) $(GLIB_LIBADD)

# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
kickoffsearch_beagle_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

# Install the desktop file needed to detect the plugin
kde_services_DATA = kickoffsearch_beagle.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kickoffsearch_beagle.pot
