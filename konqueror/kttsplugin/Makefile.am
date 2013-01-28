INCLUDES    = $(all_includes)
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = libtdehtmlkttsdplugin.la

libtdehtmlkttsdplugin_la_SOURCES = tdehtmlkttsd.cpp
libtdehtmlkttsdplugin_la_LIBADD  = $(LIB_TDEHTML)
libtdehtmlkttsdplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

pluginsdir = $(kde_datadir)/tdehtml/kpartplugins
plugins_DATA = tdehtmlkttsd.rc tdehtmlkttsd.desktop

messages: rc.cpp
	$(EXTRACTRC) *.rc > rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tdehtmlkttsd.pot
