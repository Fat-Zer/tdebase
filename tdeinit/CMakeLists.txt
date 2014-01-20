#################################################
#
#  (C) 2010-2011 Timothy Pearson
#  kb9vqf (AT) pearsoncomputing.net
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${DBUS_TQT_LIBRARY_DIRS}
)


##### tdeinit_phase1 (executable) #######################

tde_add_executable( tdeinit_phase1
  SOURCES phase1.cpp
  LINK tdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### tdeinit_displayconfig (executable) #######################
if( WITH_XRANDR )
  set( TDEINIT_DISPLAYCONFIG_OPTIONAL_LINK "tderandr-shared" )
endif ( )

tde_add_executable( tdeinit_displayconfig
  SOURCES displayconfig.cpp
  LINK tdecore-shared ${TDEINIT_DISPLAYCONFIG_OPTIONAL_LINK}
  DESTINATION ${BIN_INSTALL_DIR}
)