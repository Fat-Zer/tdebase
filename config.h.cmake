#define VERSION "@VERSION@"

// Defined if you have fvisibility and fvisibility-inlines-hidden support.
#cmakedefine __KDE_HAVE_GCC_VISIBILITY 1

// Defined if compiler supports long long type.
#cmakedefine HAVE_LONG_LONG 1

// konsole
#cmakedefine HAVE_PROC_CWD 1

// kdesktop, konsole, kcontrol, kicker
#cmakedefine HAVE_XRENDER 1

// taskmanager, klipper
#cmakedefine HAVE_XFIXES 1

// kdesktop, kcontrol, ksplashml
#cmakedefine HAVE_XCURSOR 1

// konsole, tdm, kxkb
#cmakedefine HAVE_XKB 1

// kxkb
#cmakedefine HAVE_XTEST 1

// xscreensaver
#cmakedefine HAVE_XSCREENSAVER 1

/* Defines where xscreensaver stores its graphic hacks */
#define XSCREENSAVER_HACKS_DIR "@XSCREENSAVER_DIR@"

// libart
#cmakedefine HAVE_LIBART 1

// libr
#cmakedefine HAVE_ELFICON 1

// libconfig
#cmakedefine HAVE_LIBCONFIG 1
#cmakedefine HAVE_LIBCONFIG_OLD_API 1

// tdm, tdeioslave
#cmakedefine HAVE_TERMIOS_H 1

// tdeioslave/media
#cmakedefine WITH_HAL 1
#ifdef WITH_HAL
#define COMPILE_HALBACKEND
#define COMPILE_LINUXCDPOLLING
#endif

// tdeioslave/media
#cmakedefine WITH_TDEHWLIB 1
#ifdef WITH_TDEHWLIB
// forcibly deactivate HAL support and substitute TDE hardware library support
#undef COMPILE_HALBACKEND
#define COMPILE_TDEHARDWAREBACKEND
#endif

// tdeioslave/fish, kcontrol/info
#cmakedefine HAVE_SYS_IOCTL_H 1

// tdeioslave/smtp, tdeioslave/pop3
#cmakedefine HAVE_LIBSASL2 1

// tdm, kcontrol
#cmakedefine HAVE_GETIFADDRS 1

// tdeio_fish
#cmakedefine HAVE_STROPTS 1
#cmakedefine HAVE_LIBUTIL_H 1
#cmakedefine HAVE_UTIL_H 1
#cmakedefine HAVE_PTY_H 1
#cmakedefine HAVE_OPENPTY 1

// tdeio_man
#cmakedefine HAVE_UNISTD_H 1
#cmakedefine HAVE_STRING_H 1

// tdeio_smtp, ksysguard
#cmakedefine kde_socklen_t @kde_socklen_t@

// tdefile_media
#cmakedefine HAVE_STATVFS

// taskmanager
#cmakedefine HAVE_XCOMPOSITE

// kcontrol/fonts
#cmakedefine HAVE_FONTCONFIG 1
#cmakedefine HAVE_FREETYPE2 1

// kcontrol/tdefontinst
#cmakedefine HAVE_XFT 1
#cmakedefine HAVE_GETOPT_H 1

// kcontrol/energy
#cmakedefine HAVE_DPMS 1

// kdesktop, kcontrol/screensaver, tdescreensaver
#cmakedefine HAVE_GLXCHOOSEVISUAL 1

// kcontrol/crypto
#cmakedefine HAVE_SSL 1

// kcontrol/nics
#cmakedefine HAVE_SYS_SOCKIO_H 1
#cmakedefine HAVE_GETNAMEINFO 1
#cmakedefine HAVE_STRUCT_SOCKADDR_SA_LEN 1

// kcontrol/input
#cmakedefine HAVE_LIBUSB 1

// tdeprint
#cmakedefine HAVE_SIGACTION 1
#cmakedefine HAVE_SIGSET 1

