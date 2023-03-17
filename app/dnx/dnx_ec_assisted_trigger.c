/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/espi.h>
#include <zephyr/logging/log.h>
#include "board_config.h"
#include "gpio_ec.h"
#include "dnx.h"
#include "pmc.h"
#include "pwrseq_utils.h"
LOG_MODULE_REGISTER(dnx_assisted, CONFIG_DNX_LOG_LEVEL);

static bool pending_restart;

void dnx_soc_handshake(void)
{
	LOG_DBG("%s", __func__);
	gpio_write_pin(DNX_FORCE_RELOAD_EC, 1);
	pending_restart = true;
}

void dnx_ec_assisted_restart(void)
{
	/* Trigger system reset */
	LOG_INF("%s", __func__);
	pending_restart = true;
	pmc_reset_soc(PMC_SYSTEM_COLD_RESET);
}

void dnx_ec_assisted_init(void)
{
	/* Initialize module */
	pending_restart = false;
}

void dnx_ec_assisted_manage(void)
{
	LOG_INF("%s pending:%d", __func__, pending_restart);

	if (pending_restart) {
		LOG_INF("Timeouts disabled due to Dnx");
		disable_ec_timeout();
		pending_restart = false;
	}
}
