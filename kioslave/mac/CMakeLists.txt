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
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES mac.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
tde_install_icons( )


##### kio_mac (module) ##########################

set( target kio_mac )

tde_add_kpart( ${target} AUTOMOC
  SOURCES kio_mac.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

