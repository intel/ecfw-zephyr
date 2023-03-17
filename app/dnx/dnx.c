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
#include "gpio_ec.h"
#include "espi_hub.h"
#include "pwrseq_utils.h"
#include "dnx.h"

LOG_MODULE_REGISTER(dnx, CONFIG_DNX_LOG_LEVEL);

static bool dnx_entered;

void dnx_warn_handler(uint8_t status)
{
	dnx_entered = status;
	if (status) {
		disable_ec_timeout();
		LOG_WRN("EC timeout disabled due to DnX");
	}
}

void dnx_espi_bus_reset_handler(uint8_t status)
{
	LOG_DBG("%s espi_rst=%d while dnx=%d", __func__, status, dnx_entered);

	if (dnx_entered && status == 0) {
#ifdef CONFIG_DNX_SELF_RESET
		ec_reset();
#endif
	}
}

void dnx_handle_early_handshake(void)
{
	/* DnX warning occurs prior to SUS_WRN and immediately after eSPI driver
	 * sends SLAVE_BOOT_DONE whenever DnX bit is set in IFWI strap.
	 * In such case driver callbacks haven't been registered, need to check
	 * DnX VW wire directly.
	 */
	if (espihub_dnx_status()) {
		dnx_warn_handler(1);
	};

	/* Register for other eSPI relevant notifications */
	espihub_add_warn_handler(ESPIHUB_DNX_WARNING, dnx_warn_handler);
}
