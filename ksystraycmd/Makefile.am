####### Fiddle here

INCLUDES = $(all_includes)
LDADD    = $(LIB_KDEUI)

#######	Files

bin_PROGRAMS	= ksystraycmd
METASOURCES	= ksystraycmd.moc
noinst_HEADERS  = ksystraycmd.h

ksystraycmd_SOURCES = ksystraycmd.cpp main.cpp
ksystraycmd_LDFLAGS  = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_KDECORE) $(LIB_KDEUI) -lkdefx $(LIB_KIO) -lktexteditor

messages:
	$(XGETTEXT) $(ksystraycmd_SOURCES) -o $(podir)/ksystraycmd.pot


