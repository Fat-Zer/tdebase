kde_module_LTLIBRARIES = kcm_samba.la

kcm_samba_la_SOURCES = main.cpp ksmbstatus.cpp kcmsambaimports.cpp kcmsambalog.cpp kcmsambastatistics.cpp

kcm_samba_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_samba_la_LIBADD = $(LIB_TDEIO)

AM_CPPFLAGS = $(all_includes)

METASOURCES = AUTO 

messages:
	$(XGETTEXT) $(kcm_samba_la_SOURCES) -o $(podir)/kcmsamba.pot

xdg_apps_DATA = smbstatus.desktop
