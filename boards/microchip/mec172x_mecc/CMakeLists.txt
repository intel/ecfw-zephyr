# SPDX-License-Identifier: Apache-2.0

target_sources_ifdef(CONFIG_MEC172X_AIC_ON_ADL_S app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/mecc172x_aic_on_adl_s.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/mecc172x_aic_on_adl_s.h
    )

target_sources_ifdef(CONFIG_MEC172X_AIC_ON_ADL_P app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/mecc172x_aic_on_adl_p.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/mecc172x_aic_on_adl_p.h
    )

target_sources_ifdef(CONFIG_MEC172X_AIC_ON_MTL_P app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/mecc172x_aic_on_mtl_p.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/mecc172x_aic_on_mtl_p.h
    )
