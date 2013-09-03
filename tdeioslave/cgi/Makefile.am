SUBDIRS = kcmcgi

INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = tdeio_cgi.la

tdeio_cgi_la_SOURCES = cgi.cpp
tdeio_cgi_la_LIBADD = $(LIB_TDEIO)
tdeio_cgi_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = cgi.h

METASOURCES = AUTO

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = cgi.protocol
