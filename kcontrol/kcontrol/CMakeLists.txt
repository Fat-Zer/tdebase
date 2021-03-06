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

add_subdirectory( about )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_install_icons()
install( FILES kcontrolui.rc DESTINATION ${DATA_INSTALL_DIR}/kcontrol )
install( FILES  KControl.desktop tde-kcontrol.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcontrol (tdeinit) ########################

set( target kcontrol )

set( ${target}_SRCS
  main.cpp toplevel.cpp indexwidget.cpp helpwidget.cpp
  dockcontainer.cpp aboutwidget.cpp moduletreeview.cpp
  moduleiconview.cpp moduleIface.cpp moduleIface.skel
  global.cpp modules.cpp proxywidget.cpp kcrootonly.cpp
  searchwidget.cpp
)

tde_add_tdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdehtml-shared
)
