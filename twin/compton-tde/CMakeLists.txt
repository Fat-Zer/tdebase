#################################################
#
#  (C) 2014 Timothy Pearson
#  kb9vqf (AT) pearsoncomputing (DOT) net
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################


if( NOT WITH_XRENDER )
  tde_message_fatal( "xrender support is needed to build compton-tde.\n Pass -DWITH_XRENDER=ON to cmake arguments." )
elseif( NOT WITH_XFIXES )
  tde_message_fatal( "xfixes support is needed to build compton-tde.\n Pass -DWITH_XFIXES=ON to cmake arguments." )
endif( )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${DBUS_INCLUDE_DIRS}
)


##### compton-tde (executable) ###################

# TDE to compton config option map
#  WITH_XINNERAMA         -> CONFIG_XINERAMA
#  WITH_XRANDR            -> CONFIG_XANDR
#  HAVE_XEXT_XSYNCFENCE   -> CONFIG_XSYNC
#  WITH_OPENGL            -> CONFIG_VSYNC_OPENGL
#  WITH_OPENGL            -> CONFIG_VSYNC_OPENGL_GLSL
#  WITH_OPENGL            -> CONFIG_VSYNC_OPENGL_FBO
#  WITH_PCRE              -> CONFIG_REGEX_PCRE
#  WITH_PCRE              -> CONFIG_REGEX_PCRE_JIT
#  WITH_LIBCONFIG         -> CONFIG_LIBCONFIG
#
#  HAVE_LIBCONFIG_OLD_API -> CONFIG_LIBCONFIG_LEGACY (set up in compton_config.h)
#  CONFIG_XSYNC           -> CONFIG_GLX_SYNC
#
#  CONFIG_DBUS              - always ON
#  CONFIG_C2                - always ON

# TODO: think about some configuration option for CONFIG_VSYNC_DRM
# NOTE: HAVE__XEXT_XSYNCFENCE is an automatically detected feature that shows
#       that libXext provides coresponding feature.

add_definitions("-std=c99")

set( compton_SRCS compton.c )

set( CONFIG_XSYNC ${HAVE_XEXT_XSYNCFENCE} )

if( WITH_OPENGL )
  set( CONFIG_VSYNC_OPENGL      ${WITH_OPENGL} )
  set( CONFIG_VSYNC_OPENGL_GLSL ${WITH_OPENGL} )
  set( CONFIG_VSYNC_OPENGL_FBO  ${WITH_OPENGL} )
  set( CONFIG_GLX_SYNC          ${CONFIG_XSYNC} )
  list( APPEND compton_LIBRARIES ${GL_LIBRARIES})
  list( APPEND compton_SRCS opengl.c )
endif( )

if( WITH_LIBCONFIG )
  set( CONFIG_LIBCONFIG ${WITH_LIBCONFIG} )
  list( APPEND compton_LIBRARIES ${LIBCONFIG_LIBRARIES} )
endif( )

if( WITH_XINERAMA )
  set( CONFIG_XINERAMA ${WITH_XINERAMA} )
  list( APPEND compton_LIBRARIES ${XINERAMA_LIBRARIES} )
endif( )

if( WITH_XRANDR )
  set( CONFIG_XRANDR ${WITH_XRANDR} )
  list( APPEND compton_LIBRARIES ${XRANDR_LIBRARIES} )
endif( )

if( WITH_PCRE )
  set( CONFIG_REGEX_PCRE        ${WITH_PCRE} )
  set( CONFIG_REGEX_PCRE_JIT    ${WITH_PCRE} )
  list( APPEND compton_LIBRARIES ${LIBPCRE_LIBRARIES} )
endif( )

configure_file( compton_config.h.cmake compton_config.h )

# permanently turn on some optional features: dbus c2
list( APPEND compton_SRCS dbus.c c2.c )
list( APPEND compton_LIBRARIES ${DBUS_LIBRARIES} )

tde_add_executable( compton-tde
    SOURCES ${compton_SRCS}
  LINK m ${compton_LIBRARIES} ${XRENDER_LIBRARIES} ${XFIXES_LIBRARIES}
    ${XDAMAGE_LIBRARIES} ${XEXT_LIBRARIES} ${XCOMPOSITE_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
