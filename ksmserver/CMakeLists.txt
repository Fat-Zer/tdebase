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

if( NOT DBUS_SYSTEM_BUS )
  set( DBUS_SYSTEM_BUS "unix:path=/var/run/dbus/system_bus_socket" CACHE INTERNAL "" FORCE )
endif()

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kdmlib
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
  ${DBUS_INCLUDE_DIRS}
  ${HAL_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES shutdownkonq.png DESTINATION ${DATA_INSTALL_DIR}/ksmserver/pics )
install( FILES ksmserver.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( FILES move_session_config.sh DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )


##### ksmserver (kdeinit) #######################

tde_add_kdeinit_executable( ksmserver AUTOMOC
  SOURCES
    main.cpp server.cpp shutdowndlg.cpp
    legacy.cpp startup.cpp shutdown.cpp client.cpp
    KSMServerInterface.skel server.skel timed.ui
  LINK dmctl-static kdeui-shared ${HAL_LIBRARIES} ${DBUS_LIBRARIES}
)