## Makefile.am of tdebase/tdeioslave/sftp

INCLUDES = $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -ltdetexteditor
METASOURCES = AUTO

####### Files

check_PROGRAMS = ksshprocesstest

ksshprocesstest_SOURCES = ksshprocesstest.cpp
ksshprocesstest_LDADD = $(LIB_KSYCOCA) ksshprocess.lo process.lo atomicio.lo

kde_module_LTLIBRARIES = tdeio_sftp.la

tdeio_sftp_la_SOURCES = process.cpp atomicio.cpp tdeio_sftp.cpp sftpfileattr.cpp ksshprocess.cpp
tdeio_sftp_la_LIBADD = $(LIB_KIO)
tdeio_sftp_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
noinst_HEADERS = atomicio.h tdeio_sftp.h ksshprocess.h process.h sftpfileattr.h sftp.h

kdelnk_DATA = sftp.protocol
kdelnkdir = $(kde_servicesdir)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/tdeio_sftp.pot
