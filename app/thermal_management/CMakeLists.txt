# SPDX-License-Identifier: Apache-2.0

if (CONFIG_THERMAL_MANAGEMENT)
    target_sources(app
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/thermalmgmt.c
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/thermalmgmt.h
        )
endif()
