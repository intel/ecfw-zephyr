/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <soc.h>
#include <zephyr/logging/log.h>
#include "board_config.h"
#include "gpio_ec.h"
#include "periphmgmt.h"
#include "pwrseq_utils.h"
#include "kbs_matrix.h"
LOG_MODULE_REGISTER(soc_debug, CONFIG_PWRMGT_LOG_LEVEL);

#ifdef CONFIG_SOC_DEBUG_CONSENT_GPIO
struct k_work_delayable sampling_work;
static bool sampled;

static void soc_debug_sampling_work_handler(struct k_work *work)
{
	LOG_DBG("%s", __func__);

	/* SW strap takes precedence over HW strap evaluation
	 * also for this feature.
	 */
	ec_evaluate_timeout();
	sampled = true;
}

void soc_debug_handler(uint8_t debug_consent)
{
	/* Ignore any changes prior to sampling time */
	if (!sampled) {
		return;
	}

	LOG_DBG("%s soc_gpio: %d", __func__, debug_consent);
	if (!debug_consent) {
		LOG_WRN("EC WDT disabled via PMC_GPIO/SoC-bootstall");
		disable_ec_timeout();
	}
}
#endif

void soc_debug_init(void)
{
	LOG_DBG("%s", __func__);

#ifdef CONFIG_SOC_DEBUG_CONSENT_GPIO
	sampled = false;
	k_work_init_delayable(&sampling_work, soc_debug_sampling_work_handler);

#ifndef CONFIG_POWER_SEQUENCE_DISABLE_TIMEOUTS
	k_work_schedule(&sampling_work,
			      K_MSEC(CONFIG_SOC_GPIO_VALID_AFTER_MS));
#endif /* CONFIG_POWER_SEQUENCE_DISABLE_TIMEOUTS */

	/* Monitor for changes */
	periph_register_button(TIMEOUT_DISABLE, soc_debug_handler);
#endif
}

void soc_debug_reset(void)
{
	LOG_DBG("%s", __func__);

#ifdef CONFIG_SOC_DEBUG_CONSENT_GPIO
	/* Required whenever eSPI reset or other condition requires*/
	if (sampled) {
		sampled = false;
		k_work_schedule(&sampling_work,
			      K_MSEC(CONFIG_SOC_GPIO_VALID_AFTER_MS));
	}
#endif /* SOC_DEBUG_CONSENT_GPIO */
}

void soc_debug_consent_kbs(void)
{
#ifdef CONFIG_POWER_SEQUENCE_DISABLE_TIMEOUT_HOTKEY
	/* Check if key already detected */
	if (kbs_keyseq_boot_detect(KEYSEQ_TIMEOUT)) {
		disable_ec_timeout();
		LOG_WRN("EC timeout disabled via kbs");
	}
#endif
}
