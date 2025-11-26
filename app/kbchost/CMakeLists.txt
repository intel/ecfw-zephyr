# SPDX-License-Identifier: Apache-2.0

target_sources_ifdef(CONFIG_ESPI_PERIPHERAL_8042_KBC app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/kbchost.c
    ${CMAKE_CURRENT_LIST_DIR}/keyboard_utility.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/kbchost.h
    ${CMAKE_CURRENT_LIST_DIR}/keyboard_utility.h
    )
