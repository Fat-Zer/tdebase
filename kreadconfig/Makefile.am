AM_CPPFLAGS = -DQT_NO_CAST_ASCII 

INCLUDES = $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
LDADD   =       $(LIB_TDECORE)

bin_PROGRAMS	= kreadconfig kwriteconfig
kreadconfig_SOURCES	= kreadconfig.cpp
kwriteconfig_SOURCES	= kwriteconfig.cpp

messages:
	$(XGETTEXT) $(kreadconfig_SOURCES) -o $(podir)/kreadconfig.pot
	$(XGETTEXT) $(kwriteconfig_SOURCES) -o $(podir)/kwriteconfig.pot
