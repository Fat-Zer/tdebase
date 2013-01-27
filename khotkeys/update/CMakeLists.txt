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
  ${CMAKE_SOURCE_DIR}/khotkeys/shared
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### khotkeys_update (executable) ##############

tde_add_executable( khotkeys_update
  SOURCES update.cpp
  LINK khotkeys_shared-shared
  DESTINATION ${LIB_INSTALL_DIR}/tdeconf_update_bin
)
