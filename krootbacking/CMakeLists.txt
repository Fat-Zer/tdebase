#################################################
#
#  (C) 2011 Timothy Pearson
#  kb9vqf (AT) pearsoncomputing (DOT) net
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
)

##### krootbacking (executable) ####################

tde_add_executable( krootbacking AUTOMOC
  SOURCES
    main.cpp krootbacking.cpp
  LINK kdeui-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
