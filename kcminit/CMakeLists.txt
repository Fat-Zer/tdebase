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


##### kcminit (tdeinit) #########################

tde_add_tdeinit_executable( kcminit AUTOMOC
  SOURCES main.cpp main.skel
  LINK kutils-shared
)


##### kcminit_startup (tdeinit) #################

tde_add_tdeinit_executable( kcminit_startup AUTOMOC
  SOURCES main.cpp main.skel
  LINK kutils-shared
)
