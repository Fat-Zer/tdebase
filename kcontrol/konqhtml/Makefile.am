AM_CPPFLAGS  = -I$(top_srcdir)/libkonq -I$(top_srcdir)/kcontrol/input $(all_includes)
AM_LDFLAGS =  $(all_libraries)

METASOURCES = AUTO 

kde_module_LTLIBRARIES = kcm_konqhtml.la

kcm_konqhtml_la_SOURCES = advancedTabDialog.cpp htmlopts.cpp jsopts.cpp \
			     javaopts.cpp pluginopts.cpp appearance.cpp \
			     khttpoptdlg.cpp policydlg.cpp main.cpp \
			     jspolicies.cpp nsconfigwidget.ui policies.cpp \
			     domainlistview.cpp advancedTabOptions.ui \
			     filteropts.cpp

kcm_konqhtml_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_konqhtml_la_LIBADD = -ltdehtml -ltdeui

xdg_apps_DATA = tdehtml_behavior.desktop tdehtml_java_js.desktop tdehtml_fonts.desktop tdehtml_filter.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmkonqhtml.pot

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/WebBrowsing
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Settings/WebBrowsing/nsplugin.desktop
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Settings/WebBrowsing/tdehtml_appearance.desktop
