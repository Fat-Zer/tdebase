#################################################
#
#  (C) 2010-2011 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

# termios.h (kdm, kioslave)
if( BUILD_KDM OR BUILD_KIOSLAVES )
  check_include_file( termios.h HAVE_TERMIOS_H )
endif( )


# sys/ioctl.h (kioslave/fish, kcontrol/info)
if( BUILD_KIOSLAVES OR BUILD_KCONTROL )
  check_include_file( sys/ioctl.h HAVE_SYS_IOCTL_H )
endif( )


# pam
if( WITH_PAM AND (BUILD_KCHECKPASS OR BUILD_KDM) )
  check_library_exists( pam pam_start "" HAVE_PAM )
  if( HAVE_PAM )
    check_include_file( "security/pam_appl.h" SECURITY_PAM_APPL_H )
  endif( )
  if( HAVE_PAM AND SECURITY_PAM_APPL_H )
    set( PAM_LIBRARY pam;dl )
  else( )
    tde_message_fatal( "pam are requested, but not found on your system" )
  endif( )
endif( )


# hal (ksmserver, kioslaves)
if( BUILD_KSMSERVER OR (WITH_HAL AND BUILD_KIOSLAVES))
  pkg_search_module( HAL hal )
  if( NOT HAL_FOUND )
    tde_message_fatal( "hal is required, but was not found on your system" )
  endif( )
endif( )


# xrender (kdesktop, konsole, kcontrol, kicker)
if( BUILD_KDESKTOP OR BUILD_KONSOLE OR BUILD_KCONTROL OR BUILD_KICKER )
  pkg_search_module( XRENDER xrender )
  if( XRENDER_FOUND )
    set( HAVE_XRENDER 1 )
  endif( )
endif( )


# xcursor (kioslave, kcontrol)
if( WITH_XCURSOR )
  pkg_search_module( XCURSOR xcursor )
  if( XCURSOR_FOUND )
    set( HAVE_XCURSOR 1 CACHE INTERNAL "" FORCE )
  else( )
    tde_message_fatal( "xcursor are requested, but not found on your system" )
  endif( )
endif( )


# xfixes (klipper)
if( WITH_XFIXES )
  pkg_search_module( XFIXES xfixes )
  if( XFIXES_FOUND )
    set( HAVE_XFIXES 1 CACHE INTERNAL "" FORCE )
  else( )
    tde_message_fatal( "xfixes are requested, but not found on your system" )
  endif( )
endif( )


# GL
if( BUILD_KDESKTOP OR BUILD_KCONTROL OR BUILD_KSCREENSAVER )
check_library_exists( GL glXChooseVisual "" HAVE_GLXCHOOSEVISUAL )
  if( HAVE_GLXCHOOSEVISUAL )
    set( GL_LIBRARY "GL" )
  endif( )
endif( )


# glib-2.0
if( BUILD_NSPLUGINS )
  pkg_search_module( GLIB2 glib-2.0 )
  if( NOT GLIB2_FOUND )
    tde_message_fatal( "glib-2.0 are required, but not found on your system" )
  endif( )
endif( )


# kde_socklen_t
if( BUILD_KIOSLAVES OR BUILD_KSYSGUARD )
  set( kde_socklen_t socklen_t )
endif( )


# getifaddrs (kcontrol, kdm)
if( BUILD_KCONTROL OR BUILD_KDM )
  check_function_exists( getifaddrs HAVE_GETIFADDRS )
endif( )


# xkb (konsole, kdm, kxkb)
if( BUILD_KONSOLE OR BUILD_KDM OR BUILD_KXKB )
  check_include_file( X11/XKBlib.h HAVE_X11_XKBLIB_H )
  if( HAVE_X11_XKBLIB_H )
    check_library_exists( X11 XkbLockModifiers "" HAVE_XKB )
    if( BUILD_KDM )
      check_library_exists( X11 XkbSetPerClientControls "" HAVE_XKBSETPERCLIENTCONTROLS )
    endif( )
  endif( )
endif( )


# XBINDIR, XLIBDIR (kdm, kxkb)
if( BUILD_KDM OR BUILD_KXKB )
  find_program( some_x_program NAMES iceauth xrdb xterm )
  if( NOT some_x_program )
    set( some_x_program /usr/bin/xrdb )
    message( STATUS "Warning: Could not determine X binary directory. Assuming /usr/bin." )
  endif( )
  get_filename_component( proto_xbindir "${some_x_program}" PATH )
  get_filename_component( XBINDIR "${proto_xbindir}" ABSOLUTE )
  get_filename_component( xrootdir "${XBINDIR}" PATH )
  set( XBINDIR ${XBINDIR} CACHE INTERNAL "" FORCE )
  set( XLIBDIR "${xrootdir}/lib/X11" CACHE INTERNAL "" FORCE )
endif( )


# arts
if( WITH_ARTS )
  pkg_search_module( ARTS arts )
  if( NOT ARTS_FOUND )
    message( FATAL_ERROR "\naRts is requested, but was not found on your system" )
  endif( )
else( )
  set( WITHOUT_ARTS 1 )
endif( )


# required stuff
find_package( Qt )
find_package( TQt )
find_package( TDE )


# dbus-tqt need Qt flags
# dbus (kdm, ksmserver)
if( BUILD_KDM OR BUILD_KSMSERVER )

  pkg_search_module( DBUS dbus-1 )
  if( NOT DBUS_FOUND )
    tde_message_fatal( "dbus-1 is required, but was not found on your system" )
  endif( )

  # check for dbus-tqt
  tde_save( CMAKE_REQUIRED_INCLUDES CMAKE_REQUIRED_LIBRARIES )
  set( CMAKE_REQUIRED_INCLUDES ${QT_INCLUDE_DIRS} ${DBUS_INCLUDE_DIRS} )
  set( CMAKE_REQUIRED_LIBRARIES ${TQT_LDFLAGS} )
  check_cxx_source_compiles("
    #include <tqt.h>
    #include <dbus/connection.h>
    int main(int, char**) { return 0; } "
    HAVE_DBUS_QT3_07 )
  tde_restore( CMAKE_REQUIRED_INCLUDES CMAKE_REQUIRED_LIBRARIES )
  if( NOT HAVE_DBUS_QT3_07 )
    tde_message_fatal( "dbus-tqt is required, but was not found on your system" )
  endif( )

endif( )
