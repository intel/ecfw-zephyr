/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#include "board.h"
#include "board_config.h"
#include "smc.h"
#include "smchost.h"
#include "smchost_commands.h"
#include "scicodes.h"
#include "sci.h"
#include "acpi.h"
#include "pwrplane.h"
#include "dswmode.h"
#include "kbchost.h"
LOG_MODULE_DECLARE(smchost, CONFIG_SMCHOST_LOG_LEVEL);

static u8_t sys_pwr_state;
static bool pwrbtn_notify;
static u8_t legacy_wake_status;
static bool cs_state;
#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC
static u8_t prev_led_values;
#endif

bool smchost_is_system_in_cs(void)
{
	return cs_state;
}

void smchost_pwrbtn_handler(u8_t pwrbtn_sts)
{
	LOG_INF("%s notify: %d acpi_mode: %d state: %d", __func__,
		pwrbtn_notify, g_acpi_state_flags.acpi_mode,
		pwrseq_system_state());

	g_acpi_tbl.acpi_flags2.pwr_btn = pwrbtn_sts;

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
		legacy_wake_status |= WAKE_HID_EVENT;
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

static void suspend_smc(void)
{
	u8_t ack = 0x01;

	send_to_host(&ack, 1);
}

static void resume_smc(void)
{
	/* TODO: Check why nothing is done in current EC */
}

static void sx_entry(void)
{
	g_acpi_state_flags.sci_enabled = 0;
}

static void sx_exit(void)
{
	g_acpi_state_flags.sci_enabled = 1;
}

static void syspwr_state_change(void)
{
	sys_pwr_state = host_req[1];
}

static void change_dsw_mode(void)
{
	dsw_update_mode(host_req[1]);
}

static void retrieve_dsw_mode(void)
{
	u8_t data = dsw_mode();

	send_to_host(&data, sizeof(data));
}

static void cs_entry(void)
{
#ifdef CONFIG_PWRMGMT_DEEP_IDLE
	pwrmgmt_request_enter_idle(CONFIG_PWRMGMT_DEEP_IDLE_ENTRY_DLY);
#endif
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
#ifdef CONFIG_PWRMGMT_DEEP_IDLE
	pwrmgmt_exit_idle();
#endif
#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC
	if (g_acpi_tbl.acpi_concept_flags1.acpi_cs_dbg_led_en) {
		kbc_set_leds(prev_led_values);
	}
#endif
	cs_state = false;
}

static void get_legacy_wake_sts(void)
{
	send_to_host(&legacy_wake_status, 1);
}

static void clear_legacy_wake_sts(void)
{
	legacy_wake_status = 0;
}

static void read_wake_sts(void)
{
	u8_t temp = smc_get_wake_sts();

	send_to_host(&temp, 1);
}

static void clear_wake_sts(void)
{
	smc_clear_wake_sts();
}


void smchost_cmd_pm_handler(u8_t command)
{
	switch (command) {
	case SMCHOST_ENABLE_PWR_BTN_NOTIFY:
		pwrbtn_notify = true;
		break;
	case SMCHOST_DISABLE_PWR_BTN_NOTIFY:
		pwrbtn_notify = false;
		break;
	case SMCHOST_SYSTEM_POWER_OFF:
		/* TODO: Implement handler */
		break;
	case SMCHOST_ENABLE_PWR_BTN_SW:
		g_pwrflags.pwr_sw_enabled = 1;
		break;
	case SMCHOST_DISABLE_PWR_BTN_SW:
		g_pwrflags.pwr_sw_enabled = 0;
		break;
	case SMCHOST_ENABLE_SOFT_PWR_BTN:
		g_pwrflags.pwr_sw_suspend_resume = 1;
		break;
	case SMCHOST_DISABLE_SOFT_PWR_BTN:
		g_pwrflags.pwr_sw_suspend_resume = 0;
		break;
	case SMCHOST_GET_LEGACY_WAKE_STS:
		get_legacy_wake_sts();
		break;
	case SMCHOST_CLEAR_LEGACY_WAKE_STS:
		clear_legacy_wake_sts();
		break;
	case SMCHOST_SUSPEND_SMC:
		suspend_smc();
		break;
	case SMCHOST_RESUME_SMC:
		resume_smc();
		break;
	case SMCHOST_UPDATE_SYSPWR_STATE:
		syspwr_state_change();
		break;
	case SMCHOST_READ_WAKE_STS:
		read_wake_sts();
		break;
	case SMCHOST_CLEAR_WAKE_STS:
		clear_wake_sts();
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
	case SMCHOST_CS_ENTRY:
		cs_entry();
		break;
	case SMCHOST_CS_EXIT:
		cs_exit();
		break;
	case SMCHOST_RESET_KSC:
	default:
		LOG_WRN("%s: command 0x%X without handler", __func__, command);
		break;
	}
}
