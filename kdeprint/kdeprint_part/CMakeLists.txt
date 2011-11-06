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

install( FILES tdeprint_part.rc DESTINATION ${DATA_INSTALL_DIR}/tdeprint_part )
install( FILES tdeprint_part.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### libtdeprint_part (module) #################

tde_add_kpart( libtdeprint_part AUTOMOC
  SOURCES printpart.cpp
  LINK tdeprint_management-shared kparts-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
