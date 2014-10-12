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
  ${CMAKE_SOURCE_DIR}/ksysguard/CContLib
  ${CMAKE_SOURCE_DIR}/ksysguard/ksysguardd
)


##### ksysguardd (static) #######################

tde_add_library( ksysguardd STATIC
  SOURCES
    cpu.c memory.c
)
