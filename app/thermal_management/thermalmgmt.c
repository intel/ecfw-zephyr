/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <logging/log.h>
#include "thermalmgmt.h"
#include "fan.h"
#include "thermal_sensor.h"
#include "board_config.h"
#include "smc.h"

LOG_MODULE_REGISTER(thermal, CONFIG_THERMAL_MGMT_LOG_LEVEL);

struct therm_sensor *ptherm_sensor_list;
static u8_t max_adc_sensors;

int thermalmgmt_task_init(void)
{
	u8_t idx;
	u8_t adc_ch_bits = 0;

	if (fan_init()) {
		LOG_WRN("Fan module init failed!");
	}

	ptherm_sensor_list = therm_sensor_list_init(&max_adc_sensors);

	LOG_INF("Num of thermal sensors: %d", max_adc_sensors);

	for (idx = 0; idx < max_adc_sensors; idx++) {
		LOG_INF("adc ch: %d, for acpi sen: %d",
				ptherm_sensor_list[idx].adc_ch,
				ptherm_sensor_list[idx].acpi_loc);

		adc_ch_bits |= BIT(ptherm_sensor_list[idx].adc_ch);
	}

	LOG_INF("adc ch sensors bit: %x", adc_ch_bits);
	if (thermal_sensors_init(adc_ch_bits)) {
		LOG_WRN("Thermal Sensor module init failed!");
	}

	return 0;
}

void thermalmgmt_thread(void *p1, void *p2, void *p3)
{
	u32_t *period = (u32_t *)p1;

	while (1) {
		k_sleep(*period);

		fan_set_duty_cycle(PWM_CH_CPU, 30);

		thermal_sensors_update();

		for (u8_t idx = 0; idx < max_adc_sensors; idx++) {
			smc_update_thermal_sensor(
				ptherm_sensor_list[idx].acpi_loc,
				adc_temp_val[ptherm_sensor_list[idx].adc_ch]);
		}
	}
}
