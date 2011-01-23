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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### other data ################################

install( FILES arts.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_arts (module) #########################

tde_add_kpart( kcm_arts AUTOMOC
  SOURCES arts.cpp generaltab.ui hardwaretab.ui krichtextlabel.cpp
  LINK kio-shared kmid-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
