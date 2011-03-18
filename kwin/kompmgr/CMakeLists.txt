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

##### check for xcomposite ######################

if( WITH_XCOMPOSITE )

  pkg_search_module( XCOMPOSITE xcomposite )
  if( XCOMPOSITE_FOUND )
    set( HAVE_XCOMPOSITE 1 CACHE INTERNAL "" )
  else( XCOMPOSITE_FOUND )
    message(FATAL_ERROR "\nxcomposite are requested, but not found on your system" )
  endif( XCOMPOSITE_FOUND )

  pkg_search_module( XFIXES xfixes )
  if( XFIXES_FOUND )
    set( HAVE_XFIXES 1 CACHE INTERNAL "" )
  else( XFIXES_FOUND )
    message(FATAL_ERROR "\nxfixes are required, but not found on your system" )
  endif( XFIXES_FOUND )

  pkg_search_module( XDAMAGE xdamage )
  if( XDAMAGE_FOUND )
    set( HAVE_XDAMAGE 1 CACHE INTERNAL "" )
  else( XDAMAGE_FOUND )
    message(FATAL_ERROR "\nxdamage are required, but not found on your system" )
  endif( XDAMAGE_FOUND )

endif( WITH_XCOMPOSITE )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### kompmgr (executable) #######################

tde_add_executable( kompmgr AUTOMOC
  SOURCES kompmgr.c
  LINK m X11 ${XFIXES_LIBRARIES} ${XRENDER_LIBRARIES} ${XCOMPOSITE_LIBRARIES} ${XDAMAGE_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
