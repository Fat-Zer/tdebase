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

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/konqueror
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TDE_LIBRARY_DIRS}
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES konqy_preloader.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )
install( FILES konqy_preload.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )


##### kded_konqy_preloader (module) #############

set( target kded_konqy_preloader )

set( ${target}_SRCS
  preloader.cc preloader.skel
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK konqueror_intern-static tdeio-shared tdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
