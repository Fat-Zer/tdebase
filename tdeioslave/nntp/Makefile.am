INCLUDES= -I$(srcdir)/../.. -I$(srcdir)/.. $(all_includes)

####### Files

kde_module_LTLIBRARIES = tdeio_nntp.la

tdeio_nntp_la_SOURCES = nntp.cpp
tdeio_nntp_la_LIBADD  = $(LIB_TDEIO)
tdeio_nntp_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

METASOURCES = AUTO

noinst_HEADERS = nntp.h

kdelnk_DATA = nntp.protocol nntps.protocol
kdelnkdir = $(kde_servicesdir)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/tdeio_nntp.pot
