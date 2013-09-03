INCLUDES= $(all_includes) 

kde_module_LTLIBRARIES = kded_kdeintegration.la

kded_kdeintegration_la_SOURCES = module.cpp
kded_kdeintegration_la_METASOURCES = AUTO
kded_kdeintegration_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_kdeintegration_la_LIBADD = $(LIB_TDEIO)

CLEANFILES = module_functions.cpp module_functions.h

servicesdir = $(kde_servicesdir)/kded
services_DATA = kdeintegration.desktop

module.lo : module_functions.cpp module_functions.h

module_functions.cpp : ../utils/module_functions.cpp
	cp -f ../utils/module_functions.cpp . || exit 1

module_functions.h : ../utils/module_functions.h
	cp -f ../utils/module_functions.h . || exit 1
