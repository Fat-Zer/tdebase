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
  ${CMAKE_BINARY_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### ksplashsimple (executable) ################

tde_add_executable( ksplashsimple
  SOURCES main.cpp
  LINK ${TQT_LIBRARIES} X11 ${XINERAMA_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
