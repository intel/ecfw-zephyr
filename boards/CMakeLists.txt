# SPDX-License-Identifier: Apache-2.0

target_sources(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/board.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/board.h
    )

if(CONFIG_SOC_FAMILY_MEC)

target_sources_ifdef(CONFIG_BOARD_MEC172XLJ_MTL_TEMPLATE app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/microchip/mtl_template_mec172xlj.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/microchip/mtl_template_mec172xlj.h
    )

target_sources_ifdef(CONFIG_BOARD_MEC172X_MTL_S app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/microchip/mtl_s_mec172x.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/microchip/mtl_s_mec172x.h
    )

target_sources_ifdef(CONFIG_BOARD_MEC172X_PTL_UH app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/microchip/ptl_uh_mec172x.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/microchip/ptl_uh_mec172x.h
    )

target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/microchip
    )

elseif(CONFIG_SOC_SERIES_NPCX4)

include(${CMAKE_CURRENT_LIST_DIR}/nuvoton/npcx4m8f_mecc/CMakeLists.txt)

target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/nuvoton
    ${CMAKE_CURRENT_LIST_DIR}/nuvoton/npcx4m8f_mecc    )

endif()