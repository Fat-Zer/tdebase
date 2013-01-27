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
  ${CMAKE_BINARY_DIR}/tdeioslave/media/libmediacommon
  ${CMAKE_SOURCE_DIR}/tdeioslave/media/libmediacommon
  ${CMAKE_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
  ${HAL_INCLUDE_DIRS}
  ${DBUS_TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${DBUS_TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES mediamanager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )


##### kded_mediamanager (module) ################

set( target kded_mediamanager )

if( WITH_HAL )
  set( ${target}_SRCS
    mediamanager.cpp mediamanager.skel medialist.cpp
    backendbase.cpp fstabbackend.cpp removablebackend.cpp
    mediadirnotify.cpp mediadirnotify.skel
    decryptdialog.ui dialog.cpp
    halbackend.cpp linuxcdpolling.cpp tdehardwarebackend.cpp
  )

  tde_add_kpart( ${target} AUTOMOC
    SOURCES ${${target}_SRCS}
    LINK mediacommon-static tdeinit_kded-shared ${HAL_LIBRARIES} -lhal-storage ${DBUS_TQT_LIBRARIES}
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )
else( )
  set( ${target}_SRCS
    mediamanager.cpp mediamanager.skel medialist.cpp
    backendbase.cpp fstabbackend.cpp removablebackend.cpp
    decryptdialog.ui dialog.cpp
    mediadirnotify.cpp mediadirnotify.skel tdehardwarebackend.cpp
  )

  tde_add_kpart( ${target} AUTOMOC
    SOURCES ${${target}_SRCS}
    LINK mediacommon-static tdeinit_kded-shared
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )
endif( )
