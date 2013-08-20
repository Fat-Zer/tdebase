project( kate )

add_subdirectory( utils )
add_subdirectory( interfaces )
add_subdirectory( app )
add_subdirectory( data )
add_subdirectory( pics )


##### install import cmake modules ###############

tde_install_export( )
