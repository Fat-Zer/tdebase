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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kcontrol/kfontinst/lib
  ${CMAKE_SOURCE_DIR}/kcontrol/kfontinst/viewpart
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_install_icons()
install( FILES kcmfontinst.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_fontinst (module) #####################

tde_add_kpart( kcm_fontinst AUTOMOC
  SOURCES
    KCmFontInst.cpp KFileFontIconView.cpp KFileFontView.cpp
    PrintDialog.cpp SettingsDialog.cpp
  LINK kfontinstprint-static kfontinst-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
