#################################################
#
#  (C) 2014 Timothy Pearson
#  kb9vqf (AT) pearsoncomputing (DOT) net
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################


if( NOT WITH_XRENDER )
  tde_message_fatal( "xrender support is needed to build compton-tde.\n Pass -DWITH_XRENDER=ON to cmake arguments." )
elseif( NOT WITH_XRANDR )
  tde_message_fatal( "xrandr support is needed to build compton-tde.\n Pass -DWITH_XFIXES=ON to cmake arguments." )
elseif( NOT WITH_XFIXES )
  tde_message_fatal( "xfixes support is needed to build compton-tde.\n Pass -DWITH_XFIXES=ON to cmake arguments." )
elseif( NOT WITH_XDAMAGE )
  tde_message_fatal( "xdamage support is needed to build compton-tde.\n Pass -DWITH_XDAMAGE=ON to cmake arguments." )
elseif( NOT WITH_XEXT )
  tde_message_fatal( "xext support is needed to build compton-tde.\n Pass -DWITH_XEXT=ON to cmake arguments." )
elseif( NOT WITH_XINERAMA )
  tde_message_fatal( "xinerama support is needed to build compton-tde.\n Pass -DWITH_XEXT=ON to cmake arguments." )
endif( )


include_directories(
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### compton-tde (executable) ###################

add_definitions("-Wall" "-std=c99")

option(CONFIG_LIBCONFIG "Enable configuration file parsing using libconfig" ON)
if (CONFIG_LIBCONFIG)
	add_definitions("-DCONFIG_LIBCONFIG")
endif ()

option(CONFIG_VSYNC_DRM "Enable DRM VSync support" ON)
if (CONFIG_VSYNC_DRM)
	add_definitions("-DCONFIG_LIBCONFIG")
endif ()

option(CONFIG_VSYNC_OPENGL "Enable OpenGL support" ON)
if (CONFIG_VSYNC_OPENGL)
	add_definitions("-DCONFIG_VSYNC_OPENGL")
	list(APPEND compton_SRCS src/opengl.c)
endif ()

option(CONFIG_XINERAMA "Enable additional Xinerama features" ON)
if (CONFIG_XINERAMA)
	add_definitions("-DCONFIG_XINERAMA")
endif ()

option(CONFIG_C2 "Enable matching system" ON)
if (CONFIG_C2)
	add_definitions("-DCONFIG_C2")
	list(APPEND compton_SRCS src/c2.c)
endif ()

tde_add_executable( compton-tde
  SOURCES c2.c compton.c opengl.c
  LINK m GL config Xinerama ${XRENDER_LIBRARIES} ${XRANDR_LIBRARIES} ${XFIXES_LIBRARIES} ${XDAMAGE_LIBRARIES} ${XEXT_LIBRARIES} ${XCOMPOSITE_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
