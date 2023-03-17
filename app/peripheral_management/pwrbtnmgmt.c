/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include "gpio_ec.h"
#include <soc.h>
#include <zephyr/logging/log.h>
#include "pwrbtnmgmt.h"
#include "periphmgmt.h"
#include "board_config.h"
LOG_MODULE_REGISTER(periph, CONFIG_PERIPHERAL_LOG_LEVEL);

/* Delay to start power on sequencing - 300 ms*/
#define PWRBTN_POWERON_DELAY          300
#define MAX_PWRBTN_HANDLERS           4

struct pwrbtn_handler {
	sys_snode_t	node;
	pwrbtn_handler_t handler;
};

/* This is just a pool */
static struct pwrbtn_handler pwrbtn_handlers[MAX_PWRBTN_HANDLERS];
static int pwrbtn_handler_index;
static bool usb_pwr_btn_sts = HIGH;
static bool sys_pwr_btn_sts = HIGH;
static bool pwr_btn_out_sts = HIGH;

void pwrbtn_btn_evt_processor(void)
{
	bool pwrbtn_evt;

	/*
	 * Calculate the executable power button status based
	 * on system and USB dock power button status.
	 */
	pwrbtn_evt = usb_pwr_btn_sts & sys_pwr_btn_sts;

	/* Current status should be different from last
	 * executed status.
	 */
	if (pwrbtn_evt != pwr_btn_out_sts) {

		pwr_btn_out_sts = pwrbtn_evt;

		LOG_DBG(" %s: power button event %d is executing ",
				__func__, pwr_btn_out_sts);

		gpio_write_pin(PM_PWRBTN, pwr_btn_out_sts);

		for (int i = 0; i < MAX_PWRBTN_HANDLERS; i++) {
			if (pwrbtn_handlers[i].handler) {
				LOG_DBG("Calling handler %s", __func__);
				pwrbtn_handlers[i].handler(pwr_btn_out_sts);
			}
		}
	}
}


void sys_pwrbtn_evt_processor(uint8_t pwrbtn_evt)
{
	LOG_DBG("sys_evt=%d, usb_pwr_btn_sts=%d, sys_pwr_btn_sts=%d",
		pwrbtn_evt, usb_pwr_btn_sts, sys_pwr_btn_sts);

	sys_pwr_btn_sts = pwrbtn_evt;
	pwrbtn_btn_evt_processor();
}

void pwrbtn_register_handler(pwrbtn_handler_t handler)
{
	LOG_DBG("%s", __func__);

	if (pwrbtn_handler_index < MAX_PWRBTN_HANDLERS - 1) {
		pwrbtn_handlers[pwrbtn_handler_index].handler = handler;
		pwrbtn_handler_index++;
	}
}

int pwrbtn_init(void)
{
	LOG_INF("%s", __func__);

	/* Register power button for debouncing */
	return	periph_register_button(PWRBTN_EC_IN_N,
			sys_pwrbtn_evt_processor);

}

void pwrbtn_trigger_wake(void)
{
	gpio_write_pin(PM_PWRBTN, 1);
	k_msleep(20);
	gpio_write_pin(PM_PWRBTN, 0);
	k_msleep(20);
	gpio_write_pin(PM_PWRBTN, 1);
}
