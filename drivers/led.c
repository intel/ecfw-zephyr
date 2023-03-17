/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include "board_config.h"
#include "led.h"

LOG_MODULE_REGISTER(led, CONFIG_LED_LOG_LEVEL);

/*
 * PWM channel uses main clock frequency i.e. 48 MHz.
 * For desired frequency of 1 Hz, division should be:
 *
 * division = main clock freq / desired freq.
 *          = 48 MHz / 1Hz = 48000000.
 *
 * PWM Duty cycle = pulse width / period.
 *
 * To calculate duty cycle in percentage, multiplier should be:
 * multiplier = division / 100
 *            = 48000000 / 100 = 480000.
 */
#define	LED_PWM_FREQ_MULT	480000
#define MAX_DUTY_CYCLE		100u

/*
 * PWM channel uses main clock frequency i.e. 48 MHz.
 * For desired frequency of 24 KHz, division should be:
 *
 * division = main clock freq / desired freq.
 *          = 48 MHz / 24KHz = 2000.
 *
 * PWM Duty cycle = pulse width / period.
 *
 * To calculate duty cycle in percentage, multiplier should be:
 * multiplier = division / 100
 *            = 2000 / 100 = 20.
 */
#define	LED_KBD_BKLT_PWM_FREQ_MULT	20

static const struct device *led_dev[LED_TOTAL] = {NULL};

void led_blink(enum led_num idx, uint8_t duty_cycle)
{
	int ret;
	uint32_t pwm_multiplier;

	if (!led_dev[idx]) {
		LOG_WRN("No LED device");
		return;
	}

	if (duty_cycle > MAX_DUTY_CYCLE) {
		duty_cycle = MAX_DUTY_CYCLE;
	}

	switch (idx) {
	case LED_KBD_BKLT:
		pwm_multiplier = LED_KBD_BKLT_PWM_FREQ_MULT;
		break;

	case LED2:
	default:
		pwm_multiplier = LED_PWM_FREQ_MULT;
		break;
	}

	ret = pwm_set_cycles(led_dev[idx], 0,
			pwm_multiplier * MAX_DUTY_CYCLE,
			pwm_multiplier * duty_cycle, 0);

	if (ret) {
		LOG_WRN("LED blink error: %d", ret);
	}
}

void led_init(enum led_num idx)
{
	if (idx == LED2) {

#if DT_NODE_HAS_STATUS(BAT_LED2, okay)
		led_dev[LED2] = DEVICE_DT_GET(DT_PWMS_CTLR(BAT_LED2));

		if (!device_is_ready(led_dev[LED2])) {
			LOG_WRN("LED device not ready");
		}
#endif

	} else if (idx == LED_KBD_BKLT) {

#if DT_NODE_HAS_STATUS(KBD_BKLT_LED, okay)
		led_dev[LED_KBD_BKLT] =
			DEVICE_DT_GET(DT_PWMS_CTLR(KBD_BKLT_LED));

		if (!device_is_ready(led_dev[LED_KBD_BKLT])) {
			LOG_WRN("KBD BKLT LED device not ready");
		}
#endif
	}
}
