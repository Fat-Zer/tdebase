
kdemimedir = $(kde_mimedir)/application
kdemime_DATA = x-ksysguard.desktop

rcdir = $(kde_datadir)/ksysguard
rc_DATA = ksysguardui.rc

xdg_apps_DATA = ksysguard.desktop

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = ksysguardapplet.desktop

# claim, which subdirectories you want to install
SUBDIRS = ksgrd SensorDisplayLib

# set the include path for X, qt and KDE
INCLUDES = -I$(srcdir)/ksgrd -I$(srcdir)/SensorDisplayLib -I$(top_builddir)/ksysguard/gui/SensorDisplayLib $(all_includes)
 
####### This part is very ksysguard specific
# you can add here more. This one gets installed 
bin_PROGRAMS = ksysguard kpm

# Which sources should be compiled for ksysguard.
ksysguard_SOURCES = \
	SensorBrowser.cc \
	WorkSheet.cc \
	WorkSheetSettings.cc \
	Workspace.cc \
	ksysguard.cc ksysguard.skel

ksysguard_LDADD = \
	ksgrd/libksgrd.la \
	SensorDisplayLib/libsensordisplays.la \
	$(LIB_TDEUI) $(LIB_TDEIO) $(LIB_TDEDNSSD)
ksysguard_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor

kpm_SOURCES = kpm.c

appdatadir = $(kde_datadir)/ksysguard
appdata_DATA = ProcessTable.sgrd SystemLoad.sgrd KSysGuardApplet.xml

# This stuff is now for the kicker applet
kde_module_LTLIBRARIES = sysguard_panelapplet.la

sysguard_panelapplet_la_SOURCES = \
	KSysGuardApplet.cc \
	KSGAppletSettings.cc

sysguard_panelapplet_la_LIBADD = \
	ksgrd/libksgrd.la \
	SensorDisplayLib/libsensordisplays.la \
	$(LIB_TDEUI) $(LIB_TDEIO)
sysguard_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

EXTRA_DIST = $(lnk_DATA)

# just to make sure, automake makes them 
METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) `find . -name "*.ui"` >> rc.cpp
	extractattr --attr=display,title SystemLoad.sgrd KSysGuardApplet.xml >> rc.cpp
	$(XGETTEXT) `find . -name "*.cpp" -o -name "*.cc"` -o $(podir)/ksysguard.pot
