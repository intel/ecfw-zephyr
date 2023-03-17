/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include "port80display.h"
#include "board.h"
#include "board_config.h"
#include "smchost.h"
#include "smchost_commands.h"
#include "scicodes.h"
#include "sci.h"
#include "acpi.h"
#include "pwrplane.h"
#include "pwrseq_utils.h"
#include "dswmode.h"
#include "pseudog3.h"
#include "kbchost.h"
#include "espi_hub.h"
#ifdef CONFIG_DNX_EC_ASSISTED_TRIGGER_SMC
#include "dnx_ec_assisted_trigger.h"
#endif
LOG_MODULE_DECLARE(smchost, CONFIG_SMCHOST_LOG_LEVEL);

static bool pwrbtn_notify;
static uint8_t legacy_wake_status;
static bool cs_low_pwr_mode;
static bool cs_state;
#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC
static uint8_t prev_led_values;
#endif
static uint8_t pln_timer_cnt;
#ifdef EC_M_2_SSD_PLN
static void timer_pwrbtn_pln(struct k_timer *timer);
K_TIMER_DEFINE(pln_timer, timer_pwrbtn_pln, NULL);
static uint8_t pln_pin_sts;
#endif

bool smchost_is_system_in_cs(void)
{
	return cs_state;
}

void smchost_pwrbtn_handler(uint8_t pwrbtn_sts)
{
	LOG_INF("%s notify: %d acpi_mode: %d state: %d", __func__,
		pwrbtn_notify, g_acpi_state_flags.acpi_mode,
		pwrseq_system_state());

	g_acpi_tbl.acpi_flags2.pwr_btn = pwrbtn_sts;

	if (!check_btn_sci_sts(HID_BTN_SCI_PWR))
		return;

	/* Power button press/release only in ACPI mode */
	if (g_acpi_state_flags.acpi_mode) {
		if (pwrbtn_sts) {
			enqueue_sci(SCI_PWRBTN_UP);
		} else {
			enqueue_sci(SCI_PWRBTN_DOWN);
		}
	}

	switch (pwrseq_system_state()) {
	case SYSTEM_S3_STATE:
	case SYSTEM_S4_STATE:
		legacy_wake_status |= BIT(WAKE_HID_EVENT_BIT);
		if (pwrbtn_notify) {
			enqueue_sci(SCI_PWRBTN);
		}
		break;
	case SYSTEM_S0_STATE:
		enqueue_sci(SCI_PWRBTN);
		break;
	case SYSTEM_G3_STATE:
	case SYSTEM_S5_STATE:
		break;
	}
}

#ifdef EC_M_2_SSD_PLN
void set_pln_pin_sts(uint8_t sts)
{
	/* updating PLN pin status for deferred processing
	 * 0 - PLN pin low
	 * 1 - PLN pin high
	 * 2 - PLN Pin no change
	 */
	pln_pin_sts = sts;

#ifdef CONFIG_SMCHOST_EVENT_DRIVEN_TASK
	if ((sts == PLN_PIN_ASSERT) ||
	    (sts == PLN_PIN_DEASSERT)) {
		smchost_signal_request();
	}
#endif
}

uint8_t get_pln_pin_sts(void)
{
	return pln_pin_sts;
}

void manage_pln_signal(void)
{
	/* update pln signal, State should be assert/deassert */
	if (get_pln_pin_sts() != PLN_PIN_NC) {
		gpio_write_pin(EC_M_2_SSD_PLN, get_pln_pin_sts());
		set_pln_pin_sts(PLN_PIN_NC);
	}
}

/* PLN - Power loss notification is indication to SSD that system is about to
 * lose power, SSD must stop all other activity and start preparing for
 * power loss. This will prevent SSD from going into recovery.
 * EC is covering one of the nongraceful shutdown case using power button.
 * EC notifies SSD before shutdown due to power button press.
 * EC should notify SSD 2 seconds before shutdown. 2 seconds is the time taken
 * by the SSD to do some housekeeping (calculated by storage team).
 * BIOS pass force shutdown timer value to EC (since it is programmable in pmc)
 * based on that we create timer to start notifying SSD.
 */
static void timer_pwrbtn_pln(struct k_timer *timer)
{
	uint8_t plt_rst;

	LOG_INF("PLN timer expired");

	espihub_retrieve_vw(ESPI_VWIRE_SIGNAL_PLTRST, &plt_rst);
	if (plt_rst) {
		set_pln_pin_sts(PLN_PIN_ASSERT);
	}
}

void smchost_pwrbtn_pln_handler(uint8_t pwrbtn_sts)
{
	LOG_INF("%s: %d state: %d", __func__, pln_timer_cnt, pwrbtn_sts);

	if (pwrbtn_sts) {
		set_pln_pin_sts(PLN_PIN_DEASSERT);
		/* Stop PLN timer, if not running no effect */
		k_timer_stop(&pln_timer);
	} else {
		if (pln_timer_cnt) {
			k_timer_start(&pln_timer, K_SECONDS(pln_timer_cnt),
					K_NO_WAIT);
		}
	}
}

