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

install( FILES floppy.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### tdeio_floppy (module) #######################

set( target tdeio_floppy )

set( ${target}_SRCS
  tdeio_floppy.cpp program.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
