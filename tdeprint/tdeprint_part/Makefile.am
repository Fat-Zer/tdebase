AM_CPPFLAGS = -D_TDEPRINT_COMPILE

kde_module_LTLIBRARIES = libtdeprint_part.la

libtdeprint_part_la_SOURCES = printpart.cpp
libtdeprint_part_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libtdeprint_part_la_LIBADD = -ltdeprint_management $(LIB_KPARTS)

noinst_HEADERS = printpart.h

METASOURCES = AUTO
INCLUDES = $(all_includes)

# service
myservice_DATA = tdeprint_part.desktop
myservicedir = $(kde_servicesdir)

# UI file
myrcfile_DATA = tdeprint_part.rc
myrcfiledir = $(kde_datadir)/tdeprint_part

messages: rc.cpp
	$(EXTRACTRC) tdeprint_part.rc >> rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tdeprint_part.pot
