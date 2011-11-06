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
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### other data ################################

install( FILES kcm_kdnssd.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_kdnssd (module) #######################

set_source_files_properties( kcmdnssd.cpp PROPERTIES COMPILE_FLAGS -DKDE_CONFDIR=\\"${TDE_CONFIG_DIR}\\" )

tde_add_kpart( kcm_kdnssd AUTOMOC
  SOURCES configdialog.ui kcmdnssd.cpp
  LINK kdnssd-shared tdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
