AM_CPPFLAGS = -I$(srcdir)/../../backend -I$(srcdir)/.. -I../.. \
	-I$(top_srcdir)/tdmlib \
	$(all_includes)

noinst_LIBRARIES = libtdmthemer.a
libtdmthemer_a_SOURCES = \
    tdmthemer.cpp \
    tdmitem.cpp \
    tdmpixmap.cpp \
    tdmrect.cpp \
    tdmlabel.cpp \
    tdmlayout.cpp

METASOURCES = AUTO

libtdmthemer_a_COMPILE_FIRST = ../../config.ci
