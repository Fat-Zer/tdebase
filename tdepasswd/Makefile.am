
SUBDIRS = kcm

bin_PROGRAMS = tdepasswd
tdepasswd_SOURCES = tdepasswd.cpp passwd.cpp passwddlg.cpp
tdepasswd_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -ltdetexteditor
tdepasswd_LDADD = $(LIB_KIO)

METASOURCES =  AUTO
AM_CPPFLAGS= -I$(top_srcdir)/libkonq $(all_includes)
noinst_HEADERS = passwd.h passwddlg.h

xdg_apps_DATA = tdepasswd.desktop

messages:
	$(XGETTEXT) $(tdepasswd_SOURCES)  -o $(podir)/tdepasswd.pot

