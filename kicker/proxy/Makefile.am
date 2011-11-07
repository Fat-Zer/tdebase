INCLUDES = -I$(srcdir)/../libkicker $(all_includes)

bin_PROGRAMS = 
noinst_PROGRAMS = extensiondebugger
lib_LTLIBRARIES =
tdeinit_LTLIBRARIES = appletproxy.la extensionproxy.la

METASOURCES = AUTO

appletproxy_la_LIBADD = $(LIB_TDEUI) ../libkicker/libkickermain.la
appletproxy_la_SOURCES = appletproxy.cpp
appletproxy_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module -avoid-version

extensionproxy_la_LIBADD = $(LIB_TDEUI) ../libkicker/libkickermain.la
extensionproxy_la_SOURCES = extensionproxy.cpp
extensionproxy_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module -avoid-version

extensiondebugger_SOURCES = extensiondebugger.cpp
extensiondebugger_LDADD = ../libkicker/libkickermain.la
extensiondebugger_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -lktexteditor

messages:
	$(XGETTEXT) appletproxy.cpp -o $(podir)/appletproxy.pot
	$(XGETTEXT) extensionproxy.cpp -o $(podir)/extensionproxy.pot
