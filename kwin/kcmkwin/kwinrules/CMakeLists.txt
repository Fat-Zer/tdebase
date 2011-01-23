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

add_definitions( -DKCMRULES )

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

install( FILES kwinrules.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_kwinrules (module) ####################

tde_add_kpart( kcm_kwinrules AUTOMOC
  SOURCES kcm.cpp
  LINK kwinrules-static
  DESTINATION ${PLUGIN_INSTALL_DIR} )


##### kwin_rules_dialog (kdeinit) ###############

tde_add_kdeinit_executable( kwin_rules_dialog
  SOURCES main.cpp
  LINK kwinrules-static )


##### kwinrules (static) ########################

set( target kwinrules )

set( ${target}_SRCS
  ruleswidget.cpp ruleslist.cpp kwinsrc.cpp detectwidget.cpp
  ruleswidgetbase.ui ruleslistbase.ui detectwidgetbase.ui
  editshortcutbase.ui )

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kdeui-shared )
