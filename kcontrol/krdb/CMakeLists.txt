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

add_subdirectory( themes )
add_subdirectory( kcs )
add_subdirectory( ad )
add_subdirectory( pics )

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

install( FILES kcmdisplayrc.upd DESTINATION ${DATA_INSTALL_DIR}/tdeconf_update )


##### krdb (static) #############################

tde_add_library( krdb STATIC_PIC AUTOMOC
  SOURCES krdb.cpp
  LINK tdecore-shared
)
