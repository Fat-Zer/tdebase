INCLUDES = -I$(srcdir)/../../ui -I../../libkicker $(all_includes)

kde_module_LTLIBRARIES = kickermenu_remotemenu.la

kickermenu_remotemenu_la_SOURCES = remotemenu.cpp remotemenu.skel
kickermenu_remotemenu_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_remotemenu_la_LIBADD = $(LIB_TDEUI) $(top_builddir)/kicker/libkicker/libkickermain.la

kickermenu_remotemenu_la_METASOURCES = AUTO

desktopmenu_DATA = remotemenu.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext

messages:
	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_remotemenu.pot

remotemenu.lo: ../../libkicker/kickerSettings.h
