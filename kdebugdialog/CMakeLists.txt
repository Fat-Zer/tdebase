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


##### kdebugdialog (executable) #################

tde_add_executable( kdebugdialog AUTOMOC
  SOURCES
    main.cpp kabstractdebugdialog.cpp kdebugdialog.cpp
    klistdebugdialog.cpp
  LINK kdeui-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
