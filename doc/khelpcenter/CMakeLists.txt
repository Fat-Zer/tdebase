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

tde_create_handbook(
  FILES contact.docbook help.docbook index.docbook support.docbook welcome.docbook whatiskde.docbook
  DESTINATION khelpcenter )

install( FILES konq.css DESTINATION ${HTML_INSTALL_DIR}/en/khelpcenter/konq.css )
