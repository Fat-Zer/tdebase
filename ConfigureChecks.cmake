#################################################
#
#  (C) 2010-2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  (C) 2014 Timothy Pearson
#  kb9vqf (AT) pearsoncomputing (DOT) net
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

tde_setup_architecture_flags( )

# termios.h (tdm, tdeioslave)
if( BUILD_TDM OR BUILD_TDEIOSLAVES )
  check_include_file( termios.h HAVE_TERMIOS_H )
endif( )


# sys/ioctl.h (tdeioslave/fish, kcontrol/info)
if( BUILD_TDEIOSLAVES OR BUILD_KCONTROL )
  check_include_file( sys/ioctl.h HAVE_SYS_IOCTL_H )
endif( )


# pam
if( WITH_PAM AND (BUILD_KCHECKPASS OR BUILD_TDM) )
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


# hal (ksmserver, tdeioslaves)
if( WITH_HAL )
  pkg_search_module( HAL hal )
  if( NOT HAL_FOUND )
    tde_message_fatal( "hal is required, but was not found on your system" )
  endif( )
endif( )


# udev (tsak)
if( BUILD_TSAK )
  pkg_search_module( UDEV udev )
  if( NOT UDEV_FOUND )
    tde_message_fatal( "udev is required, but was not found on your system" )
  endif( )
endif( )


##### check for gcc visibility support #########
# FIXME
# This should check for [T]Qt3 visibility support

if( WITH_GCC_VISIBILITY )
  if( NOT UNIX )
    tde_message_fatal(FATAL_ERROR "\ngcc visibility support was requested, but your system is not *NIX" )
  endif( NOT UNIX )
  set( __KDE_HAVE_GCC_VISIBILITY 1 )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
endif( )


# xrender (kdesktop, konsole, kcontrol, kicker, twin)
if( WITH_XRENDER OR BUILD_KDESKTOP OR BUILD_KONSOLE OR BUILD_KCONTROL OR BUILD_KICKER )
  pkg_search_module( XRENDER xrender )
  if( XRENDER_FOUND )
    set( HAVE_XRENDER 1 )
  elseif( WITH_XRENDER )
    tde_message_fatal( "xrender is requested, but was not found on your system" )
  endif( )
endif( )


# xcursor (tdeioslave, kcontrol)
if( WITH_XCURSOR )
  pkg_search_module( XCURSOR xcursor )
  if( XCURSOR_FOUND )
    set( HAVE_XCURSOR 1 )
  else( )
    tde_message_fatal( "xcursor is requested, but was not found on your system" )
  endif( )
endif( )


# xcomposite (kicker, twin)
if( WITH_XCOMPOSITE )
  pkg_search_module( XCOMPOSITE xcomposite )
  if( XCOMPOSITE_FOUND )
    set( HAVE_XCOMPOSITE 1 )
  else( XCOMPOSITE_FOUND )
    tde_message_fatal( "xcomposite is requested, but was not found on your system" )
  endif( XCOMPOSITE_FOUND )


  # xdamage (twin/kompmgr)
  pkg_search_module( XDAMAGE xdamage )
  if( NOT XDAMAGE_FOUND )
    tde_message_fatal( "xdamage is required for xcomposite support, but was not found on your system" )
  endif( )

  # xext (twin/kompmgr)
  pkg_search_module( XEXT xext )
  if( NOT XEXT_FOUND )
    tde_message_fatal( "xext is required for xcomposite support, but was not found on your system" )
  endif( )

endif( )


# xfixes (klipper, kicker)
if( WITH_XFIXES )
  pkg_search_module( XFIXES xfixes )
  if( XFIXES_FOUND )
    set( HAVE_XFIXES 1 CACHE INTERNAL "" FORCE )
  else( )
    tde_message_fatal( "xfixes is requested, but was not found on your system" )
  endif( )
endif( )


# libconfig (twin/compton-tde)
if( WITH_LIBCONFIG )
  pkg_search_module( LIBCONFIG libconfig )
  if( LIBCONFIG_FOUND )
    set( HAVE_LIBCONFIG 1 )
    if( LIBCONFIG_VERSION VERSION_LESS 1.5.0 )
      set( HAVE_LIBCONFIG_OLD_API 1 )
    endif( )
  else( LIBCONFIG_FOUND )
    tde_message_fatal( "libconfig is requested, but was not found on your system" )
  endif( )
endif( )


# xtest (kxkb)
if( WITH_XTEST )
  pkg_search_module( XTEST xtst )
  if( XTEST_FOUND )
    set( HAVE_XTEST 1 )
  else( XTEST_FOUND )
    tde_message_fatal( "xtest is requested, but was not found on your system" )
  endif( )
endif( )


