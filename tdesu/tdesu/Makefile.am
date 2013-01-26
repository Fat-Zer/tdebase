## Makefile.am for tdesu by Geert Jansen

INCLUDES= $(all_includes)

## tdesu
bin_PROGRAMS = 	tdesu
tdesu_SOURCES = tdesu.cpp sudlg.cpp
tdesu_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -ltdetexteditor
tdesu_LDADD   = $(LIB_KIO) -ltdesu
tdesu_METASOURCES =  AUTO
noinst_HEADERS = sudlg.h

## Messages
messages:
	$(XGETTEXT) $(tdesu_SOURCES) -o $(podir)/tdesu.pot
