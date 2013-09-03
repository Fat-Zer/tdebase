kde_module_LTLIBRARIES = kcm_tdm.la

# _don't_ add a theme configurator!!
kcm_tdm_la_SOURCES = background.cpp tdm-appear.cpp tdm-font.cpp tdm-shut.cpp \
	tdm-users.cpp tdm-conv.cpp main.cpp kbackedcombobox.cpp

kcm_tdm_la_LDFLAGS  = $(KDE_RPATH) $(all_libraries) -module -avoid-version -no-undefined
kcm_tdm_la_LIBADD = $(top_builddir)/kcontrol/background/libkcmbgnd.la $(top_builddir)/kcontrol/background/libbgnd.la $(LIB_TDEIO) $(LIB_KNEWSTUFF)

AM_CPPFLAGS= -I$(top_builddir)/kcontrol/background \
          -I$(top_srcdir)/kcontrol/background \
          -I$(top_srcdir)/kdesktop $(all_includes)

METASOURCES = AUTO

messages:
	$(XGETTEXT) $(kcm_tdm_la_SOURCES) -o $(podir)/tdmconfig.pot

xdg_apps_DATA = tdm.desktop
