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

install( FILES find.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/menuext )
install( FILES kfind.desktop websearch.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/menuext/find )


##### kickermenu_find (module) ##################

tde_add_kpart( kickermenu_find AUTOMOC
  SOURCES findmenu.cpp
  LINK kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
