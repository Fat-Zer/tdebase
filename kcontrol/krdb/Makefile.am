AM_CPPFLAGS = $(all_includes)

SUBDIRS = . themes kcs ad pics


noinst_LTLIBRARIES = libkrdb.la
libkrdb_la_SOURCES = krdb.cpp
libkrdb_la_LIBADD = $(LIB_TDECORE)
libkrdb_la_LDFLAGS = -no-undefined $(all_libraries) 

messages:
	$(XGETTEXT) *.cpp -o $(podir)/krdb.pot

update_DATA = kcmdisplayrc.upd
updatedir = $(kde_datadir)/tdeconf_update
