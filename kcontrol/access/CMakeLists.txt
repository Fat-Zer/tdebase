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

install( FILES kcmaccess.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kaccess.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES eventsrc DESTINATION ${DATA_INSTALL_DIR}/kaccess )


##### kcm_access (module) #######################

tde_add_kpart( kcm_access AUTOMOC
  SOURCES kcmaccess.cpp
  LINK tdeio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kaccess (tdeinit) #########################

tde_add_tdeinit_executable( kaccess AUTOMOC
  SOURCES kaccess.cpp main.cpp
  LINK tdeui-shared
)