// tdesu
#cmakedefine HAVE_STRUCT_UCRED 1
#cmakedefine HAVE_GETPEEREID 1
#cmakedefine HAVE_SYS_SELECT_H 1
#cmakedefine HAVE_SYS_WAIT_H 1
#cmakedefine DEFAULT_SUPER_USER_COMMAND "@DEFAULT_SUPER_USER_COMMAND@"

// tdm, kcheckpass, kdesktop
#cmakedefine HAVE_PAM 1

// kcheckpass
#cmakedefine KCHECKPASS_PAM_SERVICE "@KCHECKPASS_PAM_SERVICE@"

// kdesktop
#cmakedefine TDESCREENSAVER_PAM_SERVICE "@TDESCREENSAVER_PAM_SERVICE@"

// tdm
#cmakedefine XBINDIR "@XBINDIR@"
#define KDE_BINDIR "@TDE_BIN_DIR@"
#define KDE_DATADIR "@TDE_DATA_DIR@"
#define KDE_CONFDIR "@TDE_CONFIG_DIR@"

#cmakedefine HAVE_XKBSETPERCLIENTCONTROLS 1

#cmakedefine HAVE_GETDOMAINNAME 1
#cmakedefine HAVE_INITGROUPS 1
#cmakedefine HAVE_MKSTEMP 1
#cmakedefine HAVE_SETPROCTITLE 1
#cmakedefine HAVE_SYSINFO 1
#cmakedefine HAVE_STRNLEN 1
#cmakedefine HAVE_GETIFADDRS 1
#cmakedefine HAVE_CRYPT 1

#cmakedefine HAVE_SETUSERCONTEXT 1
#cmakedefine HAVE_GETUSERSHELL 1
#cmakedefine HAVE_LOGIN_GETCLASS 1
#cmakedefine HAVE_AUTH_TIMEOK 1

#cmakedefine HAVE_LASTLOG_H 1
#cmakedefine HAVE_TERMIO_H 1

#cmakedefine HAVE_STRUCT_SOCKADDR_IN_SIN_LEN 1
#cmakedefine HAVE_STRUCT_PASSWD_PW_EXPIRE 1
#cmakedefine HAVE_STRUCT_UTMP_UT_USER 1

#cmakedefine HAVE_SETLOGIN 1
#cmakedefine HONORS_SOCKET_PERMS 1

#cmakedefine HAVE_UTMPX 1
#cmakedefine HAVE_LASTLOGX 1
#cmakedefine BSD_UTMP 1

#cmakedefine HAVE_ARC4RANDOM 1
#cmakedefine DEV_RANDOM "@DEV_RANDOM@"

#cmakedefine USE_PAM 1
#cmakedefine TDM_PAM_SERVICE "@TDM_PAM_SERVICE@"

#cmakedefine USESHADOW "@USESHADOW@"
#cmakedefine HAVE_SHADOW "@HAVE_SHADOW@"

#cmakedefine XDMCP 1


// ksmserver
#cmakedefine DBUS_SYSTEM_BUS "@DBUS_SYSTEM_BUS@"
#cmakedefine HAVE__ICETRANSNOLISTEN 1

// ksplashml
#cmakedefine HAVE_XINERAMA 1

// khotkeys
#cmakedefine HAVE_ARTS 1
#cmakedefine COVARIANT_RETURN_BROKEN 1

// tdm, kxkb
#cmakedefine XLIBDIR "@XLIBDIR@"

// tdm, kcontrol
#cmakedefine WITH_XRANDR "@WITH_XRANDR@"

// tsak
#cmakedefine BUILD_TSAK "@BUILD_TSAK@"

// Defined when wanting ksmserver shutdown debugging timing markers in .xsession-errors
#cmakedefine BUILD_PROFILE_SHUTDOWN 1

// Kickoff menu
#cmakedefine KICKOFF_DIST_CONFIG_SHORTCUT1 "@KICKOFF_DIST_CONFIG_SHORTCUT1@"
#cmakedefine KICKOFF_DIST_CONFIG_SHORTCUT2 "@KICKOFF_DIST_CONFIG_SHORTCUT2@"

// TDE compositor binary name
#define TDE_COMPOSITOR_BINARY "compton-tde"
