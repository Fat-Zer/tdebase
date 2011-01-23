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
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### other data ################################

install( FILES  DESTINATION ${DATA_INSTALL_DIR}/kicker/menuext )


##### appletproxy (kdeinit) #####################

tde_add_kdeinit_executable( appletproxy AUTOMOC
  SOURCES appletproxy.cpp
  LINK kickermain-shared
)


##### extensionproxy (kdeinit) ##################

tde_add_kdeinit_executable( extensionproxy AUTOMOC
  SOURCES extensionproxy.cpp
  LINK kickermain-shared
)
