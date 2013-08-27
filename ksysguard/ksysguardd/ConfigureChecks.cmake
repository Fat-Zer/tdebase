#################################################
#
#  (C) 2013 Alexander Golubev
#  fatzer2 (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

# lm_sensors
if( WITH_SENSORS )
  check_include_file( "sensors/sensors.h" HAVE_SENSORS_SENSORS_H )
  check_library_exists( sensors sensors_init "" HAVE_SENSORS_LIB )
  if( HAVE_SENSORS_SENSORS_H AND HAVE_SENSORS_LIB )
    set( SENSORS_LIBRARIES sensors )
  else( )
    tde_message_fatal( "lm_sensors are required, but not found on your system" )
  endif( )
endif( WITH_SENSORS )
