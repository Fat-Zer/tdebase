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

add_subdirectory( lib )
add_subdirectory( thumbnail )
add_subdirectory( viewpart )
add_subdirectory( kcmfontinst )
add_subdirectory( tdefile-plugin )
add_subdirectory( tdefontinst )
add_subdirectory( tdeio )

install( FILES installfont.desktop DESTINATION ${DATA_INSTALL_DIR}/konqueror/servicemenus )
