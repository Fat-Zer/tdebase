## Makefile.am of tdebase/tdeioslave/man

INCLUDES=$(all_includes)

kde_module_LTLIBRARIES = tdeio_smb.la
tdeio_smb_la_SOURCES = tdeio_smb.cpp \
             tdeio_smb_auth.cpp \
             tdeio_smb_browse.cpp \
             tdeio_smb_config.cpp \
             tdeio_smb_dir.cpp \
             tdeio_smb_file.cpp \
             tdeio_smb_internal.cpp \
	     tdeio_smb_mount.cpp

tdeio_smb_la_LIBADD = -ltdeio -lsmbclient $(SMBCLIENT_EXTRA_LIBS)

tdeio_smb_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = tdeio_smb.h tdeio_smb_internal.h

kdelnk_DATA = smb.protocol

kdelnkdir = $(kde_servicesdir)

dirtree_DATA = smb-network.desktop
dirtreedir = $(kde_datadir)/konqueror/dirtree/remote

remote_DATA = smb-network.desktop
remotedir = $(kde_datadir)/remoteview

METASOURCES = AUTO

mimetypedir = $(kde_mimedir)/application
mimetype_DATA = x-smb-workgroup.desktop x-smb-server.desktop

messages:
	$(XGETTEXT) *.cpp -o $(podir)/tdeio_smb.pot
