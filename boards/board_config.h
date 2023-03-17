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

#if defined(CONFIG_SOC_FAMILY_MEC)

#ifdef CONFIG_BOARD_MEC1501MODULAR_ASSY6885
#include "mec15xx_aic_rvp_selection.h"
#elif CONFIG_BOARD_MEC172XMODULAR_ASSY6930
#include "mecc172x_aic_rvp_selection.h"
#elif defined(CONFIG_BOARD_MEC1501_EUCLID)
#include "euclid_mec1501.h"
#elif defined(CONFIG_BOARD_MEC1501_ADL)
#include "adl_mec1501.h"
#elif defined(CONFIG_BOARD_MEC1501_ADL_P)
#include "adl_p_mec1501.h"
#elif defined(CONFIG_BOARD_MEC1501_MTL_P)
#include "mtl_p_mec1501.h"
#elif defined(CONFIG_BOARD_MEC172X_MTL_S)
#include "mtl_s_mec172x.h"
#else
#error "Platform not supported"
#endif /* CONFIG_BOARD_MEC1501MODULAR_ASSY6885 */

#endif /* CONFIG_SOC_FAMILY_MEC */

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
