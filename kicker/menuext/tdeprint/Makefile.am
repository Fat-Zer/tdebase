INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kickermenu_tdeprint.la

kickermenu_tdeprint_la_SOURCES = print_mnu.cpp
kickermenu_tdeprint_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_tdeprint_la_LIBADD = $(LIB_TDEUI) $(LIB_TDEIO) -ltdeprint

kickermenu_tdeprint_la_METASOURCES = AUTO

desktopmenu_DATA = printmenu.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext

messages:
	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_tdeprint.pot
