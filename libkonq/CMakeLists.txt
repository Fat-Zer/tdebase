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

project( libkonq )


add_subdirectory( pics )
add_subdirectory( favicons )

add_definitions(
  -include tqt.h
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    konq_popupmenu.h knewmenu.h kfileivi.h konq_drag.h
    konq_iconviewwidget.h konq_defaults.h konq_settings.h
    konq_operations.h libkonq_export.h konq_dirpart.h
    konq_propsview.h konq_events.h konq_undo.h
    konq_historymgr.h konq_historycomm.h konq_pixmapprovider.h
    kivdirectoryoverlay.h konq_faviconmgr.h konq_xmlguiclient.h
    konqbookmarkmanager.h konq_filetip.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other data ################################

install( FILES directory_bookmarkbar.desktop DESTINATION ${DATA_INSTALL_DIR}/kbookmark )
install( FILES konqpopupmenuplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )


##### konq (shared lib) #########################

set( target konq )

set( ${target}_SRCS
  konq_popupmenu.cc knewmenu.cc konq_xmlguiclient.cc
  kfileivi.cc konq_iconviewwidget.cc konq_settings.cc
  konq_drag.cc konq_operations.cc konq_dirpart.cc
  konq_propsview.cc konq_events.cc konq_bgnddlg.cc
  konq_undo.cc konq_undo.skel konq_historymgr.cc
  konq_historycomm.cc konq_historycomm.skel konq_pixmapprovider.cc
  kivdirectoryoverlay.cc konq_faviconmgr.cc konq_faviconmgr.skel
  konq_filetip.cc
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 4.2.0
  LINK kparts-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### install import cmake modules ###############

tde_install_export( )