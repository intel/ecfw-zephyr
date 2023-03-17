# SPDX-License-Identifier: Apache-2.0

target_sources(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/board.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/board.h
    )

if(CONFIG_SOC_FAMILY_MEC)

# RVP + MECC
include(${CMAKE_CURRENT_LIST_DIR}/microchip/mec15xx_mecc/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/microchip/mec172x_mecc/CMakeLists.txt)

target_sources_ifdef(CONFIG_BOARD_MEC1501_ADL app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/microchip/adl_mec1501.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/microchip/adl_mec1501.h
    )

target_sources_ifdef(CONFIG_BOARD_MEC1501_ADL_P app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/microchip/adl_p_mec1501.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/microchip/adl_p_mec1501.h
    )

target_sources_ifdef(CONFIG_BOARD_MEC1501_MTL_P app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/microchip/mtl_p_mec1501.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/microchip/mtl_p_mec1501.h
    )

target_sources_ifdef(CONFIG_BOARD_MEC172X_MTL_S app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/microchip/mtl_s_mec172x.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/microchip/mtl_s_mec172x.h
    )

target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/microchip
    ${CMAKE_CURRENT_LIST_DIR}/microchip/mec15xx_mecc
    ${CMAKE_CURRENT_LIST_DIR}/microchip/mec172x_mecc
    )

endif()
