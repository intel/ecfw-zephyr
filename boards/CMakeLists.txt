target_sources(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/board.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/board.h
    )

if(CONFIG_SOC_FAMILY_MEC)

target_sources_ifdef(CONFIG_BOARD_MEC1501_ICL app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/microchip/icl_mec1501.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/microchip/icl_mec1501.h
    )

target_sources_ifdef(CONFIG_BOARD_MEC1501_TGL app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/microchip/tgl_mec1501.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/microchip/tgl_mec1501.h
    )

target_sources_ifdef(CONFIG_BOARD_MEC1501_EUCLID app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/microchip/euclid_mec1501.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/microchip/euclid_mec1501.h
    )

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

target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/microchip
    )

endif()
