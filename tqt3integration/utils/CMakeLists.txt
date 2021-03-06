#################################################
#
#  (C) 2011 Timothy Pearson
#  kb9vqf (AT) pearsoncomputing.net
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

add_subdirectory( qt )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### gen (internal executable) ########################

tde_add_executable( gen AUTOMOC
  SOURCES
    gen.cpp
  LINK tdecore-shared
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qtkde_functions.cpp ${CMAKE_CURRENT_BINARY_DIR}/module_functions.cpp ${CMAKE_CURRENT_BINARY_DIR}/module_functions.h
  COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/qt
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/gen ${CMAKE_CURRENT_SOURCE_DIR}/gen.txt
  COMMAND cat ${CMAKE_CURRENT_BINARY_DIR}/qt/in/qtkdeintegration_x11_0.cpp ${CMAKE_CURRENT_SOURCE_DIR}/qt/in/qtkdeintegration_x11_1.cpp ${CMAKE_CURRENT_BINARY_DIR}/qtkdeintegration_x11.cpp.gen ${CMAKE_CURRENT_SOURCE_DIR}/qt/in/qtkdeintegration_x11_2.cpp > ${CMAKE_CURRENT_BINARY_DIR}/qt/qtkdeintegration_x11.cpp
  COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/qt/in/qtkdeintegration_x11_p_1.h ${CMAKE_CURRENT_BINARY_DIR}/qtkdeintegration_x11_p.h.gen ${CMAKE_CURRENT_SOURCE_DIR}/qt/in/qtkdeintegration_x11_p_2.h > ${CMAKE_CURRENT_BINARY_DIR}/qt/qtkdeintegration_x11_p.h
  COMMAND rm -f ${CMAKE_CURRENT_BINARY_DIR}/qtkdeintegration_x11.cpp.gen ${CMAKE_CURRENT_BINARY_DIR}/qtkdeintegration_x11_p.h.gen
#  COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/qt/in/qt.patch ${CMAKE_CURRENT_BINARY_DIR}/qt/
  DEPENDS gen
)
ADD_CUSTOM_TARGET(generate_tqt3_bindings DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/qtkde_functions.cpp)