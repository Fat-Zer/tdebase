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

add_subdirectory( trees )
add_subdirectory( web_module )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libkonq
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TDE_LIBRARY_DIRS}
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES konqsidebarplugin.h DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other data ################################

install( FILES konqsidebartng.rc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES konq_sidebartng.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES .version DESTINATION ${DATA_INSTALL_DIR}/konqsidebartng/entries )
install( FILES konqsidebartng.upd DESTINATION ${DATA_INSTALL_DIR}/kconf_update )
install( PROGRAMS move_konqsidebartng_entries.sh DESTINATION ${DATA_INSTALL_DIR}/kconf_update )


##### konqsidebarplugin (lib) ###################

set( target konqsidebarplugin )

set( ${target}_SRCS
  konqsidebarplugin.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 4.2.0
  LINK kparts-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### konq_sidebar (module) #####################

set( target konq_sidebar )

set( ${target}_SRCS
  konqsidebar.cpp sidebar_widget.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK konqsidebarplugin-shared konq-shared kio-shared kutils-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
