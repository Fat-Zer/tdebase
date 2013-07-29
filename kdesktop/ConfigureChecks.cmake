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

if( WITH_KDESKTOP_LOCK_BACKTRACE )
  check_include_files( "bfd.h;demangle.h;libiberty.h" HAVE_BINUTILS_DEV )
  if( NOT HAVE_BINUTILS_DEV )
    tde_message_fatal( "binutils-dev are required, but not found on your system" )
  endif( )
endif( )
