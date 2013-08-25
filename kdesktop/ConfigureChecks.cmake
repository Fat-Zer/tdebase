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

if( WITH_PAM AND (NOT DEFINED TDESCREENSAVER_PAM_SERVICE) )
  set( TDESCREENSAVER_PAM_SERVICE "kde" CACHE INTERNAL "" )
endif( )
