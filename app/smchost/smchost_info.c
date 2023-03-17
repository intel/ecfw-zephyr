/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#include "board.h"
#include "board_config.h"
#include "smc.h"
#include "smchost.h"
#include "smchost_commands.h"
#include "pwrplane.h"
#include "periphmgmt.h"
#include "espi_hub.h"
#include "system.h"
#include "flashhdr.h"

LOG_MODULE_DECLARE(smchost, CONFIG_SMCHOST_LOG_LEVEL);

static struct acpi_hid_btn_sci btn_ctrl;

uint8_t check_btn_sci_sts(uint8_t btn_sci_en_dis)
{
	uint8_t ret = 0;

	switch (btn_sci_en_dis) {
	case HID_BTN_SCI_PWR:
		ret = btn_ctrl.pwr_btn_en_dis;
		break;
	case HID_BTN_SCI_VOL_UP:
		ret = btn_ctrl.vol_up_en_dis;
		break;
	case HID_BTN_SCI_VOL_DOWN:
		ret = btn_ctrl.vol_down_en_dis;
		break;
	case HID_BTN_SCI_HOME:
		ret = btn_ctrl.win_btn_en_dis;
		break;
	case HID_BTN_SCI_ROT_LOCK:
		ret = btn_ctrl.rot_lock_en_dis;
		break;
	default:
		break;
	}

	return ret;
}

static void btn_sci_cntrl(void)
{
	btn_ctrl = g_acpi_tbl.acpi_btn_cntrl;
}

#ifdef CONFIG_DEPRECATED_SMCHOST_CMD
static void query_system_status(void)
{
	uint8_t system_sts = 0x00;

	system_sts |= BIT(4);
	send_to_host(&system_sts, 1);
}
#endif

/**
 * @brief Returns a key to identify SMC and state information.
 *
 * To be used by test manufacturing software. Output
 *  Byte 0 - 2: "KSC"
 *  Byte 3 Bit 7: 1 = Geyserville support enabled
 *  Bit 6: 1 = Thermal states locked
 *  Bit 5: 1 = Extended thermal sensor supported
 *  Bit 4: 1 = MAF mode
 *  Bit 3: 1 = SAF Mode
 *  Bit 2: reserved for future use.
 *  Bit 1: 1 = PECI access mode
 *  Bit 0: 1 = ACPI mode
 */
static void smc_mode(void)
{
	uint8_t res[] = {'K', 'S', 'C', 0};

	res[SMC_CAPS_INDEX] |= BIT(GEYSERVILLE_SUPPORT);
	res[SMC_CAPS_INDEX] |= BIT(LEGACY_SUPPORT);

	/* Encode boot config */
	switch (espihub_boot_mode()) {
	case FLASH_BOOT_MODE_MAF:
		res[SMC_CAPS_INDEX] |= BIT(BOOT_CONFIG_MAF);
		break;
	case FLASH_BOOT_MODE_SAF:
		res[SMC_CAPS_INDEX] |= BIT(BOOT_CONFIG_SAF);
		break;
	default:
		/* Dedicated flash / G3 flash sharing */
		break;
	}

	if (g_acpi_state_flags.acpi_mode) {
		res[SMC_CAPS_INDEX] |= BIT(ACPI_MODE);
	}

#ifdef CONFIG_THERMAL_MANAGEMENT
	if (peci_access_mode == PECI_OVER_ESPI_MODE) {
		res[SMC_CAPS_INDEX] |= BIT(PECI_ACCESS_MODE_POS);
	}
#endif

	send_to_host(res, sizeof(res));
}

/**
 * @brief Returns a switch status.
 *
 *  Bit 5: Virtual Dock
 *  Bit 4: AC power
 *  Bit 3: Home button
 *  Bit 2: NMI is active
 *  Bit 1: Virtual battery
 *  Bit 0: Lid closed in legacy BIOS
 */
static void get_switch_status(void)
{
	uint8_t sw_status = read_io_switch_status();

	send_to_host(&sw_status, 1);
	LOG_DBG("%s sw_status:0x%X", __func__, sw_status);
}

static void smc_get_fab_id(void)
{
	uint16_t platform_id = get_platform_id();

	send_to_host((uint8_t *)&platform_id, sizeof(platform_id));
}

static void read_revision(void)
{
	uint8_t version[4] = {0};

	version[0] = major_version();
	version[1] = minor_version();
	version[2] = patch_id();
	version[3] = qs_build_version();

	send_to_host((uint8_t *)&version, 4);
}

static void read_platform_signature(void)
{
	uint8_t value[8];

	send_to_host((uint8_t *)value, 8);
}

static void get_shutdown_reason(void)
{
	uint8_t shutdown_status = read_shutdown_reason();

	send_to_host((uint8_t *)&shutdown_status, sizeof(shutdown_status));
}


void smchost_cmd_info_handler(uint8_t command)
{
	switch (command) {
#ifdef CONFIG_DEPRECATED_SMCHOST_CMD
	case SMCHOST_QUERY_SYSTEM_STS:
		query_system_status();
		break;
#endif
	case SMCHOST_GET_PSR_SHUTDOWN_REASON:
		get_shutdown_reason();
		/* Clear shutodown reason */
		set_shutdown_reason(SHUTDOWN_REASON_DEFAULT);
		break;
	case SMCHOST_GET_SMC_MODE:
		smc_mode();
		break;
	case SMCHOST_GET_SWITCH_STS:
		get_switch_status();
		break;
	case SMCHOST_GET_FAB_ID:
		smc_get_fab_id();
		break;
	case SMCHOST_READ_PLAT_SIGNATURE:
		read_platform_signature();
		break;
	case SMCHOST_READ_REVISION:
		read_revision();
		break;
	case SMCHOST_HID_BTN_SCI_CONTROL:
		btn_sci_cntrl();
		break;
	default:
		LOG_WRN("%s: command 0x%X without handler", __func__, command);
		break;
	}
}
