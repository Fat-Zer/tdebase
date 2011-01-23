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
  ${TDE_LIBRARY_DIRS}
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES
    khtmlkttsd.rc khtmlkttsd.desktop
  DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins )


##### libkhtmlkttsdplugin (module) ##############

set( target libkhtmlkttsdplugin )

tde_add_kpart( ${target} AUTOMOC
  SOURCES khtmlkttsd.cpp
  LINK khtml-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
