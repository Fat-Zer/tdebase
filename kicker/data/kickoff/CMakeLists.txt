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

tde_install_icons(
    leave recently_used suspend2disk suspend2ram
  DESTINATION ${DATA_INSTALL_DIR}/kicker/icons )

install( FILES
    resize_handle.png main_corner_tl.png main_corner_tr.png
    search-gradient.png menu_separator.png search-tab-center.png
    search-tab-left.png search-tab-right.png search-tab-top-center.png
    search-tab-top-left.png left_triangle.png right_triangle.png
    kmenu_basic.mng
    search-tab-top-right.png search-gradient-topdown.png
    search-running.mng
  DESTINATION ${DATA_INSTALL_DIR}/kicker/pics )
