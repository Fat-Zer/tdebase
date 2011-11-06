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
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### kwin_killer_helper (executable) ###########

tde_add_executable( kwin_killer_helper
  SOURCES killer.cpp
  LINK tdeui-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
