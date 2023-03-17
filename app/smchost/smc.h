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
#define WAKE_HID_EVENT_BIT	0
#define WAKE_S3_TIMEOUT_BIT	1

/**
 * @brief Thermal sensor list index in ACPI table.
 */
enum acpi_thrm_sens_idx {
	/* Sen 1: pch */
	ACPI_THRM_SEN_1,
	/* Sen 2: skin */
	ACPI_THRM_SEN_2,
	/* Sen 3: Ambient */
	ACPI_THRM_SEN_3,
	/* Sen 4: VR */
	ACPI_THRM_SEN_4,
	/* Sen 5: DDR */
	ACPI_THRM_SEN_5,

	ACPI_THRM_SEN_TOTAL,
	ACPI_THERM_SEN_UNDEF = 0xFF,
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
void smc_generate_wake(uint8_t wake_reason);
void smc_clear_wake_sts(void);
uint8_t smc_get_wake_sts(void);

/**
 * @brief Update the thermal sensor temperature value in ACPI table.
 *
 * @param idx Index in the list of thermal sensors defined in ACPI space.
 * @param temp temperature value in order of magnitude 10 degree celsius.
 */
void smc_update_thermal_sensor(enum acpi_thrm_sens_idx idx, int16_t temp);

/**
 * @brief Update fan capability value in ACPI table.
 *
 * @param fan_capability_val byte value for all bit-fields of @ref
 *	  fan_capability_support.
 */
void smc_update_fan_capability(uint8_t fan_capability_val);

/**
 * @brief Update the fan rpm (rotation per minute) value for given fan device.
 *
 * @param fan_idx fan device index.
 * @param rpm rotation per minunte value of the fan.
 */
void smc_update_fan_tach(uint8_t fan_idx, uint16_t rpm);

/**
 * @brief Update thermal sensor trip status.
 *
 * @param status is bit-field value indicating sensor number trip change event.
 *	  e.g. status = 0x5 = 0000 0101b -> indicates, sensor number 1
 *	  and sensor number 3 got changes in trip status.
 *
 */
void smc_update_therm_trip_status(uint16_t status);

/**
 * @brief Update fan RPM sensor trip status.
 *
 * @param status is bit-field value indicating fan RPM number trip change event.
 *	  e.g. status = 0x5 = 0000 0101b -> indicates, fan number 1
 *	  and fan number 3 have change in RPM trip status.
 */
void smc_update_rpm_trip_status(uint8_t status);

/**
 * @brief Update cpu temperature value to ACPI offset acpi_remote_temp.
 *
 * @param temp cpu temperature in degree celsius.
 */
void smc_update_cpu_temperature(int temp);
void smc_update_gpu_temperature(int temp);

/**
 * @brief Update pch dts temperature value to ACPI offset
 *
 * @param temp pch temperature in degree celsius.
 */
void smc_update_pch_dts_temperature(int temp);

/**
 * @brief Check write permissions for ACPI offset.
 *
 * @param offset acpi table offset.
 * @return 1 if offset has write permissions, otherwise 0.
 */
bool smc_is_acpi_offset_write_permitted(uint8_t offset);

#endif /* __SMC_H__ */
