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

tde_install_icons( )
install( FILES knetattach.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### knetattach (executable) ###################

tde_add_executable( knetattach AUTOMOC
  SOURCES knetattach.ui main.cpp
  LINK tdeio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
