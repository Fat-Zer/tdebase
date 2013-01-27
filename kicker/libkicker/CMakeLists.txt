#################################################
#
#  (C) 2010-2011 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
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

install( FILES kickerSettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )


##### kickermain (shared) #######################

set( target kickermain )

set( ${target}_SRCS
  appletinfo.cpp global.cpp kickertip.cpp menuinfo.cpp
  panelbutton.cpp panner.cpp kickerSettings.kcfgc
  kshadowsettings.cpp kshadowengine.cpp paneldrag.cpp
  simplebutton.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK tdeio-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
