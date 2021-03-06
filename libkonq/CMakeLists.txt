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

check_include_file( "sys/statvfs.h" HAVE_STATVFS )

add_subdirectory( pics )
add_subdirectory( favicons )
add_subdirectory( servicemenus )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
  ${ARTS_INCLUDE_DIRS}
  ${TDE_INCLUDE_DIR}/arts/
  ${CMAKE_BINARY_DIR}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${ARTS_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    konq_popupmenu.h knewmenu.h tdefileivi.h konq_drag.h
    konq_iconviewwidget.h konq_defaults.h konq_settings.h
    konq_operations.h libkonq_export.h konq_dirpart.h
    konq_propsview.h konq_events.h konq_undo.h
    konq_historymgr.h konq_historycomm.h konq_pixmapprovider.h
    kivdirectoryoverlay.h kivfreespaceoverlay.h konq_faviconmgr.h
    konq_xmlguiclient.h konqbookmarkmanager.h konq_filetip.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other data ################################

install( FILES directory_bookmarkbar.desktop DESTINATION ${DATA_INSTALL_DIR}/kbookmark )
install( FILES konqpopupmenuplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )


##### konq (shared lib) #########################

tde_add_library( konq SHARED AUTOMOC
  SOURCES
    konq_popupmenu.cc knewmenu.cc konq_xmlguiclient.cc tdefileivi.cc
    konq_iconviewwidget.cc konq_settings.cc konq_drag.cc konq_operations.cc
    konq_dirpart.cc konq_propsview.cc konq_events.cc konq_bgnddlg.cc konq_undo.cc
    konq_undo.skel konq_historymgr.cc konq_historycomm.cc konq_historycomm.skel
    konq_pixmapprovider.cc kivdirectoryoverlay.cc kivfreespaceoverlay.cc
    konq_faviconmgr.cc konq_faviconmgr.skel konq_filetip.cc
  VERSION 4.2.0
  LINK tdeparts-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### konq_sound (module) #######################

if( WITH_ARTS )
  tde_add_kpart( konq_sound
    SOURCES konq_sound.cc
    LINK artskde-shared
    DESTINATION ${PLUGIN_INSTALL_DIR} )
endif( )


##### install import cmake modules ###############

tde_install_export( )
