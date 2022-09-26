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
#include "thermalmgmt.h"
#ifdef CONFIG_DTT_SUPPORT_THERMALS
#include "dtt.h"
#endif

LOG_MODULE_DECLARE(smchost, CONFIG_SMCHOST_LOG_LEVEL);

#define ACPI_DTT_SENSOR_IDX_MASK	0x0F
#define ACPI_DTT_HYSTERESIS_MASK	0xF0
#define ACPI_DTT_HYSTERESIS_OFFSET	4u
#define ACPI_DTT_HYSTERESIS_MULTIPLIER	10u

static void set_shutdown_threshold(void)
{
	host_update_crit_temp(host_req[1]);
}

static void set_os_active_trip(void)
{
	host_set_bios_bsod_override(host_req[1], host_req[2]);
}

#ifdef CONFIG_DTT_SUPPORT_THERMALS
static void dtt_set_tmp_threshold(void)
{
	uint8_t sns, hyst;
	struct dtt_threshold thrd;

	sns = g_acpi_tbl.acpi_temp_snsr_select & ACPI_DTT_SENSOR_IDX_MASK;

	hyst = g_acpi_tbl.acpi_temp_snsr_select & ACPI_DTT_HYSTERESIS_MASK;
	hyst = (hyst >> ACPI_DTT_HYSTERESIS_OFFSET);
	thrd.temp_hyst = hyst * ACPI_DTT_HYSTERESIS_MULTIPLIER;

	thrd.low_temp = g_acpi_tbl.acpi_temp_low_thrshld;
	thrd.high_temp = g_acpi_tbl.acpi_temp_high_thrshld;

	smc_update_dtt_threshold_limits(sns, thrd);
}
#endif /* CONFIG_DTT_SUPPORT_THERMALS */

static void update_pwm(void)
{
#ifndef CONFIG_THERMAL_FAN_OVERRIDE

	switch (g_acpi_tbl.acpi_fan_idx) {
	case FAN_CPU:	/* For sake of backward compatibility, CPU fan allowed to run at index 0*/
	case BIT(FAN_CPU):
		host_update_fan_speed(FAN_CPU, g_acpi_tbl.acpi_pwm_end_val);
		break;
	case BIT(FAN_REAR):
		host_update_fan_speed(FAN_REAR, g_acpi_tbl.acpi_pwm_end_val);
		break;
	case BIT(FAN_GFX):
		host_update_fan_speed(FAN_GFX, g_acpi_tbl.acpi_pwm_end_val);
		break;
	case BIT(FAN_PCH):
		host_update_fan_speed(FAN_PCH, g_acpi_tbl.acpi_pwm_end_val);
		break;
	default:
		LOG_WRN("Invalid fan index, BIOS must send only one bit set per fan index");
		break;
	}

#endif
}

static void update_pwm_with_override(uint8_t speed)
{
#ifndef CONFIG_THERMAL_FAN_OVERRIDE
	host_set_bios_fan_override(speed ? 1 : 0, speed);
	host_update_fan_speed(FAN_CPU, speed);
#endif
}

static void update_hw_peripherals_status(void)
{
	uint8_t hw_peripherals_sts[] = {0x0, 0x0};

	/* Get hw status */
	get_hw_peripherals_status(hw_peripherals_sts);

	/* Send hw peripherals status to BIOS */
	send_to_host(hw_peripherals_sts, sizeof(hw_peripherals_sts));
}

void smchost_cmd_thermal_handler(uint8_t command)
{
	switch (command) {
	case SMCHOST_SET_OS_ACTIVE_TRIP:
		set_os_active_trip();
		break;
#ifdef CONFIG_DTT_SUPPORT_THERMALS
	case SMCHOST_SET_TMP_THRESHOLD:
		dtt_set_tmp_threshold();
		break;
#endif /* CONFIG_DTT_SUPPORT_THERMALS */
	case SMCHOST_SET_SHDWN_THRESHOLD:
		set_shutdown_threshold();
		break;
	case SMCHOST_UPDATE_PWM:
		update_pwm();
		break;
	case SMCHOST_BIOS_FAN_CONTROL:
		update_pwm_with_override(host_req[1]);
		break;
	case SMCHOST_GET_HW_PERIPHERALS_STS:
		update_hw_peripherals_status();
		break;
	default:
		LOG_WRN("%s: command 0x%X without handler", __func__, command);
		break;
	}
}
