#################################################
#
#  (C) 2010-2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#################################################

cmake_minimum_required( VERSION 2.8 )


##### general package setup #####################

project( tdebase )

set( PACKAGE tdebase )
set( VERSION "R14" )


##### include essential cmake modules ###########

include( FindPkgConfig )
include( CheckIncludeFile )
include( CheckCSourceRuns )
include( CheckCXXSourceRuns )
include( CheckCXXSourceCompiles )
include( CheckLibraryExists )
include( CheckFunctionExists )
include( CheckStructHasMember )
include( CheckTypeSize )


##### include our cmake modules #################

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" )
include( TDEMacros )


##### setup install paths #######################

include( TDESetupPaths )
tde_setup_paths( )


##### add apidox targets ############

add_custom_target(apidox
  COMMAND "./generate_apidox" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" "${HTML_INSTALL_DIR}" "/usr/share/qt3/doc/html"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake/")

add_custom_target(install-apidox
  COMMAND "./install_apidox" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" "${HTML_INSTALL_DIR}" "${CMAKE_INSTALL_PREFIX}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake/")


##### optional stuff ############################

option( WITH_ALL_OPTIONS "Enable all optional support" OFF )
option( WITH_SASL "Enable SASL support" ${WITH_ALL_OPTIONS} )
option( WITH_LDAP "Enable LDAP support" ${WITH_ALL_OPTIONS} )
option( WITH_SAMBA "Enable SAMBA support" ${WITH_ALL_OPTIONS} )
option( WITH_OPENEXR "Enable openexr support" ${WITH_ALL_OPTIONS} )
option( WITH_XCOMPOSITE "Enable xcomposite support" ${WITH_ALL_OPTIONS} )
option( WITH_XCURSOR "Enable xcursor support" ${WITH_ALL_OPTIONS} )
option( WITH_XFIXES "Enable xfixes support" ${WITH_ALL_OPTIONS} )
option( WITH_XRANDR "Enable xrandr support" ${WITH_ALL_OPTIONS} )
option( WITH_XRENDER "Enable xrender support" ${WITH_ALL_OPTIONS} )
option( WITH_XDAMAGE "Enable xdamage support" ${WITH_ALL_OPTIONS} )
option( WITH_XEXT "Enable xext support" ${WITH_ALL_OPTIONS} )
option( WITH_XTEST "Enable xtest support" ${WITH_ALL_OPTIONS} )
option( WITH_XSCREENSAVER "Enable xscreensaver support" ${WITH_ALL_OPTIONS} )
option( WITH_LIBART "Enable libart support (for SVG icons and wallpapers)" ${WITH_ALL_OPTIONS} )
option( WITH_LIBUSB "Enable control of some mouse models through libusb" ${WITH_ALL_OPTIONS} )
option( WITH_LIBRAW1394 "Enable visualization of ieee1394 devices through libraw1394" ${WITH_ALL_OPTIONS} )
option( WITH_SUDO_TDESU_BACKEND "Use sudo as backend for tdesu (default is su)" ${WITH_ALL_OPTIONS} )
option( WITH_PAM "tdm should use PAM" ${WITH_ALL_OPTIONS} )
option( WITH_SHADOW "tdm should use shadow passwords" ${WITH_ALL_OPTIONS} )
option( WITH_XDMCP "xdmcp support for tdm" ${WITH_ALL_OPTIONS} )
option( WITH_XINERAMA "Enable xinerama extension support" ${WITH_ALL_OPTIONS} )
option( WITH_ARTS "Enable aRts support" ${WITH_ALL_OPTIONS} )
option( WITH_I8K "Enable Dell laptop support (ksysguard)" ${WITH_ALL_OPTIONS} )
option( WITH_HAL "Enable HAL support" ${WITH_ALL_OPTIONS} )
option( WITH_TDEHWLIB "Enable TDE hardware library support" OFF )
option( WITH_UPOWER "Enable UPOWER support" ${WITH_ALL_OPTIONS} )
option( WITH_GCC_VISIBILITY "Enable fvisibility and fvisibility-inlines-hidden" ${WITH_ALL_OPTIONS} )

##### options comments ##########################

# WITH_SASL         affects kioslaves(kio_ldap kio_smtp kio_pop3)
# WITH_LDAP         affects kioslaves(kio_ldap)
# WITH_SAMBA        affects kioslaves(kio_samba) kcontrol
# WITH_OPENEXR      affects kioslaves(thumbnail)
# WITH_XCOMPOSITE   affects twin kicker(taskmanager) tdm
# WITH_XCURSOR      affects kioslaves(thumbnail), kcontrol(input), kdesktop
# WITH_XFIXES       affects twin klipper kicker(taskmanager)
# WITH_XDAMAGE      affects <nothing>
# WITH_XEXT         affects <nothing>
# WITH_XRANDR       affects tdm, kcontrol(displayconfig randr iccconfig)
# WITH_XRENDER      affects twin konsole kicker(taskmanager) kdesktop
#                           kcontrol(style)
# WITH_XTEST        desription adds some hotkey functonality. Its functionality
#                              replaced by xkb nearly everywhere except the
#                              mouse gestures. Also it used as a fallback if
#                              xkb initialization fails.
# WITH_XTEST        affects tdm kxkb khotkeys
# WITH_XSCREENSAVER affects kdesktop
# WITH_LIBART       affects tdm kcontrol(icons background)
# WITH_LIBUSB       affects kcontrol(input)
# WITH_XDMCP        affects tdm
# WITH_XINERAMA     affects ksplashml
# WITH_ARTS         affects libkonq khotkeys
# WITH_I8K          affects ksysguard
# WITH_HAL          affects kioslaves(media) ksmserver
# WITH_TDEHWLIB     affects kioslaves(media), replaces WITH_HAL on systems without HAL
# WITH_LIBRAW1394   affects kcontrol(view1394)
# WITH_PAM          affects tdm kdesktop kcheckpass
# WITH_SHADOW       affects tdm kcheckpass
# WITH_UPOWER       affects ksmserver
# WITH_SUDO_TDESU_BACKEND affects tdesu

# NOTE: WITH_SHADOW and WITH_PAM shouldn't be set concurrently
#       WITH_PAM will override WITH_SHADOW
# NOTE: if WITH_XCOMPOSITE=ON then the following options are also required:
#       WITH_XRENDER=ON
#       WITH_XFIXES=ON
#       WITH_XDAMAGE=ON
#       WITH_XEXT=ON
# NOTE: seems WITH_XDAMAGE and WITH_XET doesn't change anything
#       they are just required by WITH_XCOMPOSITE

##### user requested modules ####################

option( BUILD_ALL "Build all" OFF )
option( BUILD_TDEINIT "Build tdeinit" ${BUILD_ALL} )
option( BUILD_L10N "Build l10n" ${BUILD_ALL} )
option( BUILD_PICS "Build pics" ${BUILD_ALL} )
option( BUILD_APPLNK "Build applnk" ${BUILD_ALL} )
option( BUILD_DOC "Build doc" ${BUILD_ALL} )
option( BUILD_LIBKONQ "Build libkonq" ${BUILD_ALL} )
option( BUILD_KCMINIT "Build kcminit" ${BUILD_ALL} )
option( BUILD_KCONTROL "Build kcontrol" ${BUILD_ALL} )
option( BUILD_TWIN "Build twin" ${BUILD_ALL} )
option( BUILD_KATE "Build kate" ${BUILD_ALL} )
option( BUILD_KONQUEROR "Build konqueror" ${BUILD_ALL} )
option( BUILD_KIOSLAVES "Build kioslaves" ${BUILD_ALL} )
option( BUILD_KICKER "Build kicker" ${BUILD_ALL} )
option( BUILD_KDESKTOP "Build kdesktop" ${BUILD_ALL} )
option( BUILD_KONSOLE "Build konsole" ${BUILD_ALL} )
option( BUILD_KLIPPER "Build klipper" ${BUILD_ALL} )
option( BUILD_KMENUEDIT "Build kmenuedit" ${BUILD_ALL} )
option( BUILD_TDEPRINT "Build tdeprint" ${BUILD_ALL} )
option( BUILD_KPERSONALIZER "Build kpersonalizer" ${BUILD_ALL} )
option( BUILD_KDEPASSWD "Build kdepasswd" ${BUILD_ALL} )
option( BUILD_KTIP "Build ktip" ${BUILD_ALL} )
option( BUILD_KDEEJECT "Build kdeeject" ${BUILD_ALL} )
option( BUILD_KDIALOG "Build kdialog" ${BUILD_ALL} )
option( BUILD_KREADCONFIG "Build kreadconfig" ${BUILD_ALL} )
option( BUILD_TDESU "Build tdesu" ${BUILD_ALL} )
option( BUILD_TDM "Build tdm" ${BUILD_ALL} )
option( BUILD_KSMSERVER "Build ksmserver" ${BUILD_ALL} )
option( BUILD_KSPLASHML "Build ksplashml" ${BUILD_ALL} )
option( BUILD_STARTTDE "Build starttde" ${BUILD_ALL} )
option( BUILD_KFIND "Build kfind" ${BUILD_ALL} )
option( BUILD_DRKONQI "Build drkonqi" ${BUILD_ALL} )
option( BUILD_KDCOP "Build kdcop" ${BUILD_ALL} )
option( BUILD_KDEBUGDIALOG "Build kdebugdialog" ${BUILD_ALL} )
option( BUILD_KNETATTACH "Build knetattach" ${BUILD_ALL} )
option( BUILD_KPAGER "Build kpager" ${BUILD_ALL} )
option( BUILD_KAPPFINDER "Build kappfinder" ${BUILD_ALL} )
option( BUILD_KCHECKPASS "Build kcheckpass" ${BUILD_ALL} )
option( BUILD_KHELPCENTER "Build khelpcenter" ${BUILD_ALL} )
option( BUILD_KHOTKEYS "Build khotkeys" ${BUILD_ALL} )
option( BUILD_KSCREENSAVER "Build tdescreensaver" ${BUILD_ALL} )
option( BUILD_KSTART "Build kstart" ${BUILD_ALL} )
option( BUILD_KSYSTRAYCMD "Build ksystraycmd" ${BUILD_ALL} )
option( BUILD_NSPLUGINS "Build nsplugins"  ${BUILD_ALL} )
option( BUILD_KSYSGUARD "Build ksysguard"  ${BUILD_ALL} )
option( BUILD_KXKB "Build kxkb"  ${BUILD_ALL} )
option( BUILD_TSAK "Build tsak"  ${BUILD_ALL} )
option( BUILD_KROOTBACKING "Build krootbacking"  ${BUILD_ALL} )
option( BUILD_TQT3INTEGRATION "Build tqt3integration"  ${BUILD_ALL} )
option( BUILD_CRASHTEST "Build crash test program"  ${BUILD_ALL} )


##### set PKG_CONFIG_PATH #######################

set( ENV{PKG_CONFIG_PATH} "${PKG_CONFIG_PATH}:$ENV{PKG_CONFIG_PATH}:${LIB_INSTALL_DIR}/pkgconfig" )


##### configure checks ##########################

include( ConfigureChecks.cmake )


###### global compiler settings #################

add_definitions(
  -DHAVE_CONFIG_H
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TQT_CXX_FLAGS}" )
set( CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined" )
set( CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined" )

check_type_size( "long long" LONG_LONG )


##### tdebase directories #######################

add_subdirectory( tdmlib )

tde_conditional_add_subdirectory( BUILD_TDEINIT tdeinit )
tde_conditional_add_subdirectory( BUILD_L10N l10n )
tde_conditional_add_subdirectory( BUILD_PICS pics )
tde_conditional_add_subdirectory( BUILD_APPLNK applnk )
tde_conditional_add_subdirectory( BUILD_DOC doc )
tde_conditional_add_subdirectory( BUILD_LIBKONQ libkonq )
tde_conditional_add_subdirectory( BUILD_KCMINIT kcminit )
add_subdirectory( kcontrol )
tde_conditional_add_subdirectory( BUILD_TWIN twin )
tde_conditional_add_subdirectory( BUILD_KATE kate )
tde_conditional_add_subdirectory( BUILD_KONQUEROR konqueror )
tde_conditional_add_subdirectory( BUILD_KIOSLAVES kioslave )
tde_conditional_add_subdirectory( BUILD_KICKER kicker )
tde_conditional_add_subdirectory( BUILD_KDESKTOP kdesktop )
tde_conditional_add_subdirectory( BUILD_KONSOLE konsole )
tde_conditional_add_subdirectory( BUILD_KLIPPER klipper )
tde_conditional_add_subdirectory( BUILD_KMENUEDIT kmenuedit )
tde_conditional_add_subdirectory( BUILD_TDEPRINT tdeprint )
tde_conditional_add_subdirectory( BUILD_KPERSONALIZER kpersonalizer )
tde_conditional_add_subdirectory( BUILD_KDEPASSWD kdepasswd )
tde_conditional_add_subdirectory( BUILD_KTIP ktip )
tde_conditional_add_subdirectory( BUILD_KDEEJECT kdeeject )
tde_conditional_add_subdirectory( BUILD_KDIALOG kdialog )
tde_conditional_add_subdirectory( BUILD_KREADCONFIG kreadconfig )
tde_conditional_add_subdirectory( BUILD_TDESU tdesu )
tde_conditional_add_subdirectory( BUILD_TDM tdm )
tde_conditional_add_subdirectory( BUILD_KSMSERVER ksmserver )
tde_conditional_add_subdirectory( BUILD_KSPLASHML ksplashml )
tde_conditional_add_subdirectory( BUILD_KFIND kfind )
tde_conditional_add_subdirectory( BUILD_DRKONQI drkonqi )
tde_conditional_add_subdirectory( BUILD_KDCOP kdcop )
tde_conditional_add_subdirectory( BUILD_KDEBUGDIALOG kdebugdialog )
tde_conditional_add_subdirectory( BUILD_KNETATTACH knetattach )
tde_conditional_add_subdirectory( BUILD_KPAGER kpager )
tde_conditional_add_subdirectory( BUILD_KAPPFINDER kappfinder )
tde_conditional_add_subdirectory( BUILD_KCHECKPASS kcheckpass )
tde_conditional_add_subdirectory( BUILD_KHELPCENTER khelpcenter )
tde_conditional_add_subdirectory( BUILD_KHOTKEYS khotkeys )
tde_conditional_add_subdirectory( BUILD_KSCREENSAVER tdescreensaver )
tde_conditional_add_subdirectory( BUILD_KSTART kstart )
tde_conditional_add_subdirectory( BUILD_KSYSTRAYCMD ksystraycmd )
tde_conditional_add_subdirectory( BUILD_NSPLUGINS nsplugins )
tde_conditional_add_subdirectory( BUILD_KSYSGUARD ksysguard )
tde_conditional_add_subdirectory( BUILD_KXKB kxkb )
tde_conditional_add_subdirectory( BUILD_TSAK tsak )
tde_conditional_add_subdirectory( BUILD_KROOTBACKING krootbacking )
tde_conditional_add_subdirectory( BUILD_TQT3INTEGRATION tqt3integration )

##### install starttde & related stuff ##########

if( BUILD_STARTTDE )
  install( PROGRAMS starttde DESTINATION ${BIN_INSTALL_DIR} )
  install( PROGRAMS migratekde3 r14-xdg-update DESTINATION ${BIN_INSTALL_DIR} )
endif()


##### write configure files #####################

configure_file( config.h.cmake config.h @ONLY )
