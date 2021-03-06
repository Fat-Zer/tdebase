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

# FIXME there is only Linux support

include( ConfigureChecks.cmake )

if( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  set( OS_SPECIFIC_DIR Linux )
elseif( ${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD" )
  set( OS_SPECIFIC_DIR OpenBSD )
elseif( ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD" )
  set( OS_SPECIFIC_DIR FreeBSD )
else()
  tde_message_fatal( "Your operating system (${CMAKE_SYSTEM_NAME}) is not supported yet." )
endif()

add_subdirectory( ${OS_SPECIFIC_DIR} )


add_definitions(
  -DKSYSGUARDDRCFILE=\"${SYSCONF_INSTALL_DIR}/ksysguarddrc\"
  -DOSTYPE_${CMAKE_SYSTEM_NAME}
)


include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/${OS_SPECIFIC_DIR}
  ${CMAKE_SOURCE_DIR}/ksysguard/CContLib
)


##### ksysguardd (executable) ###################

tde_add_executable( ksysguardd AUTOMOC
  SOURCES Command.c conf.c ksysguardd.c PWUIDCache.c
  LINK ccont-static ksysguardd-static ${TDE_LIB_DIR}/libtdefakes_nonpic.a
    ${SENSORS_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
