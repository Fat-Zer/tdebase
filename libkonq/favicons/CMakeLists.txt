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

add_definitions(
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  #${CMAKE_CURRENT_SOURCE_DIR}
  #${CMAKE_BINARY_DIR}
  #${CMAKE_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TDE_LIBRARY_DIRS}
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES favicons.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )
install( FILES favicons.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS move_favicons.sh DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )


##### kded_favicons (module) ####################

set( target kded_favicons )

set( ${target}_SRCS
  favicons.cpp favicons.skel
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kdeinit_kded-shared
  DESTINATION ${TDE_MODULEDIR}
)
