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

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES nfs.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kio_nfs (module) ##########################

set( target kio_nfs )

add_custom_command( OUTPUT mount_xdr.c
  COMMAND rpcgen -c -o mount_xdr.c ${CMAKE_CURRENT_SOURCE_DIR}/mount.x
  DEPENDS mount.x )

add_custom_command( OUTPUT nfs_prot_xdr.c
  COMMAND rpcgen -c -o nfs_prot_xdr.c ${CMAKE_CURRENT_SOURCE_DIR}/nfs_prot.x
  DEPENDS nfs_prot.x )

tde_add_kpart( ${target} AUTOMOC
  SOURCES kio_nfs.cpp mount_xdr.c nfs_prot_xdr.c
  LINK tdeio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
