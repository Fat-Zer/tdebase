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

# FIXME initial work, only PAM
# FIXME needs more checks (for kerberos, etc)
# FIXME Xsession need some improvements

if( NOT DEFINED KDM_PAM_SERVICE )
  set( KDM_PAM_SERVICE "kde" CACHE INTERNAL "" )
endif( )

include( ConfigureChecks.cmake )

add_subdirectory( backend )
add_subdirectory( kfrontend )
