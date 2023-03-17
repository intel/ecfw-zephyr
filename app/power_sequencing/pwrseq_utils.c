/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/device.h>
#include "board.h"
#include "board_config.h"
#include "port80display.h"
LOG_MODULE_DECLARE(pwrmgmt, CONFIG_PWRMGT_LOG_LEVEL);

/* This reset delay value is finalized as per the debug experiments
 * that were carried to fix the KSC reset issue.
 */
#define DEFAULT_EC_RESET_DELAY_MS	2u
#define PWROK_RSMRST_DELAY_US		10u

static struct wdt_timeout_cfg m_cfg_wdt;

/** Find the watchdog device instance and setup a timeout
 * to initiate the reset after timeout
 */
static int ec_arm_reset(void)
{
	int err;

	const struct device *wdt = DEVICE_DT_GET(WDT_0);

	if (!device_is_ready(wdt)) {
		/* device exists, but it failed to initialize */
		LOG_ERR("%s: WDT Device not ready", __func__);
		return -EINVAL;
	}

	m_cfg_wdt.callback = NULL;
	m_cfg_wdt.flags = WDT_FLAG_RESET_SOC;
	m_cfg_wdt.window.max = DEFAULT_EC_RESET_DELAY_MS;
	LOG_DBG("%s: max window time: %d", __func__, m_cfg_wdt.window.max);

	err = wdt_install_timeout(wdt, &m_cfg_wdt);
	if (err < 0) {
		LOG_ERR("%s: Watchdog install error", __func__);
	}

	LOG_DBG("%s: Timeout Installed Successfully", __func__);
	err = wdt_setup(wdt, 0);
	if (err < 0) {
		LOG_ERR("%s: Watchdog setup error", __func__);
	}

	LOG_DBG("%s: Setup Done Successfully", __func__);

	return 0;
}

void ec_reset(void)
{
	LOG_DBG("%s: Reset KSC command receieved", __func__);

	gpio_write_pin(SYS_PWROK, 0);
	gpio_write_pin(PCH_PWROK, 0);
#ifdef CONFIG_POSTCODE_MANAGEMENT
	port80_display_off();
#endif
	/* The busy wait is required to block the execution
	 * and to ramp down the above signals. Ideally, these should
	 * not be altered by any other thread.
	 */
	k_busy_wait(PWROK_RSMRST_DELAY_US);

	gpio_write_pin(PM_RSMRST, 0);
	LOG_DBG("%s: Before calling the reset_ec_chip", __func__);

	ec_arm_reset();

	/* Flush logging subsystem buffer prior to reset */
	LOG_PANIC();

	/* Once the EC reset is triggered, there should not be any other
	 * thread that gets executed, to avoid this, we need to stop
	 * indefinitely, so that no other thread gets executed during the
	 * reset sequence.
	 */
	while (1) {
	}

}
