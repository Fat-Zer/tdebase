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

add_definitions( -D_TDEPRINT_COMPILE )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_install_icons( kjobviewer )
install( FILES kjobviewer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kjobviewerui.rc DESTINATION ${DATA_INSTALL_DIR}/kjobviewer )


##### kjobviewer (tdeinit) ######################

tde_add_tdeinit_executable( kjobviewer AUTOMOC
  SOURCES kjobviewer.cpp main.cpp
  LINK tdeprint_management-shared
)
