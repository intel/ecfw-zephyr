# SPDX-License-Identifier: Apache-2.0

target_sources_ifdef(CONFIG_MEC15XX_AIC_ON_ICL app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/icl_mec1501.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/icl_mec1501.h
    )

target_sources_ifdef(CONFIG_MEC15XX_AIC_ON_TGL app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/tgl_mec1501.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/tgl_mec1501.h
    )

target_sources_ifdef(CONFIG_MEC15XX_AIC_ON_ADL_S app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/mec15xx_aic_on_adl_s.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/mec15xx_aic_on_adl_s.h
    )

target_sources_ifdef(CONFIG_MEC15XX_AIC_ON_ADL_P app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/mec15xx_aic_on_adl_p.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/mec15xx_aic_on_adl_p.h
    )
