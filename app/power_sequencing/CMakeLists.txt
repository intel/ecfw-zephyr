# SPDX-License-Identifier: Apache-2.0

target_sources_ifdef(CONFIG_PWRMGMT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/pwrplane.c
    ${CMAKE_CURRENT_LIST_DIR}/dswmode.c
    ${CMAKE_CURRENT_LIST_DIR}/deepsx.c
    ${CMAKE_CURRENT_LIST_DIR}/pseudog3.c
    ${CMAKE_CURRENT_LIST_DIR}/pmc.c
    ${CMAKE_CURRENT_LIST_DIR}/pwrseq_utils.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/pwrplane.h
    ${CMAKE_CURRENT_LIST_DIR}/dswmode.h
    ${CMAKE_CURRENT_LIST_DIR}/deepsx.h
    ${CMAKE_CURRENT_LIST_DIR}/pseudog3.h
    ${CMAKE_CURRENT_LIST_DIR}/pmc.h
    ${CMAKE_CURRENT_LIST_DIR}/pwrseq_utils.h
    )
