# forcibly remove thread-related defines & flags
AUTOMAKE_OPTIONS = foreign
CPPFLAGS = $(USER_INCLUDES) $(X_INCLUDES) $(KRB4_INCS) $(KRB5_INCS) $(DBUS_INCS) -I.. -I../..
LDFLAGS = $(USER_LDFLAGS) $(X_LDFLAGS) $(X_RPATH) $(KRB4_RPATH) $(KRB5_RPATH)
LDADD = $(LIB_X11) -lXau $(LIBXDMCP) $(PASSWDLIBS) $(LIBSHADOW) $(LIBGEN) \
    $(LIB_LIBS) $(KRB4_LIBS) $(KRB5_LIBS) $(DBUS_LIBS) $(LIBSOCKET) $(LIBRESOLV) \
    $(LIBUCB) $(LIBUTIL) $(LIBPOSIX4)

bin_PROGRAMS = tdm
tdm_SOURCES = \
    access.c \
    auth.c \
    bootman.c \
    choose.c \
    client.c \
    consolekit.c \
    ctrl.c \
    daemon.c \
    dm.c \
    dpylist.c \
    error.c \
    genauth.c \
    inifile.c \
    krb5auth.c \
    mitauth.c \
    netaddr.c \
    policy.c \
    process.c \
    protodpy.c \
    reset.c \
    resource.c \
    rpcauth.c \
    server.c \
    session.c \
    sessreg.c \
    socket.c \
    streams.c \
    util.c \
    xdmauth.c \
    xdmcp.c

EXTRA_DIST = printf.c

noinst_HEADERS = dm.h dm_socket.h dm_error.h dm_auth.h greet.h

# for unsermake (automake is handled by SUBDIRS in ../)
tdm_COMPILE_FIRST = ../config.ci
