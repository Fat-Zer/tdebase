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

add_subdirectory( Applications )
add_subdirectory( Manpages )
# add_subdirectory( Scrollkeeper )

install( FILES
  applets.desktop contact.desktop faq.desktop info.desktop
  links.desktop support.desktop userguide.desktop welcome.desktop
  hardwaremodules.desktop kcontrolmodules.desktop konqplugins.desktop
  tdeioslaves.desktop quickstart.desktop visualdict.desktop adminguide.desktop
  DESTINATION ${DATA_INSTALL_DIR}/khelpcenter/plugins )
