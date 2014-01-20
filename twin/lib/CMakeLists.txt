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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${DBUS_TQT_LIBRARY_DIRS}
)


##### tdecorations (shared) #####################

set( target tdecorations )

set( ${target}_SRCS
  kdecoration.cpp kdecoration_p.cpp kdecoration_plugins_p.cpp
  kdecorationfactory.cpp kcommondecoration.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK tdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}
)

##### headers ###################################

install( FILES kdecoration.h kdecoration_p.h kdecoration_plugins_p.h kdecorationfactory.h kcommondecoration.h
         DESTINATION ${INCLUDE_INSTALL_DIR} )
