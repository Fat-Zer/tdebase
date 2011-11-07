INCLUDES = $(all_includes)

bin_PROGRAMS = kwin_killer_helper

kwin_killer_helper_SOURCES = killer.cpp
kwin_killer_helper_LDADD = $(LIB_TDEUI)
kwin_killer_helper_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -lktexteditor

METASOURCES = AUTO
