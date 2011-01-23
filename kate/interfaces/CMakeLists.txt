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
  ${CMAKE_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### headers ###################################

install( FILES
    application.h documentmanager.h mainwindow.h
    plugin.h viewmanager.h pluginconfiginterface.h
    pluginconfiginterfaceextension.h toolviewmanager.h
    pluginmanager.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kate )


##### kateinterfacesprivate (static lib) ########

set( target kateinterfacesprivate )

set( ${target}_SRCS
  application.cpp mainwindow.cpp documentmanager.cpp
  viewmanager.cpp toolviewmanager.cpp pluginmanager.cpp
  plugin.cpp pluginconfiginterface.cpp
  pluginconfiginterfaceextension.cpp
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK katepartinterfaces-shared
)
