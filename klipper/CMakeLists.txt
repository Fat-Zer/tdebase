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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### other data ################################

tde_install_icons( klipper )
install( FILES klipper.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES klipper.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )
install( FILES klipper.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )
install( FILES klipperrc.desktop DESTINATION ${CONFIG_INSTALL_DIR} RENAME klipperrc )
install( FILES klipperrc.upd klippershortcuts.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS klipper-1-2.pl klipper-trinity1.sh DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )


##### klipper (executable) ######################

configure_file( ${CMAKE_SOURCE_DIR}/cmake/modules/template_dummy_cpp.cmake dummy.cpp COPYONLY )

tde_add_tdeinit_executable( klipper AUTOMOC
  SOURCES dummy.cpp
  LINK ${TQT_LIBRARIES}
  EMBED klipper_common-static
)


##### klipper_panelapplet (module) ##############

tde_add_kpart( klipper_panelapplet AUTOMOC
  SOURCES applet.cpp applet.skel
  LINK ${TQT_LIBRARIES} klipper_common-static
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### klipper_common (static) ###################

tde_add_library( klipper_common STATIC_PIC AUTOMOC
  SOURCES
    main.cpp toplevel.cpp urlgrabber.cpp configdialog.cpp
    toplevel.skel clipboardpoll.cpp history.cpp historyitem.cpp
    historystringitem.cpp klipperpopup.cpp popupproxy.cpp
    historyimageitem.cpp historyurlitem.cpp
  LINK tdeio-shared ${XFIXES_LIBRARIES}
)
