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

install( FILES iccconfig.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_iccconfig (module) ####################

set_source_files_properties( iccconfig.cpp PROPERTIES COMPILE_FLAGS -DKDE_CONFDIR=\\"${TDE_CONFIG_DIR}\\" )

tde_add_kpart( kcm_iccconfig AUTOMOC
  SOURCES
    iccconfig.cpp iccconfigbase.ui iccconfig.skel
  LINK tdeio-shared tderandr-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
