/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr.h>
#include <device.h>
#include "gpio_ec.h"
#include <soc.h>
#include <logging/log.h>
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


void pwrbtn_btn_handler(u8_t pwrbtn_sts)
{
	LOG_DBG("%s", __func__);
	gpio_write_pin(PM_PWRBTN, pwrbtn_sts);

	for (int i = 0; i < MAX_PWRBTN_HANDLERS; i++) {
		if (pwrbtn_handlers[i].handler) {
			LOG_DBG("Calling handler %s\n", __func__);
			pwrbtn_handlers[i].handler(pwrbtn_sts);
		}
	}
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
	return	periph_register_button(PWRBTN_EC_IN_N, pwrbtn_btn_handler);

}

void pwrbtn_trigger_wake(void)
{
	gpio_write_pin(PM_PWRBTN, 1);
	k_msleep(20);
	gpio_write_pin(PM_PWRBTN, 0);
	k_msleep(20);
	gpio_write_pin(PM_PWRBTN, 1);
}
