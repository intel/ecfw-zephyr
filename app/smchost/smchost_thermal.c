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
#include "scicodes.h"
#include "sci.h"
#include "acpi.h"
#include "thermalmgmt.h"

LOG_MODULE_DECLARE(smchost, CONFIG_SMCHOST_LOG_LEVEL);

/**
 * @brief Get the acpi fan idx object
 *
 * Host sends bit field value for fan index. This function converts it to local defined enumeration.
 *
 * @return enum fan_type
 */
static inline enum fan_type get_acpi_fan_idx(void)
{
	switch (g_acpi_tbl.acpi_fan_idx) {
	case BIT(FAN_CPU):
		return FAN_CPU;
	case BIT(FAN_REAR):
		return FAN_REAR;
	case BIT(FAN_GFX):
		return FAN_GFX;
	case BIT(FAN_PCH):
		return FAN_PCH;

	default: /* Any other value defaulted to CPU fan */
		LOG_WRN("Host to ensure correct fan index used");
		return FAN_CPU;
	}
}

static void set_shutdown_threshold(void)
{
	host_update_crit_temp(host_req[1]);
}

static void set_os_active_trip(void)
{
	host_set_bios_bsod_override(host_req[1], host_req[2]);
}


static void update_pwm(void)
{
#ifndef CONFIG_THERMAL_FAN_OVERRIDE
	host_update_fan_speed(get_acpi_fan_idx(), g_acpi_tbl.acpi_pwm_end_val);
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
