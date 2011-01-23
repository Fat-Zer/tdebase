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

add_subdirectory( kcmcgi )


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES cgi.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kio_cgi (module) ##########################

set( target kio_cgi )

tde_add_kpart( ${target} AUTOMOC
  SOURCES cgi.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
