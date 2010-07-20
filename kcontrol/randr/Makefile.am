AM_CPPFLAGS  = $(all_includes)

lib_LTLIBRARIES =
kde_module_LTLIBRARIES = kcm_randr.la

METASOURCES = AUTO

kcm_randr_la_SOURCES = krandrmodule.cpp
kcm_randr_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_randr_la_LIBADD = $(LIB_KDEUI) $(LIB_XRANDR)

noinst_HEADERS = krandrmodule.h krandrtray.h krandrapp.h \
    krandrpassivepopup.h configdialog.h

xdg_apps_DATA = krandrtray.desktop

krandr_data_DATA = randr.desktop
krandr_datadir = $(kde_appsdir)/.hidden

# Autostart
autostartdir = $(prefix)/share/autostart
autostart_DATA = krandrtray-autostart.desktop

#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/Desktop
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop
#	$(DESTDIR)$(kde_appsdir)/Settings/Desktop/krandrmodule.desktop

bin_PROGRAMS = krandrtray

krandrtray_SOURCES = main.cpp krandrtray.cpp krandrapp.cpp krandrpassivepopup.cpp configdialog.cpp
krandrtray_LDFLAGS = $(all_libraries) $(KDE_RPATH) -lkrandr
krandrtray_LDADD = $(LIB_KFILE) $(LIB_KUTILS) $(LIB_XRANDR)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/krandr.pot
