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
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### ksystraycmd (executable) ##################

tde_add_executable( ksystraycmd AUTOMOC
  SOURCES ksystraycmd.cpp main.cpp
  LINK kdeui-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
