INCLUDES = -I$(srcdir)/../core -I../core -I$(srcdir)/../buttons \
	-I../../libkicker -I$(srcdir)/../../libkicker \
	-I$(top_srcdir)/libkonq -I$(top_srcdir)/tdmlib $(DBUS_INCS) $(all_includes)

noinst_LTLIBRARIES = libkicker_ui.la

libkicker_ui_la_SOURCES = addbutton_mnu.cpp appletitem.ui appletview.ui addapplet.cpp \
        addapplet_mnu.cpp appletop_mnu.cpp \
        browser_mnu.cpp client_mnu.cpp dirdrop_mnu.cpp \
        nonKDEButtonSettings.ui exe_dlg.cpp k_new_mnu.cpp k_mnu.cpp k_mnu.skel\
        quickbrowser_mnu.cpp service_mnu.cpp kmenuitembase.ui \
        addextension_mnu.cpp extensionop_mnu.cpp k_mnu_stub.cpp \
        recentapps.cpp browser_dlg.cpp itemview.cpp kickoff_bar.cpp \
        removeapplet_mnu.cpp removeextension_mnu.cpp removecontainer_mnu.cpp \
        removebutton_mnu.cpp popupmenutitle.cpp popupmenutop.cpp hidebutton.cpp \
        addappletvisualfeedback.cpp clicklineedit.cpp flipscrollview.cpp \
        media_watcher.cpp media_watcher.skel mykickoffsearchinterface.cpp query.cpp

libkicker_ui_la_LIBADD = $(top_builddir)/libkonq/libkonq.la $(top_builddir)/tdmlib/libdmctl.la \
		$(LIB_TDEABC) ../interfaces/libkickoffsearch_interfaces.la

libkicker_ui_la_METASOURCES = AUTO

noinst_HEADERS = addapplet.h appletwidget.h addbutton_mnu.h addapplet_mnu.h appletop_mnu.h \
		browser_mnu.h client_mnu.h dirdrop_mnu.h exe_dlg.h k_new_mnu.h k_mnu.h \
		quickbrowser_mnu.h service_mnu.h \
		addextension_mnu.h extensionop_mnu.h \
		recentapps.h browser_dlg.h itemview.h query.h \
		removeapplet_mnu.h removeextension_mnu.h removecontainer_mnu.h \
		removebutton_mnu.h popupmenutitle.h hidebutton.h \
		addappletvisualfeedback.h clicklineedit.h

kicker_ui_data_DATA = default-favs
kicker_ui_datadir = $(kde_datadir)/kicker

removecontainer_mnu.lo: ../../libkicker/kickerSettings.h
removeextension_mnu.lo: ../../libkicker/kickerSettings.h
addextension_mnu.lo: ../core/extensionSettings.h
appletop_mnu.lo: ../../libkicker/kickerSettings.h
extensionop_mnu.lo: ../../libkicker/kickerSettings.h
k_mnu.lo: ../../libkicker/kickerSettings.h
k_new_mnu.lo: ../../libkicker/kickerSettings.h
removecontainer_mnu.lo: ../core/extensionSettings.h
removeextension_mnu.lo: ../core/extensionSettings.h
service_mnu.lo: ../../libkicker/kickerSettings.h
browser_mnu.lo: ../../libkicker/kickerSettings.h
recentapps.lo: ../../libkicker/kickerSettings.h

