/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BOARD_COMMON_H__
#define __BOARD_COMMON_H__

/**
 * @brief This global variable helps to configure variable gpios.
 */
extern uint8_t boot_mode_maf;

#if defined(CONFIG_SOC_FAMILY_MICROCHIP_MEC)

#if CONFIG_BOARD_MEC172XMODULAR_ASSY6930
#include "mecc172x_aic_rvp_selection.h"
#elif defined(CONFIG_BOARD_MTL_S_MEC172X_NSZ)
#include "mtl_s_mec172x.h"
#elif defined(CONFIG_BOARD_PTL_UH)
#include "ptl_uh_mec172x.h"
#elif defined(CONFIG_BOARD_MTL_S_MEC172X_LJ)
#include "mtl_template_mec172xlj.h"
#elif defined(CONFIG_BOARD_MEC172X_MTL_S)
#else
#error "Platform not supported"
#endif /* CONFIG_BOARD_MEC1501MODULAR_ASSY6885 */

#elif defined(CONFIG_SOC_SERIES_NPCX4)
#if defined(CONFIG_BOARD_NPCX4M8F_PTL)
#include "npcx4m8f_aic_rvp_selection.h"
#endif

#endif /* CONFIG_SOC_FAMILY_MICROCHIP_MEC */

#ifdef CONFIG_THERMAL_MANAGEMENT
#include "thermalmgmt.h"
#include "board_thermal.h"
#endif
/**
 * @brief Perform platform configuration depending on the board.
 *
 * @retval 0 If successful, otherwise negative error code.
 */
int board_init(void);

/**
 * @brief Perform platform configuration during suspend depending on the board.
 *
 * Note: Allows to optimize power consumption while the system is in S3/S4/S5.
 *
 * @retval 0 If successful, otherwise negative error code.
 */
int board_suspend(void);

/**
 * @brief Perform platform configuration during resume depending on the board.
 *
 * Note: Allows to restore pin functionality when the system exits S3/S4/S5.
 *
 * @retval 0 If successful, otherwise negative error code.
 */
int board_resume(void);

#endif /* __BOARD_COMMON_H__ */
