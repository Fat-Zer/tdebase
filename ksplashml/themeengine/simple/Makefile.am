
INCLUDES = $(all_includes)

bin_PROGRAMS = ksplashsimple
ksplashsimple_SOURCES = main.cpp
ksplashsimple_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -lktexteditor
ksplashsimple_LDADD = $(LIB_XINERAMA) $(LIB_X11) $(LIB_QT)
