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

add_subdirectory( themer )
add_subdirectory( themes )
add_subdirectory( pics )
add_subdirectory( sessions )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kdm/backend
  ${CMAKE_SOURCE_DIR}/kdmlib
  ${CMAKE_SOURCE_DIR}/kcontrol/background
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

if( NOT DEFINED GENKDMCONF_FLAGS )
  set( GENKDMCONF_FLAGS "--no-old" )
endif( )

install( CODE "execute_process( COMMAND ${CMAKE_CURRENT_BINARY_DIR}/genkdmconf --in \$ENV{DESTDIR}${CONFIG_INSTALL_DIR}/kdm --no-in-notice --face-src ${CMAKE_CURRENT_SOURCE_DIR}/pics ${GENKDMCONF_FLAGS} )" )


##### config.ci (generated) #####################

add_custom_command( OUTPUT config.ci
  COMMAND perl -w ${CMAKE_SOURCE_DIR}/kdm/confproc.pl ${CMAKE_SOURCE_DIR}/kdm/config.def config.ci
  DEPENDS ${CMAKE_SOURCE_DIR}/kdm/confproc.pl ${CMAKE_SOURCE_DIR}/kdm/config.def )


##### kdm_config (executable) ###################

set_property( SOURCE kdm_config.c APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/config.ci )

tde_add_executable( kdm_config
  SOURCES kdm_config.c
  LINK
  DESTINATION ${BIN_INSTALL_DIR}
)


##### kdm_greet (executable) ####################

tde_add_executable( kdm_greet AUTOMOC
  SOURCES
    kdm_greet.c kdmconfig.cpp kdmclock.cpp kconsole.cpp
    kfdialog.cpp kgdialog.cpp kchooser.cpp kgverify.cpp
    kdmshutdown.cpp kdmadmindialog.cpp kgreeter.cpp
    kgapp.cpp
  LINK kdmthemer-static kdeui-shared Xtst
  DESTINATION ${BIN_INSTALL_DIR}
)


##### krootimage (executable) ###################

tde_add_executable( krootimage AUTOMOC
  SOURCES krootimage.cpp
  LINK bgnd-static kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### genkdmconf (executable) ###################

set_property( SOURCE genkdmconf.c APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/config.ci )

tde_add_executable( genkdmconf AUTOMOC
  SOURCES genkdmconf.c
  LINK X11
  DESTINATION ${BIN_INSTALL_DIR}
)


##### kdmctl (executable) #######################

tde_add_executable( kdmctl
  SOURCES kdmctl.c
  LINK
  DESTINATION ${BIN_INSTALL_DIR}
)
