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

# FIXME no support for lm_sensors yet

add_subdirectory( CContLib )
add_subdirectory( ksysguardd )
add_subdirectory( gui )
add_subdirectory( pics )
add_subdirectory( example )

tde_install_icons( ksysguard )
