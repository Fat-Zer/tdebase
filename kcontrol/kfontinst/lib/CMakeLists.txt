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

add_definitions( -D_LARGEFILE64_SOURCE )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### kfontinst (shared) ########################

tde_add_library( kfontinst SHARED
  SOURCES Misc.cpp FcEngine.cpp
  VERSION 0.0.0
  LINK tdeio-shared ${XFT_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)
