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

if( WITH_SASL )
  check_include_file( "sasl/sasl.h" HAVE_SASL_SASL_H )
  if( NOT HAVE_SASL_SASL_H )
    find_path( SASL_H_PATH "sasl/sasl.h" )
    if( SASL_H_PATH )
      set( HAVE_SASL_SASL_H "1" )
    endif( )
  endif( )
  check_library_exists( sasl2 sasl_client_init "" HAVE_LIBSASL2 )
  if( HAVE_SASL_SASL_H AND HAVE_LIBSASL2 )
    set( SASL_LIBRARIES sasl2 )
  else( )
    tde_message_fatal( "sasl2 are requested, but not found on your system" )
  endif( )
endif( )
