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

tde_install_icons( kdeprintfax )
tde_install_icons( abentry DESTINATION ${DATA_INSTALL_DIR}/kdeprintfax/icons )
install( FILES kdeprintfax.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kdeprintfaxui.rc faxfilters DESTINATION ${DATA_INSTALL_DIR}/kdeprintfax )
install( PROGRAMS anytops DESTINATION ${DATA_INSTALL_DIR}/kdeprintfax )


##### kdeprintfax (executable) ##################

tde_add_executable( kdeprintfax AUTOMOC
  SOURCES
    main.cpp kdeprintfax.cpp faxab.cpp faxctrl.cpp
    confgeneral.cpp configdlg.cpp conffax.cpp
    confsystem.cpp conffilters.cpp filterdlg.cpp
    defcmds.cpp
  LINK kabc-shared kdeprint-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
