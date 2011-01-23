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


##### other data ################################

install( FILES kwebdesktop.desktop DESTINATION ${DATA_INSTALL_DIR}/kdesktop/programs )
install( FILES kwebdesktop.kcfg DESTINATION ${KCFG_INSTALL_DIR} )


##### kwebdesktop (executable) ##################

set( target kwebdesktop )

set( ${target}_SRCS
  kwebdesktop.cpp kwebdesktopsettings.kcfgc
)

tde_add_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK khtml-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
