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
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES krandrtray.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES randr.desktop DESTINATION ${APPS_INSTALL_DIR}/.hidden )
install( FILES krandrtray-autostart.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )


##### kcm_randr (module) ########################

tde_add_kpart( kcm_randr AUTOMOC
  SOURCES krandrmodule.cpp
  LINK kdeui-shared krandr-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### krandrtray (executable) ###################

tde_add_executable( krandrtray AUTOMOC
  SOURCES
    main.cpp krandrtray.cpp krandrapp.cpp
    krandrpassivepopup.cpp configdialog.cpp
  LINK kutils-shared krandr-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
