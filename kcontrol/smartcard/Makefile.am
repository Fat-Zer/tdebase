AM_CPPFLAGS  = $(all_includes)
kde_module_LTLIBRARIES = kcm_smartcard.la

kcm_smartcard_la_SOURCES = smartcard.cpp smartcardbase.ui nosmartcardbase.ui smartcard.skel

kcm_smartcard_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined

kcm_smartcard_la_LIBADD = -ltdeui -lksmartcard $(LIB_TDEIO)

METASOURCES = AUTO

noinst_HEADERS = smartcard.h

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmsmartcard.pot

xdg_apps_DATA = smartcard.desktop
