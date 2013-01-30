AM_CPPFLAGS  = $(all_includes)

lib_LTLIBRARIES =
kde_module_LTLIBRARIES = kcm_randr.la

METASOURCES = AUTO

kcm_randr_la_SOURCES = tderandrmodule.cpp
kcm_randr_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_randr_la_LIBADD = $(LIB_TDEUI) $(LIB_XRANDR)

noinst_HEADERS = tderandrmodule.h tderandrtray.h tderandrapp.h \
    tderandrpassivepopup.h configdialog.h

xdg_apps_DATA = tderandrtray.desktop

tderandr_data_DATA = randr.desktop
tderandr_datadir = $(kde_appsdir)/.hidden

# Autostart
autostartdir = $(prefix)/share/autostart
autostart_DATA = tderandrtray-autostart.desktop

#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/Desktop
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop
#	$(DESTDIR)$(kde_appsdir)/Settings/Desktop/tderandrmodule.desktop

bin_PROGRAMS = tderandrtray

tderandrtray_SOURCES = main.cpp tderandrtray.cpp tderandrapp.cpp tderandrpassivepopup.cpp configdialog.cpp
tderandrtray_LDFLAGS = $(all_libraries) $(KDE_RPATH) -ltderandr
tderandrtray_LDADD = $(LIB_TDEFILE) $(LIB_KUTILS) $(LIB_XRANDR)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tderandr.pot
