#################################################
#
#  (C) 2013 Timothy Pearson
#  kb9vqf (AT) pearsoncomputing.net
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


##### tdekbdledsync (executable) ################

tde_add_executable( tdekbdledsync
  SOURCES getfd.c main.cpp
  LINK udev X11
  DESTINATION ${BIN_INSTALL_DIR}
)

install( CODE "execute_process( COMMAND chown 0 \$ENV{DESTDIR}${_destination}/tdekbdledsync )" )
install( CODE "execute_process( COMMAND chmod 4755 \$ENV{DESTDIR}${_destination}/tdekbdledsync )" )