void smchost_pln_pltreset_handler(void)
{
	/* deassert pln on pltreset deassertion */
	set_pln_pin_sts(PLN_PIN_DEASSERT);
	/* Stop PLN timer, if not running no effect */
	k_timer_stop(&pln_timer);
}
#endif

static void sx_entry(void)
{
	g_acpi_state_flags.sci_enabled = 0;
}

static void sx_exit(void)
{
	g_acpi_state_flags.sci_enabled = 1;
}

static void change_dsw_mode(void)
{
	dsw_update_mode(host_req[1]);
}

static void change_pg3_mode(void)
{
	pseudo_g3_enable(host_req[1]);
}

static void pg3_prog_counter(void)
{
	pseudo_g3_program_counter(g_acpi_tbl.acpi_pg3_counter,
				  g_acpi_tbl.acpi_pg3_wake_timer);
}

static void config_ssd_pln(void)
{
	LOG_DBG("%s: PLN Count %02x", __func__, host_req[1]);

	if (host_req[1] > MINIMUM_PLN_TIME) {
		pln_timer_cnt = host_req[1] - MINIMUM_PLN_TIME;
	} else {
		/* Invalid PLN timer value from host */
		LOG_ERR("Invalid pln time");
		pln_timer_cnt = 0;
	}
}

static void retrieve_dsw_mode(void)
{
	uint8_t data = dsw_mode();

	send_to_host(&data, sizeof(data));
}

static void enable_cs_lpm_mode(void)
{
	cs_low_pwr_mode = host_req[1] ? true : false;
}

static void cs_entry(void)
{
	LOG_WRN("%s %d", __func__, cs_low_pwr_mode);
	if (!cs_low_pwr_mode) {
		return;
	}

#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC
	prev_led_values = kbc_get_leds();

	if (g_acpi_tbl.acpi_concept_flags1.acpi_cs_dbg_led_en) {
		kbc_set_leds(BIT(CAPS_LOCK_POS));
	}
#endif
	cs_state = true;
}

static void cs_exit(void)
{
	LOG_WRN("%s", __func__);
#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC
	if (g_acpi_tbl.acpi_concept_flags1.acpi_cs_dbg_led_en) {
		kbc_set_leds(prev_led_values);
	}
#endif
	cs_state = false;

#ifdef CONFIG_THERMAL_MANAGEMENT
	thermalmgmt_handle_cs_exit();
#endif
}

static void get_legacy_wake_sts(void)
{
	send_to_host(&legacy_wake_status, 1);
}

static void clear_legacy_wake_sts(void)
{
	legacy_wake_status = 0;
}

void smchost_cmd_pm_handler(uint8_t command)
{
	switch (command) {
	case SMCHOST_PLN_CONFIG:
		config_ssd_pln();
		break;
	case SMCHOST_ENABLE_PWR_BTN_NOTIFY:
		pwrbtn_notify = true;
		break;
	case SMCHOST_DISABLE_PWR_BTN_NOTIFY:
		pwrbtn_notify = false;
		break;
	case SMCHOST_ENABLE_PWR_BTN_SW:
		g_pwrflags.pwr_sw_enabled = 1;
		break;
	case SMCHOST_DISABLE_PWR_BTN_SW:
		g_pwrflags.pwr_sw_enabled = 0;
		break;
	case SMCHOST_GET_LEGACY_WAKE_STS:
		get_legacy_wake_sts();
		break;
	case SMCHOST_CLEAR_LEGACY_WAKE_STS:
		clear_legacy_wake_sts();
		break;
	case SMCHOST_SX_ENTRY:
		sx_entry();
		break;
	case SMCHOST_SX_EXIT:
		sx_exit();
		break;
	case SMCHOST_SET_DSW_MODE:
		change_dsw_mode();
		break;
	case SMCHOST_GET_DSW_MODE:
		retrieve_dsw_mode();
		break;
	case SMCHOST_PG3_SET_MODE:
		change_pg3_mode();
		break;
	case SMCHOST_PG3_PROG_COUNTER:
		pg3_prog_counter();
		break;
	case SMCHOST_CS_LOW_PWR_MODE_SET:
		enable_cs_lpm_mode();
		break;
	case SMCHOST_CS_ENTRY:
		cs_entry();
		break;
	case SMCHOST_CS_EXIT:
		cs_exit();
		break;
#ifdef CONFIG_DNX_EC_ASSISTED_TRIGGER_SMC
	/* Set DnX strap and trigger cold restart, requires eSPI OOB support */
	case SMCHOST_DNX_TRIGGER:
		dnx_soc_handshake();
		dnx_ec_assisted_restart();
		break;
	/* Set DnX strap only, requires manual restart */
	case SMCHOST_DNX_SET_STRAP:
		dnx_soc_handshake();
		break;
#endif /* CONFIG_DNX_EC_ASSISTED_TRIGGER_SMC */
	case SMCHOST_RESET_KSC:
		ec_reset();
		break;
	default:
		LOG_WRN("%s: command 0x%X without handler", __func__, command);
		break;
	}
}
