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

#include "espi_hub.h"
#include "system.h"
#include "flashhdr.h"

LOG_MODULE_DECLARE(smchost, CONFIG_SMCHOST_LOG_LEVEL);

static void query_system_status(void)
{
	u8_t system_sts = 0x00;

	system_sts |= BIT(4);
	send_to_host(&system_sts, 1);
}


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
 *  Bit 1-2: reserved for future use.
 *  Bit 0: 1 = ACPI mode
 */
static void smc_mode(void)
{
	u8_t res[] = {'K', 'S', 'C', 0};

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

	send_to_host(res, sizeof(res));
}

static void get_dock_status(void)
{
	u8_t dock_sts;

	dock_sts = 0x00;
	send_to_host(&dock_sts, 1);
}

static void get_switch_status(void)
{
	/* TODO: Replace when real value is available */
	u8_t sw_status = 0x93;

	send_to_host(&sw_status, 1);
}

static void smc_get_fab_id(void)
{
	u16_t platform_id = get_platform_id();

	send_to_host((u8_t *)&platform_id, 2);
}


static void read_revision(void)
{
	u8_t version[2] = {0};

	version[0] = major_version();
	version[1] = minor_version();

	send_to_host((u8_t *)&version, 2);
}

static void read_platform_signature(void)
{
	u8_t value[8];

	send_to_host((u8_t *)value, 8);
}

static void get_ec_id(void)
{
	u8_t data = 0xC2;

	send_to_host(&data, 1);
}

static void get_pmic_vid(void)
{
	u8_t pmic_vid = 0x00;

#ifdef CONFIG_PMIC
	read_pmic_vid(&pmic_vid);
#endif
	send_to_host(&pmic_vid, 1);
}


void ucsi_read_version(void)
{
	u8_t res[] = {0x01, 0x00};

	send_to_host(res, 2);
}


void smchost_cmd_info_handler(u8_t command)
{
	switch (command) {
	case SMCHOST_QUERY_SYSTEM_STS:
		query_system_status();
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
	case SMCHOST_GET_PMIC_VID:
		get_pmic_vid();
		break;
	case SMCHOST_GET_DOCK_STS:
		get_dock_status();
		break;
	case SMCHOST_READ_REVISION:
		read_revision();
		break;
	case SMCHOST_UCSI_READ_VERSION:
		ucsi_read_version();
		break;
	case SMCHOST_GET_KSC_ID:
		get_ec_id();
		break;
	default:
		LOG_WRN("%s: command 0x%X without handler", __func__, command);
		break;
	}
}
