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
  ${CMAKE_BINARY_DIR}/kicker/libkicker
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${CMAKE_BINARY_DIR}/kicker/libkicker
)


##### other data ################################

install( FILES clockapplet.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )
install( FILES lcd.png DESTINATION ${DATA_INSTALL_DIR}/clockapplet/pics )


##### clock_panelapplet (static) ######################

set( target clock_panelapplet )

set( ${target}_SRCS
  clock.skel clock.cpp datepicker.cpp zone.cpp
  analog.ui digital.ui fuzzy.ui settings.ui prefs.kcfgc
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)

##### clock_panelapplet (module) ################

set( target clock_panelapplet )

set( ${target}_SRCS
  init.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  DEPENDENCIES clock_panelapplet-static
  LINK kickermain-shared clock_panelapplet-static kickermain
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
