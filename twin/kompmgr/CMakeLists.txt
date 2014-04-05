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


if( NOT WITH_XRENDER )
  tde_message_fatal( "xrender support is needed to build kompmgr.\n Pass -DWITH_XRENDER=ON to cmake arguments." )
elseif( NOT WITH_XFIXES )
  tde_message_fatal( "xfixes support is needed to build kompmgr.\n Pass -DWITH_XFIXES=ON to cmake arguments." )
endif( )


include_directories(
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### kompmgr (executable) #######################

tde_add_executable( kompmgr
  SOURCES kompmgr.c
  LINK m ${XRENDER_LIBRARIES} ${XFIXES_LIBRARIES} ${XDAMAGE_LIBRARIES} ${XEXT_LIBRARIES} ${XCOMPOSITE_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
