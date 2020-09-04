/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SMC_H__
#define __SMC_H__

#include <soc.h>
#include "acpi_region.h"

/** Number of loops to wait for host in burst */
#define BURST_TIMEOUT        20ul

/**
 * @brief Thermal sensor list index in ACPI table.
 */
enum acpi_thrm_sens_idx {
	ACPI_THRM_SEN_PCH,
	ACPI_THRM_SEN_SKIN,
	ACPI_THRM_SEN_AMBIENT,
	ACPI_THRM_SEN_VR,
	ACPI_THRM_SEN_DDR,
	ACPI_THRM_SEN_TOTAL,
	ACPI_THERM_SEN_UNDEF,
};

/**
 * @brief Wake reasons for BIOS.
 */
enum wake_reason_bios {
	/* Wake due to power button or HID Event */
	WAKE_HID_EVENT = BIT(0),
	/* Wake due to S3 timeout */
	WAKE_S3_TIMEOUT = BIT(1),
};

/**
 * @brief Wake reasons to be reported to OS.
 */
enum wake_reason {
	/* Low battery event */
	WAKE_LOW_BATTERY = BIT(0),
	/* Lid change event */
	WAKE_LID_EVENT = BIT(1),
	/* Keyboard event */
	WAKE_KBC_EVENT = BIT(2),
	/* Timer event */
	WAKE_TIMER_EVENT = BIT(3),
	/* PCIe device event */
	WAKE_PCIE_EVENT = BIT(4),
	/* LAN module event */
	WAKE_LAN_EVENT = BIT(5),
	/* Sensor hub event */
	WAKE_SENSOR_EVENT = BIT(6),
	/* Home button event */
	WAKE_HOME_BUTTON_EVENT = BIT(7),
};


void smc_init(void);

/**
 * @brief Generates a wake event via SCI.
 */
void smc_generate_wake(u8_t wake_reason);
void smc_clear_wake_sts(void);
u8_t smc_get_wake_sts(void);

/**
 * @brief Update the thermal sensor temperature value in ACPI table.
 *
 * @param idx Index in the list of thermal sensors defined in ACPI space.
 * @param temp temperature value in order of magnitude 10 degree celsius.
 */
void smc_update_thermal_sensor(enum acpi_thrm_sens_idx idx, s16_t temp);

/**
 * @brief Update the fan rpm (rotation per minute) value for given fan device.
 *
 * @param fan_idx fan device index.
 * @param rpm rotation per minunte value of the fan.
 */
void smc_update_fan_tach(u8_t fan_idx, u16_t rpm);

/**
 * @brief Update thermal sensor trip status.
 *
 * @param status is bit-field value indicating sensor number tripped.
 *	  e.g. status = 0x5 = 0000 0101b -> indicates, sensor number 1
 *	  and sensor number 3 got tripped.
 *
 */
void smc_update_therm_trip_status(u16_t status);

/**
 * @brief Update cpu temperature value to ACPI offset acpi_remote_temp.
 *
 * @param temp cpu temperature in degree celsius.
 */
void smc_update_cpu_temperature(int temp);

/**
 * @brief Check write permissions for ACPI offset.
 *
 * @param offset acpi table offset.
 * @return 1 if offset has write permissions, otherwise 0.
 */
bool smc_is_acpi_offset_write_permitted(u8_t offset);

#endif /* __SMC_H__ */
