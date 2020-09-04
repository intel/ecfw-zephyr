/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __PWRPLANE_H__
#define __PWRPLANE_H__

#include "system.h"

/**
 * @brief Power control flags.
 */
struct pwr_flags {
	u8_t ac_powered:1;
	u8_t prev_ac_presence:1;
	u8_t wait_pwr_btn_up:1;
	u8_t turn_pwr_on:1;
	u8_t en_pwr_btn_notify:1;
	u8_t pwr_sw_enabled:1;
	u8_t pwr_sw_suspend_resume:1;
	u8_t def_turn_pwr_on:1;
	u8_t turn_pwr_off:1;
	u8_t s3_ac_event:1;
	u8_t ao_ac_timer_on:1;
	u8_t soisct_shift_key:1;
	u8_t deep_s3_timer_on:1;
	u8_t g3_exit:1;
	u8_t pm_rsmrst:1;
};

/**
 * @brief Routine that handles eSPI handshake with PMC.
 *
 * This routines also handles system events such as enter/exit S3/S4/S5 and
 * user events related to power/reset sequence.
 *
 * @param p1 pointer to additional task-specific data.
 * @param p2 pointer to additional task-specific data.
 * @param p2 pointer to additional task-specific data.
 *
 */
void pwrseq_thread(void *p1, void *p2, void *p3);

/**
 * @brief Handles power sequencing error.
 *
 * This routines displays error code and stops any power sequencing.
 *
 * @param error_code the power sequencing error.
 */
void pwrseq_error(u8_t error_code);

/**
 * @brief Indicates current system power state.
 *
 * @retval the current power state.
 */
enum system_power_state pwrseq_system_state(void);

/**
 * @brief Indicates the current boot mode.
 *
 * @retval is the current boot mode.
 */
enum boot_config_mode pwrseq_get_boot_mode(void);

extern struct pwr_flags g_pwrflags;

#endif /* __PWRPLANE_H__ */
