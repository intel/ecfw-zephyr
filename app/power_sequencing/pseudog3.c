/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include "gpio_ec.h"
#include "board.h"
#include "board_config.h"
#include "pwrplane.h"
#include "pseudog3.h"
#include "espi_hub.h"
#include "periphmgmt.h"
#include "pwrbtnmgmt.h"
#include <zephyr/logging/log.h>
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

enum pg3_state_n {
	/* Pseudo G3 mode disabled */
	PG3_STATE_DISABLED,

	/* States when pseudo G3 Mode enabled */
	PG3_STATE_IDLE,
	PG3_STATE_WAITING_ENTRY,
	PG3_STATE_ENTERED,
	PG3_STATE_WAKE_WAIT,
};
static enum pg3_state_n pg3_state, pg3_prev_state;

static void counter_expired_hndlr(struct k_timer *counter);

K_TIMER_DEFINE(pg3_counter_dc, counter_expired_hndlr, NULL);
static bool pg3_generate_wake;
static bool pg3_enable_status;

static bool is_pseudo_g3_condition(void)
{
	int ac_prsnt;
	uint8_t sus_pwrdn_ack;

	/* If system is in AC mode, do not enter PG3 */
	ac_prsnt = gpio_read_pin(BC_ACOK);
	if (ac_prsnt < 0) {
		LOG_ERR("%s fail to read BC_ACOK", __func__);
		return false;
	}

	/* If system is in Virtual battery mode enter PG3 */
	if (ac_prsnt && !is_virtual_battery_prsnt()) {
		LOG_DBG("No Pseudo G3 with AC present.");
		return false;
	}

	if (gpio_read_pin(ESPI_RESET_MAF) > 0) {
		espihub_retrieve_vw(ESPI_VWIRE_SIGNAL_SUS_PWRDN_ACK, &sus_pwrdn_ack);
		if (!sus_pwrdn_ack) {
			LOG_DBG("No Pseudo G3 with sus_pwrdn_ack:0");
			return false;
		}
	}
	return true;
}

static void pseudo_g3_trigger_exit(void)
{
	LOG_DBG("Triggering Pseudo G3 exit");

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
			LOG_DBG("AC present, do not send wake");
		} else {
			pg3_generate_wake = true;
			LOG_DBG("Generate PG3 wake");
		}
	}
}

