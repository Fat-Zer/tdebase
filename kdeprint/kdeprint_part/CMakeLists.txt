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

add_definitions( -D_KDEPRINT_COMPILE )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES kdeprint_part.rc DESTINATION ${DATA_INSTALL_DIR}/kdeprint_part )
install( FILES kdeprint_part.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### libkdeprint_part (module) #################

tde_add_kpart( libkdeprint_part AUTOMOC
  SOURCES printpart.cpp
  LINK kdeprint_management-shared kparts-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
