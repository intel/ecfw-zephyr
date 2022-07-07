/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include "gpio_ec.h"
#include "board.h"
#include "board_config.h"
#include "pwrplane.h"
#include "pseudog3.h"
#include "espi_hub.h"
#include "periphmgmt.h"
#include "pwrbtnmgmt.h"
#include <logging/log.h>
LOG_MODULE_DECLARE(pwrmgmt, CONFIG_PWRMGT_LOG_LEVEL);

/*
 * Pseudo G3 (PG3) is enabled by BIOS, EC gets notified via smchost command.
 *
 * When PG3 is enabled,
 * - BIOS programs the AC, DC counter values before entering into Sx.
 * - EC triggers PG3 mode when SUS_PWRDN_ACK received and system is in DC mode.
 * - Once the BIOS programs counters, count down begins regardless of PG3 entry.
 * - When the DC counter expires and platform is in DC mode, EC sends wake to
 *   exit pseudo g3.
 * - Once PG3 exited, EC clears all counter values.
 *
 */

static enum pg3_state_n {
	/* Pseudo G3 mode disabled */
	PG3_STATE_DISABLED,

	/* States when pseudo G3 Mode enabled */
	PG3_STATE_IDLE,
	PG3_STATE_WAITING_ENTRY,
	PG3_STATE_ENTERED,

	PG3_STATE_WAKE_PCH
} pg3_state;

static void counter_expired_hndlr(struct k_timer *counter);

K_TIMER_DEFINE(pg3_counter_dc, counter_expired_hndlr, NULL);

static void pseudo_g3_enter(void)
{
	int ac_prsnt;
	uint8_t sus_pwrdn_ack;

	if (pg3_state != PG3_STATE_WAITING_ENTRY) {
		LOG_DBG("Invalid state for Pseudo G3 entry");
		return;
	}

	if (!((pwrseq_system_state() == SYSTEM_S4_STATE) ||
	     (pwrseq_system_state() == SYSTEM_S5_STATE))) {
		LOG_DBG("Platform is not in Sx yet");
		return;
	}

	/* If system is in AC mode, do not enter PG3 */
	ac_prsnt = gpio_read_pin(BC_ACOK);
	if (ac_prsnt < 0) {
		LOG_ERR("%s fail to read BC_ACOK", __func__);
		return;
	}

#ifdef CONFIG_SMCHOST
	/* If system is in Virtual battery mode enter PG3 */
	if (ac_prsnt && !is_virtual_battery_prsnt()) {
		LOG_DBG("Can not enter Pseudo G3 until AC present.");
		return;
	}
#endif

	espihub_retrieve_vw(ESPI_VWIRE_SIGNAL_SUS_PWRDN_ACK, &sus_pwrdn_ack);

	if (!sus_pwrdn_ack) {
		LOG_DBG("Can not enter Pseudo G3 until sus_pwrdn_ack asserted");
		return;
	}
	/* drive Hw signal to enter PG3 */
	gpio_write_pin(PM_DS3, 1);
	k_msleep(10);
	gpio_write_pin(PM_DS3, 0);

	LOG_DBG("Entering Pseudo G3 state");
	pg3_state = PG3_STATE_ENTERED;
}

static void pseudo_g3_exit(void)
{
	LOG_DBG("Exiting Pseudo G3 state");

	/* drive Hw signal to exit PG3 */
	gpio_write_pin(EC_PG3_EXIT, 1);
	k_msleep(10);
	gpio_write_pin(EC_PG3_EXIT, 0);

#ifdef CONFIG_PWRMGMT_PG3_EXIT_WAKE_FROM_SX
	/* If needed to wake system from Sx after PG3 exit, send the wake. */
	pwrbtn_trigger_wake();
#endif
}

static void counter_expired_hndlr(struct k_timer *counter)
{
	LOG_DBG("Counter expiry handler");

	/* Check if system is in AC / DC mode */
	int ac_prsnt = gpio_read_pin(BC_ACOK);

	if (ac_prsnt < 0) {
		LOG_ERR("%s fail to read BC_ACOK", __func__);
		return;
	}

	if (counter == &pg3_counter_dc) {
		LOG_DBG("PG3 DC Counter expired");

		if (ac_prsnt && !is_virtual_battery_prsnt()) {
			pg3_state = PG3_STATE_IDLE;
			LOG_DBG("AC present, do not send wake");
		} else {
			pg3_state = PG3_STATE_WAKE_PCH;
			LOG_DBG("Send PG3 wake");
		}
	}
}



void pseudo_g3_enable(bool state)
{
	pg3_state = state ? PG3_STATE_IDLE : PG3_STATE_DISABLED;
	LOG_DBG("pg3state %d", pg3_state);
}

bool pseudo_g3_get_state(void)
{
	return (pg3_state == PG3_STATE_ENTERED);
}

void pseudo_g3_program_counter(enum pg3_counter counter, uint32_t count)
{
	if (pg3_state == PG3_STATE_DISABLED) {
		LOG_WRN("PG3 State Disabled");
		return;
	}

	switch (counter) {
	case PG3_COUNTER_DC:
		LOG_DBG("Programming DC counter value %d seconds", count);
		k_timer_start(&pg3_counter_dc, K_SECONDS(count), K_NO_WAIT);
		break;
	default:
		/* Other counters are not supported */
		break;
	}

}

void manage_pseudog3(void)
{
	switch (pg3_state) {
	case PG3_STATE_IDLE:
		switch (pwrseq_system_state()) {
		case SYSTEM_S0_STATE:
			pg3_state = PG3_STATE_IDLE;
			break;
		case SYSTEM_S4_STATE:
		case SYSTEM_S5_STATE:
			pg3_state = PG3_STATE_WAITING_ENTRY;
			break;
		default:
			break;
		}
		break;

	case PG3_STATE_WAITING_ENTRY:
		pseudo_g3_enter();
		break;
	case PG3_STATE_WAKE_PCH:
		pseudo_g3_exit();

		if (pwrseq_system_state() == SYSTEM_S0_STATE) {
			pg3_state = PG3_STATE_IDLE;
		}

		break;

	case PG3_STATE_DISABLED:
		break;
	default:
		break;
	}
}
