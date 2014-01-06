## Makefile.am of tdedebugdialog

INCLUDES=	$(all_includes)

####### Files

bin_PROGRAMS = tdedebugdialog

tdedebugdialog_SOURCES = main.cpp kabstractdebugdialog.cpp tdedebugdialog.cpp tdelistdebugdialog.cpp
tdedebugdialog_METASOURCES = AUTO
tdedebugdialog_LDFLAGS =       $(all_libraries) $(KDE_RPATH)
tdedebugdialog_LDADD   =       $(LIB_TDEUI)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/tdedebugdialog.pot	

