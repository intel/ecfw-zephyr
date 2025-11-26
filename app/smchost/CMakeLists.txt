# SPDX-License-Identifier: Apache-2.0

target_sources_ifdef(CONFIG_SMCHOST app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/smchost.c
    ${CMAKE_CURRENT_LIST_DIR}/smchost_info.c
    ${CMAKE_CURRENT_LIST_DIR}/smchost_pm.c
    ${CMAKE_CURRENT_LIST_DIR}/smc.c
    ${CMAKE_CURRENT_LIST_DIR}/sci.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/acpi_region.h
    ${CMAKE_CURRENT_LIST_DIR}/smchost.h
    ${CMAKE_CURRENT_LIST_DIR}/smc.h
    ${CMAKE_CURRENT_LIST_DIR}/sci.h
    ${CMAKE_CURRENT_LIST_DIR}/scicodes.h
    )

target_sources_ifdef(CONFIG_PFAT_SUPPORT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/smchost_pfat.c
    )

target_sources_ifdef(CONFIG_THERMAL_MANAGEMENT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/smchost_thermal.c
    )
