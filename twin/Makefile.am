INCLUDES = -I$(srcdir)/lib $(all_includes)

SUBDIRS = lib . killer kcmtwin pics clients oldheaders data

bin_PROGRAMS = 
lib_LTLIBRARIES =
tdeinit_LTLIBRARIES = twin.la

twin_la_SOURCES = workspace.cpp client.cpp placement.cpp atoms.cpp \
	utils.cpp layers.cpp main.cpp popupinfo.cpp tabbox.cpp \
	 options.cpp plugins.cpp events.cpp KWinInterface.skel \
	killwindow.cpp geometrytip.cpp sm.cpp group.cpp bridge.cpp \
	manage.cpp notifications.cpp activation.cpp useractions.cpp \
	geometry.cpp rules.cpp

twin_la_LIBADD = $(LIB_TDEUI) lib/libtdecorations.la
twin_la_LDFLAGS = $(all_libraries) -module -avoid-version

include_HEADERS = KWinInterface.h

KDE_ICON = twin

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) -kaliasLocal *.h *.cpp killer/*.cpp lib/*.cpp -o $(podir)/twin.pot

twin_datadir = $(kde_datadir)/twin

twin_data_DATA= eventsrc

kde_kcfg_DATA = twin.kcfg

include ../admin/Doxyfile.am
