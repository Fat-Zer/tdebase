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

add_subdirectory( Templates )

install( FILES
    directory.autostart directory.templates directory.desktop
  DESTINATION ${DATA_INSTALL_DIR}/kdesktop )

install( FILES
    My_Computer My_Documents My_Network_Places Printers Trash Web_Browser
  DESTINATION ${DATA_INSTALL_DIR}/kdesktop/Desktop )

install( FILES
    Home.desktop System.desktop
  DESTINATION ${DATA_INSTALL_DIR}/kdesktop/DesktopLinks )
