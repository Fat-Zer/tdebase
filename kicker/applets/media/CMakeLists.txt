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

if( NOT BUILD_LIBKONQ )
  include( "${TDE_CMAKE_DIR}/libkonq.cmake" )
endif( NOT BUILD_LIBKONQ )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libkonq
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### other data ################################

install( FILES mediaapplet.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )


##### media_panelapplet (module) ################

tde_add_kpart( media_panelapplet AUTOMOC
  SOURCES preferencesdialog.cpp mediumbutton.cpp mediaapplet.cpp
  LINK kickermain-shared konq-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
