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

add_subdirectory( twinoptions )
add_subdirectory( twindecoration )
add_subdirectory( twinrules )

install( FILES showdesktop.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
