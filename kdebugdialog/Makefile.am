## Makefile.am of kdebugdialog

INCLUDES=	$(all_includes)

####### Files

bin_PROGRAMS = kdebugdialog

kdebugdialog_SOURCES = main.cpp kabstrackbugdialog.cpp kdebugdialog.cpp tdelistdebugdialog.cpp
kdebugdialog_METASOURCES = AUTO
kdebugdialog_LDFLAGS =       $(all_libraries) $(KDE_RPATH)
kdebugdialog_LDADD   =       $(LIB_TDEUI)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kdebugdialog.pot	

