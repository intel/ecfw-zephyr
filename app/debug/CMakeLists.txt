# SPDX-License-Identifier: Apache-2.0

target_sources_ifdef(CONFIG_POSTCODE_MANAGEMENT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/postcodemgmt.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/postcodemgmt.h
    )

