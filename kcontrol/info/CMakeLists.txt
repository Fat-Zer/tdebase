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

# FIXME no OPENGL support yet

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

install( FILES
    memory.desktop processor.desktop dma.desktop
    interrupts.desktop ioports.desktop opengl.desktop
    pci.desktop sound.desktop devices.desktop scsi.desktop
    partitions.desktop xserver.desktop cdinfo.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_info (module) #########################

tde_add_kpart( kcm_info AUTOMOC
  SOURCES main.cpp memory.cpp opengl.cpp
  LINK tdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
