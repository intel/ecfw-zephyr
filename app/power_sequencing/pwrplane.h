/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __PWRPLANE_H__
#define __PWRPLANE_H__

#include "system.h"

/* Macros needed for power boss params update to host */
#define PEAK_CUR_CAP_MODE0	0
#define PEAK_CUR_CAP_MODE1	1
#define PEAK_CUR_CAP_MODE2	2
#define PEAK_CUR_CAP_MODE3	3
#define PEAK_CUR_EQ_IOC		100
#define PEAK_CUR_150_IOC_1MS	150
#define PEAK_CUR_125_IOC_2MS	125
#define PEAK_CUR_110_IOC_10MS	110
#define PEAK_CUR_200_IOC_1MS	200
#define PEAK_CUR_150_IOC_2MS	150
#define PEAK_CUR_125_IOC_10MS	125
#define PEAK_CUR_175_IOC_2MS	175
#define PEAK_CUR_150_IOC_10MS	150

#define SHUTDOWN_REASON_DEFAULT			0x0
#define SHUTDOWN_REASON_CRTITICAL_THERMAL	0x1
/**
 * @brief Power control flags.
 */
struct pwr_flags {
	uint8_t ac_powered:1;
	uint8_t prev_ac_presence:1;
	uint8_t wait_pwr_btn_up:1;
	uint8_t turn_pwr_on:1;
	uint8_t en_pwr_btn_notify:1;
	uint8_t pwr_sw_enabled:1;
	uint8_t def_turn_pwr_on:1;
	uint8_t turn_pwr_off:1;
	uint8_t s3_ac_event:1;
	uint8_t ao_ac_timer_on:1;
	uint8_t soisct_shift_key:1;
	uint8_t deep_s3_timer_on:1;
	uint8_t g3_exit:1;
	uint8_t pm_rsmrst:1;
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
void pwrseq_error(uint8_t error_code);

/**
 * @brief Sets next_state to S5.
 *
 * This routines makes the EC transition to S5 by setting the next
 * state to S5.
 */
void set_next_state_to_S5(void);

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

/**
 * @brief Detect ATX presence.
 *
 * @retval true if atx present, else false.
 */
bool atx_detect(void);

/**
 * @brief API to shutdown the host.
 *
 * This is called by thermal task when CPU temp crosses above crit threshold.
 *
 */
void therm_shutdown(void);

/**
 * @brief API to read shutdown reason.
 *
 * This is called by smc host task to know the shutdown reason.
 *
 */
uint8_t read_shutdown_reason(void);

void set_shutdown_reason(uint8_t reason);

extern struct pwr_flags g_pwrflags;
#endif /* __PWRPLANE_H__ */
