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

add_subdirectory( kdedmodule )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES remote.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kioremote (static) ########################

set( target kioremote )

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES kio_remote.cpp remoteimpl.cpp
)


##### kio_remote (module) #######################

set( target kio_remote )

configure_file( ${CMAKE_SOURCE_DIR}/cmake/modules/template_dummy_cpp.cmake dummy.cpp COPYONLY )

tde_add_kpart( ${target}
  SOURCES dummy.cpp
  EMBED kioremote-static
  LINK tdeio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
