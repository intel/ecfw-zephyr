# SPDX-License-Identifier: Apache-2.0

target_sources_ifdef(CONFIG_BOARD_NPCX4M8F_PTL app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/npcx4m8f_aic_on_ptl.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/npcx4m8f_aic_on_ptl.h
    )
