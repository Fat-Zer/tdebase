INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kickermenu_konqueror.la

kickermenu_konqueror_la_SOURCES = konqy_menu.cpp
kickermenu_konqueror_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_konqueror_la_LIBADD = $(LIB_TDEUI) $(LIB_TDEIO)

kickermenu_konqueror_la_METASOURCES = AUTO

desktopmenu_DATA = konquerormenu.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext

messages:
	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_konqueror.pot
