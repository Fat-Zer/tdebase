#################################################
#
#  (C) 2010-2011 Timothy Pearson
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


##### tdeinit_phase1 (executable) #######################

tde_add_executable( tdeinit_phase1
  SOURCES phase1.cpp
  LINK udev
  DESTINATION ${BIN_INSTALL_DIR}
)
