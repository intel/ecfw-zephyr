# SPDX-License-Identifier: Apache-2.0

target_sources_ifdef(CONFIG_SOC_DEBUG_AWARENESS app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/soc_debug.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/soc_debug.h
    )
