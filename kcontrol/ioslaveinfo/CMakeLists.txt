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

install( FILES ioslaveinfo.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
tde_install_empty_directory( ${DATA_INSTALL_DIR}/Settings/Information )


##### kcm_ioslaveinfo (module) ##################

tde_add_kpart( kcm_ioslaveinfo AUTOMOC
  SOURCES kcmioslaveinfo.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
