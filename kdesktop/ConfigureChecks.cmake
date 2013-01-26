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

# FIXME: KSCREENSAVER_PAM_SERVICE should be "kde" or "tdescreensaver"?

if( WITH_PAM AND (NOT DEFINED KSCREENSAVER_PAM_SERVICE) )
  set( KSCREENSAVER_PAM_SERVICE "kde" CACHE INTERNAL "" )
endif( )
