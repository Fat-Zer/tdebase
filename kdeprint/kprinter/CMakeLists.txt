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

include( CheckFunctionExists )
check_function_exists( sigset HAVE_SIGACTION )
check_function_exists( sigaction HAVE_SIGSET )

add_definitions( -D_KDEPRINT_COMPILE )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### kprinter (tdeinit) ########################

tde_add_tdeinit_executable( kprinter AUTOMOC
  SOURCES main.cpp printwrapper.cpp
  LINK tdeprint-shared
)
