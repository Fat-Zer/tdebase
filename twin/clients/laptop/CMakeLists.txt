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
  ${CMAKE_SOURCE_DIR}/twin/lib
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES laptop.desktop DESTINATION ${DATA_INSTALL_DIR}/twin )


##### twin3_laptop (module) #####################

tde_add_kpart( twin3_laptop AUTOMOC
  SOURCES laptopclient.cpp
  LINK tdecorations-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
