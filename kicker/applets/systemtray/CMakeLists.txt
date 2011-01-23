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
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES systemtrayapplet.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )


##### systemtray_panelapplet (module) ###########

set( target systemtray_panelapplet )

set( ${target}_SRCS
  systemtrayapplet.cpp systemtrayapplet.skel
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kickermain-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
