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
  ${CMAKE_BINARY_DIR}/kcontrol/background
  ${CMAKE_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### other data ################################

install( FILES kdm.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_kdm (module) ##########################

set_source_files_properties( background.cpp PROPERTIES COMPILE_FLAGS -DKDE_CONFDIR=\\"${TDE_CONFIG_DIR}\\" )
set_source_files_properties( main.cpp PROPERTIES COMPILE_FLAGS -DKDE_CONFDIR=\\"${TDE_CONFIG_DIR}\\" )

tde_add_kpart( kcm_kdm AUTOMOC
  SOURCES
    background.cpp kdm-appear.cpp kdm-font.cpp kdm-shut.cpp
    kdm-users.cpp kdm-conv.cpp main.cpp kbackedcombobox.cpp
  LINK kcmbgnd-static bgnd-static knewstuff-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
