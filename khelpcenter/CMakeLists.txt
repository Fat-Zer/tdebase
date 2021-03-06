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

add_subdirectory( plugins )
add_subdirectory( pics )
add_subdirectory( searchhandlers )


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES Help.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES khelpcenter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES khelpcenter.kcfg DESTINATION ${KCFG_INSTALL_DIR} )

install( FILES
    khelpcenterui.rc intro.html.in glossary.html.in
    table-of-contents.xslt glossary.xslt index.html.in
  DESTINATION ${DATA_INSTALL_DIR}/khelpcenter )


##### khelpcenter (tdeinit) #####################

tde_add_tdeinit_executable( khelpcenter AUTOMOC
  SOURCES
    navigator.cpp navigatoritem.cpp navigatorappitem.cpp
    view.cpp searchwidget.cpp searchengine.cpp docmetainfo.cpp
    docentrytraverser.cpp formatter.cpp glossary.cpp
    toc.cpp mainwindow.cpp docentry.cpp htmlsearch.cpp
    history.cpp application.cpp searchwidget.skel
    treebuilder.cpp infotree.cpp  mainwindow.skel
    kcmhelpcenter.cpp htmlsearchconfig.cpp kcmhelpcenter.skel
    fontdialog.cpp plugintraverser.cpp scrollkeepertreebuilder.cpp
    prefs.kcfgc searchhandler.cpp
  LINK tdehtml-shared
)


##### khc_indexbuilder (executable) #############

tde_add_executable( khc_indexbuilder AUTOMOC
  SOURCES khc_indexbuilder.cpp
  LINK tdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
