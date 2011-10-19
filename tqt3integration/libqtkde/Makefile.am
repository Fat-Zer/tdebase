qtkdelib_LTLIBRARIES = libqtkde.la

libqtkde_la_SOURCES = qtkde.cpp
libqtkde_la_LIBADD = -lDCOP
libqtkde_la_LDFLAGS = $(all_libraries) -module -no-undefined -avoid-version

CLEANFILES = qtkde_functions.cpp

INCLUDES = $(all_includes)
METASOURCES = AUTO

qtkde.lo : qtkde_functions.cpp

qtkde_functions.cpp : ../utils/qtkde_functions.cpp
	cp -f ../utils/qtkde_functions.cpp . || exit 1
