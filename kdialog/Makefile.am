KDE_CXXFLAGS = -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST
INCLUDES = $(all_includes)

bin_PROGRAMS = kdialog

kdialog_SOURCES = kdialog.cpp widgets.cpp tdelistboxdialog.cpp progressdialog.cpp progressdialogiface.skel
kdialog_LDADD = $(LIB_KIO)
kdialog_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -ltdetexteditor

METASOURCES = AUTO

messages:        
	$(XGETTEXT) *.cpp -o $(podir)/kdialog.pot 
