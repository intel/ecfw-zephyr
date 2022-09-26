/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __THERMAL_MGMT_H__
#define __THERMAL_MGMT_H__

#include "fan.h"
#include "smc.h"

#define CPU_TEMP_ALERT_DELTA			3u
/* Fail safe threshold */
#define THERM_SHTDWN_THRSD			103u
/* EC tolerance range 3 deg */
#define THERM_SHTDWN_EC_TOLERANCE		3u

struct therm_bsod_override_thrsd_acpi {
	/* Temp to override during BSOD */
	uint8_t temp_bsod_override;
	/* Fan speed to override during BSOD */
	uint8_t fan_bsod_override;
	/* Flag for EC to handle BSOD */
	bool is_bsod_setting_en;
	/* Flag to store bsod temp status */
	bool is_bsod_temp_crossed;
};

/* FAN override settings */
#define FAN_OVERRIDE_ACPI		75u
#define TEMP_OVERRIDE_ACPI		85u
#define TEMP_BSOD_FAN_OFF		40u

/**
 * @brief Thermal management task.
 *
 * This routine manages:
 * - Driving the fan
 * - Reading the fan speed through Tach
 * - Reading thermal sensors over ADC
 * - Reading the CPU temperature over PECI or PECI over eSPI channel.
 *
 * @param p1 pointer to additional task-specific data.
 * @param p2 pointer to additional task-specific data.
 * @param p2 pointer to additional task-specific data.
 *
 */
void thermalmgmt_thread(void *p1, void *p2, void *p3);

/**
 * @brief Host API to allow BIOS set fan control override in Non-ACPI mode.
 *
 * By default, when system is not in ACPI mode, fan is self controlled by EC
 * based on local thermal policy. This API allows BIOS to gain CPU fan control
 * when system in non acpi mode.
 *
 * @param en 1 to enable BIOS fan override, otherwise 0.
 * @param duty_cycle fan duty cycle set by bios.
 */
void host_set_bios_fan_override(bool en, uint8_t duty_cycle);

/**
 * @brief Host API to allow BIOS to update critical CPU temperature.
 *
 * @param critical temperature set by bios.
 */
void host_update_crit_temp(uint8_t crit_temp);

/**
 * @brief Host API to update fan speed.
 *
 * This is ACPI hook for host to update fan duty cycle for selected fan device.
 *
 * @param idx fan device index.
 * @param duty_cycle duty cycle.
 */
void host_update_fan_speed(enum fan_type idx, uint8_t duty_cycle);

/**
 * @brief API for host to update OS BSOD and fan thresholds.
 *
 * @param temp_bsod_override_val bsod override value from host.
 * @param fan_bsod_override_val fan override value from host.
 */
void host_set_bios_bsod_override(uint8_t temp_bsod_override_val,
	uint8_t fan_bsod_override_val);

/**
 * @brief API for SMC host to start the peci delay timer.
 *
 * PECI is prone to fail during boot to S0. Hence peci is not accessed
 * for a prescribed time duration.
 */
void peci_start_delay_timer(void);

/**
 * @brief API to get hardware peripherals status to host.
 *
 * This routine gets hardware peripherals (fan and thermal sensors)
 * status.
 */
void get_hw_peripherals_status(uint8_t *hw_peripherals_sts);


/**
 * @brief API for SMC host to notify the CS mode exit.
 *
 */
void thermalmgmt_handle_cs_exit(void);
#endif	/* __THERMAL_MGMT_H__ */
