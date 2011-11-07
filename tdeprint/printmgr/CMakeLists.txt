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

tde_install_icons( printmgr )
install( FILES printers.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_printmgr (module) #####################

tde_add_kpart( kcm_printmgr AUTOMOC
  SOURCES kcmprintmgr.cpp
  LINK tdeprint_management-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
