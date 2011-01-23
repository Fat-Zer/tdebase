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

add_subdirectory( pics )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_install_icons( )
install( FILES ktip.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES ktip.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )
install( FILES uninstall.desktop DESTINATION ${APPS_INSTALL_DIR}/Toys RENAME ktip.desktop )
install( FILES tips DESTINATION ${DATA_INSTALL_DIR}/kdewizard )


##### ktip (executable) #########################

tde_add_executable( ktip AUTOMOC
  SOURCES ktipwindow.cpp
  LINK kdeui-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
