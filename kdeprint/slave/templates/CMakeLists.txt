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

install( FILES
    class.template printer.template pseudo.template
    jobs.template driver.template bar-bottom-left.png
    bar-top-middle.png box-bottom-middle.png
    box-top-middle.png bar-bottom-middle.png
    bar-top-right.png box-bottom-right.png
    box-top-right.png bar-bottom-right.png
    bottom-left.png box-centre-tdeprint.png
    top-left.png bar-middle-left.png bottom-middle.png
    box-middle-left.png top-middle.png bar-middle-right.png
    bottom-right.png box-middle-right.png top-right-tdeprint.png
    bar-top-left.png box-bottom-left.png box-top-left.png
    htmlstyles.css
  DESTINATION ${DATA_INSTALL_DIR}/tdeprint/template )
