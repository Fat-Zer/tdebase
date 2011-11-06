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
  ${CMAKE_SOURCE_DIR}/kwin/lib
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES redmond.desktop DESTINATION ${DATA_INSTALL_DIR}/kwin )


##### kwin3_redmond (module) ####################

tde_add_kpart( kwin3_redmond AUTOMOC
  SOURCES redmond.cpp
  LINK tdecorations-shared tdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
