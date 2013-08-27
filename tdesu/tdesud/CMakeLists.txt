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


##### tdesud (executable) #######################

tde_add_executable( tdesud
  SOURCES tdesud.cpp repo.cpp lexer.cpp handler.cpp secure.cpp
  LINK tdesu-shared
  DESTINATION ${BIN_INSTALL_DIR}
)

install( CODE "execute_process( COMMAND chown root:${nogroup} \$ENV{DESTDIR}${_destination}/tdesud )" )
install( CODE "execute_process( COMMAND chmod 2755 \$ENV{DESTDIR}${_destination}/tdesud )" )
