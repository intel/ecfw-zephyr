# SPDX-License-Identifier: Apache-2.0

target_sources_ifdef(CONFIG_PERIPHERAL_MANAGEMENT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/periphmgmt.c
    ${CMAKE_CURRENT_LIST_DIR}/pwrbtnmgmt.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/periphmgmt.h
    ${CMAKE_CURRENT_LIST_DIR}/pwrbtnmgmt.h
    )