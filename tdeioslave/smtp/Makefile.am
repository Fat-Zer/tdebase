
INCLUDES= -I$(srcdir)/../.. -I$(srcdir)/.. $(SSL_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = tdeio_smtp.la

tdeio_smtp_la_SOURCES = smtp.cc request.cc response.cc capabilities.cc command.cc transactionstate.cc
tdeio_smtp_la_LIBADD  = $(LIB_TDEIO) $(SASL2_LIBS)
tdeio_smtp_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = smtp.h request.h response.h capabilities.h command.h transactionstate.h

kdelnk_DATA = smtp.protocol smtps.protocol
kdelnkdir = $(kde_servicesdir)

TESTS = test_headergeneration test_responseparser test_commands

check_PROGRAMS = $(TESTS) interactivesmtpserver

test_headergeneration_SOURCES = test_headergeneration.cc
test_headergeneration_LDADD =  $(LIB_TDECORE)
test_headergeneration_LDFLAGS = $(all_libraries)

test_responseparser_SOURCES = test_responseparser.cc
test_responseparser_LDADD =  $(LIB_TDECORE)
test_responseparser_LDFLAGS = $(all_libraries)

test_commands_SOURCES = test_commands.cc
test_commands_LDADD =  $(tdeio_smtp_la_LIBADD)
test_commands_LDFLAGS = $(all_libraries)

interactivesmtpserver_SOURCES = interactivesmtpserver.cc
interactivesmtpserver_LDADD = $(LIB_QT)
interactivesmtpserver_LDFLAGS = $(all_libraries)
interactivesmtpserver_METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cc -o $(podir)/tdeio_smtp.pot
