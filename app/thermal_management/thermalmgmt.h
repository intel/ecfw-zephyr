/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __THERMAL_MGMT_H__
#define __THERMAL_MGMT_H__

#include "fan.h"
#include "thermal_sensor.h"
#include "smc.h"

struct therm_sensor {
	enum adc_ch_num adc_ch;
	enum acpi_thrm_sens_idx acpi_loc;
};

/**
 * @brief Performs initialization of thermal manager task.
 */
int thermalmgmt_task_init(void);

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
 * @brief API for SMC host to start the peci delay timer.
 *
 * PECI is prone to fail during boot to S0. Hence peci is not accessed
 * for a prescribed time duration.
 */
void peci_start_delay_timer(void);

#endif	/* __THERMAL_MGMT_H__ */
