## Makefile.am for trash file meta info plugin

AM_CPPFLAGS      = $(all_includes)

kde_module_LTLIBRARIES = tdefile_trash.la

tdefile_trash_la_SOURCES = tdefile_trash.cpp
tdefile_trash_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
tdefile_trash_la_LIBADD = ../libtrashcommon.la $(LIB_TDEIO)

METASOURCES = AUTO

services_DATA = tdefile_trash.desktop tdefile_trash_system.desktop
servicesdir = $(kde_servicesdir)
