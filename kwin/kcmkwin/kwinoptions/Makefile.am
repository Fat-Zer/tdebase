METASOURCES = AUTO
INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kcm_twinoptions.la 

kcm_twinoptions_la_SOURCES = windows.cpp mouse.cpp main.cpp
kcm_twinoptions_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_twinoptions_la_LIBADD = $(LIB_TDEUI)

noinst_HEADERS = windows.h mouse.h 

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kcmkwm.pot

twin_kcmtwin_options_data_DATA = twinactions.desktop  twinadvanced.desktop  twinfocus.desktop  twinmoving.desktop  twintranslucency.desktop 
twin_kcmtwin_options_datadir = $(kde_appsdir)/.hidden

xdg_apps_DATA = twinoptions.desktop
