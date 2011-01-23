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

add_subdirectory( config )

include_directories(
  ${CMAKE_SOURCE_DIR}/kwin/lib
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES modernsystem.desktop DESTINATION ${DATA_INSTALL_DIR}/kwin )


##### kwin3_modernsys (module) ##################

tde_add_kpart( kwin3_modernsys
  SOURCES modernsys.cpp
  LINK kdecorations-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
