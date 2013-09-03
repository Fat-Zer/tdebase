## Makefile.am of tdebase/tdeioslave/floppy

INCLUDES= $(all_includes)
AM_LDFLAGS = $(all_libraries)

####### Files

kde_module_LTLIBRARIES = tdeio_floppy.la

tdeio_floppy_la_SOURCES = tdeio_floppy.cpp program.cpp
tdeio_floppy_la_LIBADD = $(LIB_TDEIO)
tdeio_floppy_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

kdelnk_DATA = floppy.protocol
kdelnkdir = $(kde_servicesdir)

METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/tdeio_floppy.pot
