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
  ${CMAKE_SOURCE_DIR}/libkonq
  ${CMAKE_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TDE_LIBRARY_DIRS}
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES keditbookmarksui.rc keditbookmarks-genui.rc DESTINATION ${DATA_INSTALL_DIR}/keditbookmarks )
install( FILES uninstall.desktop DESTINATION ${APPS_INSTALL_DIR}/Internet/keditbookmarks.desktop )
install( FILES keditbookmarks.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
tde_install_icons( )


##### kbookmarkmerger ###########################

set( target kbookmarkmerger )

tde_add_executable( ${target}
  SOURCES kbookmarkmerger.cpp
  LINK tdeio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### keditbookmarks (tdeinit) ##################

set( target keditbookmarks )

set( ${target}_SRCS AUTOMOC
  main.cpp listview.cpp toplevel.cpp actionsimpl.cpp
  commands.cpp importers.cpp dcop.skel dcop.cpp
  bookmarkiterator.cpp testlink.cpp favicons.cpp
  updater.cpp exporters.cpp ${TDE_INCLUDE_DIR}/kbookmarknotifier.stub
  bookmarkinfo.cpp kebsearchline.cpp settings.kcfgc
)

set( KDE3_DCOPIDL_EXECUTABLE ${KDE3_DCOPIDLNG_EXECUTABLE} )

tde_add_tdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK konq-shared
)
