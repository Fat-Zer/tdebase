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

add_subdirectory( apps )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_install_icons( kappfinder )
install( FILES kappfinder.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES uninstall.desktop DESTINATION ${APPS_INSTALL_DIR}/System RENAME kappfinder.desktop )


##### kappfinder (executable) ###################

tde_add_executable( kappfinder AUTOMOC
  SOURCES main.cpp toplevel.cpp common.cpp
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### kappfinder_install (executable) ###########

tde_add_executable( kappfinder_install AUTOMOC
  SOURCES main_install.cpp common.cpp
  LINK tdeui-shared
)