static void pseudo_g3_set_state(uint8_t next_state)
{
	LOG_DBG("Moving to PG3 state:%d", next_state);
	pg3_prev_state = pg3_state;
	pg3_state = next_state;
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

static void pg3_handle_state_disabled(void)
{
	/* If PG3 enabled by user, then move to PG3_STATE_IDLE */
	if (pg3_enable_status) {
		pseudo_g3_set_state(PG3_STATE_IDLE);
	}
	/* Default continue in PG3_STATE_DISABLED */
}

static void pg3_handle_state_idle(void)
{
	/* Move to PG3_STATE_DISABLED if PG3 disabled by user.
	 * If PG3 enabled then,
	 * 1. Move to PG3_STATE_WAITING_ENTRY if system in S4/S5.
	 * 2. Continue in PG3_STATE_IDLE if system in S0/S3/G3..
	 */

	if (pg3_enable_status == false) {
		pseudo_g3_set_state(PG3_STATE_DISABLED);
		return;
	}

	switch (pwrseq_system_state()) {
	case SYSTEM_S4_STATE:
	case SYSTEM_S5_STATE:
		pseudo_g3_set_state(PG3_STATE_WAITING_ENTRY);
		return;
	default:
		break;
	}
	/* Default continue in PG3_STATE_IDLE */
}

static void pg3_handle_state_waiting_entry(void)
{
	/* 1. Move to PG3_STATE_IDLE if system transition to S0/S3.
	 * 2. Move to PG3_STATE_ENTERED if pg3 condition meet.
	 * 3. Continue in PG3_STATE_WAITING_ENTRY if pg3 not condition meet.
	 */
	switch (pwrseq_system_state()) {
	case SYSTEM_S0_STATE:
	case SYSTEM_S3_STATE:
	case SYSTEM_G3_STATE:
		pseudo_g3_set_state(PG3_STATE_IDLE);
		return;
	case SYSTEM_S4_STATE:
	case SYSTEM_S5_STATE:
		/* If PG3 condition meet, then move to
		 * PG3_STATE_ENTERED state
		 */
		if (is_pseudo_g3_condition()) {
			LOG_DBG("Entering Pseudo G3 state");
			/* drive Hw signal to enter PG3 */
			gpio_write_pin(PM_DS3, 1);
			k_msleep(10);
			gpio_write_pin(PM_DS3, 0);

			pseudo_g3_set_state(PG3_STATE_ENTERED);
			return;
		}
	default:
		break;
	}
	/* Default continue in PG3_STATE_WAITING_ENTRY */
}

static void pg3_handle_state_entered(void)
{
	/* 1. Move to PG3_STATE_IDLE if system transition to S0/S3.
	 * 2. Move to PG3_STATE_WAITING_ENTRY if ac connect.
	 * 3. If RSMRST released then pg3 wake due to pwrnbtn or timer expiry.
	 *    Move to PG3_STATE_WAKE_WAIT till system move to S0/S3
	 * 4. Else Continue in PG3_STATE_ENTERED as pg3 condition meet.
	 */
	switch (pwrseq_system_state()) {
	case SYSTEM_S0_STATE:
	case SYSTEM_S3_STATE:
	case SYSTEM_G3_STATE:
		pseudo_g3_set_state(PG3_STATE_IDLE);
		return;
	case SYSTEM_S4_STATE:
	case SYSTEM_S5_STATE:
		/* If PG3 condition not meet, then move to
		 * PG3_STATE_WAITING_ENTRY state and from that state
		 * move appropriately.
		 */

		/* wake due to power adapter insertion in PG3 */
		if ((gpio_read_pin(BC_ACOK) > 0)
			&& (!is_virtual_battery_prsnt())) {
			LOG_DBG("No Pseudo G3 with AC present.");
			pseudo_g3_set_state(PG3_STATE_WAITING_ENTRY);
			return;
		}

		if (gpio_read_pin(PM_RSMRST) > 0) {
			LOG_DBG("PG3 Wake triggered");
			pseudo_g3_set_state(PG3_STATE_WAKE_WAIT);
			return;
		}
	default:
		break;
	}
	/* Default continue in PG3_STATE_ENTERED */
}

static void pg3_handle_state_wake_wait(void)
{
	/* Move to PG3_STATE_IDLE if system transition to S0/S3.
	 * else continue in PG3_STATE_WAKE_WAIT.
	 */
	switch (pwrseq_system_state()) {
	case SYSTEM_S0_STATE:
	case SYSTEM_S3_STATE:
	case SYSTEM_G3_STATE:
		pseudo_g3_set_state(PG3_STATE_IDLE);
		break;
	default:
		break;
	}
	/* Default continue in PG3_STATE_WAKE_WAIT */
}

static void manage_pseudog3_states(void)
{
	switch (pg3_state) {
	case PG3_STATE_DISABLED:
		pg3_handle_state_disabled();
		break;
	case PG3_STATE_IDLE:
		pg3_handle_state_idle();
		break;
	case PG3_STATE_WAITING_ENTRY:
		pg3_handle_state_waiting_entry();
		break;
	case PG3_STATE_ENTERED:
		pg3_handle_state_entered();
		/* Host (BIOS) clears PG3 entry count bit after reading it */
		g_acpi_tbl.acpi_concept_flags1.pg3_entered_successfully = 1;
		break;
	case PG3_STATE_WAKE_WAIT:
		pg3_handle_state_wake_wait();
		/* Host (BIOS) clears PG3 exit count bit after reading it */
		g_acpi_tbl.acpi_concept_flags1.pg3_exit_after_counter_exp = 1;
		break;
	default:
		LOG_ERR("Invalid PG3 state, move to PG3_STATE_DISABLED");
		pseudo_g3_set_state(PG3_STATE_DISABLED);
		break;
	}
}

void pseudo_g3_enable(bool status)
{
	pg3_enable_status = status;
	LOG_DBG("pg3_enable_status:%d", status);
}

bool pseudo_g3_get_state(void)
{
	return (pg3_state == PG3_STATE_ENTERED);
}

bool pseudo_g3_get_prev_state(void)
{
	return (pg3_prev_state == PG3_STATE_ENTERED);
}

void manage_pseudog3(void)
{
	manage_pseudog3_states();

	if (pg3_generate_wake) {
		pg3_generate_wake = false;
		pseudo_g3_trigger_exit();
	}
}