# xscreensaver ()
if( WITH_XSCREENSAVER )
  check_library_exists( Xss XScreenSaverQueryInfo "" HAVE_XSSLIB )
  if( HAVE_XSSLIB )
    pkg_search_module( XSS xscrnsaver )
  else( )
    check_library_exists( Xext XScreenSaverQueryInfo "" HAVE_XSSLIB )
    if( HAVE_XSSLIB )
      pkg_search_module( XSS xext )
    endif( )
  endif( )

  check_include_file( X11/extensions/scrnsaver.h HAVE_XSCREENSAVER_H )
  if( HAVE_XSSLIB AND HAVE_XSCREENSAVER_H )
    set( HAVE_XSCREENSAVER 1 )
  else( )
    tde_message_fatal( "xscreensaver is requested, but was not found on your system" )
  endif( )

  # We don't really need the xscreensaver package for build, we only must know 
  # there xscreensaver stores its executables. So leave user the posibility 
  # to define XSCREENSAVER_DIR to so let the user.
  include( FindXscreensaver.cmake ) # not really good practise
  if( NOT XSCREENSAVER_DIR )
    tde_message_fatal( 
      "xscreensaver is requested, but cmake can not determine the location of XSCREENSAVER_DIR
 You have to either specify it manually with e.g. -DXSCREENSAVER_DIR=/usr/lib/misc/xscreensaver/
 or make sure that xscreensaver installed properly" )
  endif( )
endif( )


# openGL (kdesktop or kcontrol or tdescreensaver )
if( WITH_OPENGL )
  pkg_search_module( GL gl )
  if( GL_FOUND )
    # some extra check, stricktly speaking they are not necessary
    tde_save( CMAKE_REQUIRED_LIBRARIES )
    set( CMAKE_REQUIRED_LIBRARIES ${GL_LIBRARIES} )
    check_symbol_exists( glXChooseVisual "GL/glx.h" HAVE_GLXCHOOSEVISUAL )
    tde_restore( CMAKE_REQUIRED_LIBRARIES )
    if( NOT HAVE_GLXCHOOSEVISUAL )
      tde_message_fatal( "opengl is requested and found, but it doesn't provides glXChooseVisual() or GL/glx.h" )
    endif( )
  else( )
    tde_message_fatal( "opengl is requested, but not found on your system" )
  endif( )

  if( BUILD_KCONTROL )
    pkg_search_module( GLU glu )
    if( NOT GLU_FOUND )
      tde_message_fatal( "glu is required, but not found on your system" )
    endif( )
  endif( BUILD_KCONTROL )

endif( )


# glib-2.0
if( BUILD_NSPLUGINS )
  pkg_search_module( GLIB2 glib-2.0 )
  if( NOT GLIB2_FOUND )
    tde_message_fatal( "glib-2.0 are required, but not found on your system" )
  endif( )
endif( )


# kde_socklen_t
if( BUILD_TDEIOSLAVES OR BUILD_KSYSGUARD )
  set( kde_socklen_t socklen_t )
endif( )


# getifaddrs (kcontrol, tdm)
if( BUILD_KCONTROL OR BUILD_TDM )
  check_function_exists( getifaddrs HAVE_GETIFADDRS )
endif( )


# xkb (konsole, tdm, kxkb)
if( BUILD_KONSOLE OR BUILD_TDM OR BUILD_KXKB )
  check_include_file( X11/XKBlib.h HAVE_X11_XKBLIB_H )
  if( HAVE_X11_XKBLIB_H )
    check_library_exists( X11 XkbLockModifiers "" HAVE_XKB )
    if( BUILD_TDM )
      check_library_exists( X11 XkbSetPerClientControls "" HAVE_XKBSETPERCLIENTCONTROLS )
    endif( )
  endif( )
endif( )


# XBINDIR, XLIBDIR (tdm, kxkb)
if( BUILD_TDM OR BUILD_KXKB )
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

# libart

if( WITH_LIBART )
  pkg_search_module( LIBART libart-2.0 )
  if( NOT LIBART_FOUND )
    message(FATAL_ERROR "\nlibart-2.0 support are requested, but not found on your system" )
  endif( NOT LIBART_FOUND )
  set( HAVE_LIBART 1 )
endif( WITH_LIBART )


# required stuff
find_package( TQt )
find_package( TDE )



# dbus (tdm, kdesktop)
if( BUILD_TDM OR BUILD_KDESKTOP )

  pkg_search_module( DBUS dbus-1 )
  if( NOT DBUS_FOUND )
    tde_message_fatal( "dbus-1 is required, but was not found on your system" )
  endif( )

endif( )


# dbus-1-tqt (kdesktop)
if( BUILD_KDESKTOP )

  pkg_search_module( DBUS_1_TQT dbus-1-tqt )
  if( NOT DBUS_1_TQT_FOUND )
    tde_message_fatal( "dbus-1-tqt is required, but was not found on your system" )
  endif( )

endif( )


# dbus-tqt (ksmserver, kicker, tdeioslaves(media))
if( WITH_HAL AND (BUILD_KSMSERVER OR BUILD_KICKER OR BUILD_TDEIOSLAVES) )

  # check for dbus-tqt
  # dbus-tqt need Qt flags
  pkg_check_modules( DBUS_TQT REQUIRED dbus-tqt )
  tde_save( CMAKE_REQUIRED_INCLUDES CMAKE_REQUIRED_LIBRARIES )
  set( CMAKE_REQUIRED_INCLUDES ${DBUS_TQT_INCLUDE_DIRS} ${TQT_INCLUDE_DIRS} ${QT_INCLUDE_DIRS})
  set( CMAKE_REQUIRED_LIBRARIES ${DBUS_TQT_LDFLAGS} ${TQT_LDFLAGS} ${QT_LDFLAGS} )
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

# check for libr
if( WITH_ELFICON )
  pkg_search_module( LIBR libr )
  if( NOT LIBR_FOUND )
      message(FATAL_ERROR "\nelficon support was requested, but libr was not found on your system" )
  endif( NOT LIBR_FOUND )
  if( NOT "${LIBR_VERSION}" STREQUAL "0.6.0" )
      message(FATAL_ERROR "\nelficon support was requested, but the libr version on your system may not be compatible with TDE" )
  endif( NOT "${LIBR_VERSION}" STREQUAL "0.6.0" )
  set( HAVE_ELFICON 1 )
endif( )
