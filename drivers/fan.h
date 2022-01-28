/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __FAN_H__
#define __FAN_H__

enum pwm_ch_num {
	PWM_CH_00,
	PWM_CH_01,
	PWM_CH_02,
	PWM_CH_03,
	PWM_CH_04,
	PWM_CH_05,
	PWM_CH_06,
	PWM_CH_07,
	PWM_CH_08,
	PWM_CH_TOTAL,
	PWM_CH_UNDEF = 0xFF,
};

enum tach_ch_num {
	TACH_CH_00,
	TACH_CH_01,
	TACH_CH_02,
	TACH_CH_03,
	TACH_CH_TOTAL,
	TACH_CH_UNDEF = 0xFF,
};

enum fan_type {
	FAN_CPU,
	FAN_REAR,
	FAN_GFX,
	FAN_PCH,
	FAN_DEV_TOTAL,
	FAN_DEV_UNDEF = 0xFF,
};

struct fan_dev {
	enum pwm_ch_num pwm_ch;
	enum tach_ch_num tach_ch;
};

/**
 * @brief Perform the the fan initialization.
 *
 * Initialize and configure below for each fan device instance:
 * - PWM channel to drive the fan.
 * - Tach channel to read the fan speed.
 *
 * @param size size of table for number of supported fan devices.
 * @param fan_tbl pointer to array of fan_dev instances.
 *
 * @return 0 if success, otherwise error code.
 */
int fan_init(int size, struct fan_dev *fan_tbl);

/**
 * @brief Set fan supply power on / off.
 *
 * Call this function when fan power supply needs to be removed like in
 * connected standby low power mode.
 *
 * @param power_state 1 - to set fan power on, else 0.
 *
 * @return 0 if success, otherwise error code.
 */
int fan_power_set(bool power_state);

/**
 * @brief  Set the fan pwm channel duty cycle percent.
 *
 * @param fan_idx fan type.
 * @param duty_cycle Fan pwm duty_cycle in % value can be between 0 to 100.
 *
 * @return 0 if success, otherwise error code.
 */
int fan_set_duty_cycle(enum fan_type fan_idx, uint8_t duty_cycle);

/**
 * @brief  read fan rpm value using tach.
 *
 * The updated fan speed i.e. rotation per minute (rpm) value is stored in the
 * fan_dev struct parameter fan->rpm.
 *
 * @param fan_idx fan type.
 * @param rpm pointer to update rpm value.
 *
 * @return 0 if success, otherwise error code.
 */
int fan_read_rpm(enum fan_type fan_idx, uint16_t *rpm);


#endif	/* __FAN_H__ */
