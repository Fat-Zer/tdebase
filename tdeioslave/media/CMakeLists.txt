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

add_subdirectory( libmediacommon )
add_subdirectory( mediamanager )
add_subdirectory( medianotifier )
add_subdirectory( mounthelper )
add_subdirectory( tdefile-plugin )
add_subdirectory( tdecmodule )
add_subdirectory( mimetypes )
add_subdirectory( services )
add_subdirectory( propsdlgplugin )


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libmediacommon
  ${CMAKE_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES media.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kiomedia (static) #########################

set( target kiomedia )

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES kio_media.cpp mediaimpl.cpp mediaimpl.skel
)


##### kio_media (module) ########################

set( target kio_media )

tde_add_kpart( ${target} AUTOMOC
  SOURCES dummy.cpp
  EMBED kiomedia-static
  LINK mediacommon-static tdeio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
