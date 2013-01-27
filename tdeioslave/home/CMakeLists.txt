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

add_subdirectory( kdedmodule )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES home.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kiohome (static) ##########################

set( target kiohome )

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES kio_home.cpp homeimpl.cpp
)


##### kio_home (module) #########################

set( target kio_home )

tde_add_kpart( ${target}
  SOURCES dummy.cpp
  EMBED kiohome-static
  LINK tdeio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
