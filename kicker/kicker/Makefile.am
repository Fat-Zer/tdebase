INCLUDES = $(all_includes)

# SUBDIRS = core interfaces ui buttons plugins .
SUBDIRS = core interfaces ui buttons .

bin_PROGRAMS =
lib_LTLIBRARIES =
tdeinit_LTLIBRARIES = kicker.la

CLEANFILES = dummy.cpp

kicker_la_LIBADD = core/libkicker_core.la  buttons/libkicker_buttons.la \
	ui/libkicker_ui.la ../libkicker/libkickermain.la $(LIB_TDEIO) $(LIB_TDEUTILS) $(LIB_TDEABC)

kicker_la_SOURCES = dummy.cpp
kicker_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

autostart_DATA = panel.desktop
autostartdir = $(datadir)/autostart

xdg_apps_DATA = kcmkicker.desktop

messages: rc.cpp
	$(EXTRACTRC) ui/*ui >> rc.cpp
	$(EXTRACTRC) core/*.kcfg >> rc.cpp
	$(XGETTEXT) buttons/*.cpp core/*.cpp ui/*.cpp *.cpp -o $(podir)/kicker.pot

dummy.cpp:
	echo > $@
 
tdeconf_PROGRAMS = kicker-3.4-reverseLayout
tdeconfdir = $(libdir)/tdeconf_update_bin

kicker_3_4_reverseLayout_SOURCES = kicker-3.4-reverseLayout.cpp
kicker_3_4_reverseLayout_LDADD   = $(LIB_QT) $(LIB_TDECORE)
kicker_3_4_reverseLayout_LDFLAGS = $(all_libraries)

updatedir = $(kde_datadir)/tdeconf_update
update_DATA = kickerrc.upd
update_SCRIPTS = kicker-3.1-properSizeSetting.pl kicker-3.5-taskbarEnums.pl

