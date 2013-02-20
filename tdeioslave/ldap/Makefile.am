## Makefile.am of tdebase/tdeioslave/ldap

INCLUDES   = $(all_includes) $(LDAP_INCS)
AM_CXXFLAGS = -DLDAP_DEPRECATED
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LDAP_RPATH)
LDADD      = $(LIB_KIO) $(LDAP_LIBS)

####### Files

kde_module_LTLIBRARIES = tdeio_ldap.la

tdeio_ldap_la_SOURCES = tdeio_ldap.cpp
tdeio_ldap_la_LIBADD = $(LIB_KIO) $(LDAP_LIBS) $(LIB_TDEABC)
tdeio_ldap_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LDAP_RPATH) -module $(KDE_PLUGIN)

noinst_HEADERS = tdeio_ldap.h

kdelnk_DATA = ldap.protocol ldaps.protocol
kdelnkdir = $(kde_servicesdir)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/tdeio_ldap.pot
