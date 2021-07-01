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
#include "peci_hub.h"
#include "pwrplane.h"

LOG_MODULE_REGISTER(thermal, CONFIG_THERMAL_MGMT_LOG_LEVEL);

/* PECI takes some time to initialize in CPU side, initial request will fail
 * after reaching S0.
 * Hence allow 1 sec time before reading temperature from peci.
 * Till then report fail safe temperature of 28C.
 */
#define CPU_TEMP_ACCESS_DELAY_SEC		2U

#define CPU_FAIL_SAFE_TEMPERATURE		28U

/* CPU fail critical temperature value is 72C */
#define CPU_FAIL_CRITICAL_TEMPERATURE		72U

/*
 * EC Self control fan speed based on CPU temperature info.
 *
 * Two simple approaches are possible
 * Approach #1 Control fan with liner profile.
 * For CPU temperature between 0 to 100, fan to also rotate at equivalent speed,
 * but rather than changing speed on every single degree change, it is changed
 * after 8 degrees.
 *
 * Approach #2 Use a lookup table based on temperature
 */
#define GET_FAN_SPEED_FOR_TEMP(temp)		(temp & (~0x7))

struct therm_sensor *therm_sensor_tbl;
struct fan_dev *fan_dev_tbl;
static u8_t max_adc_sensors;
static u8_t max_fan_dev;
static bool thermal_initialized;
static bool peci_initialized;
static u8_t fan_duty_cycle[FAN_DEV_TOTAL];
static bool fan_duty_cycle_change;
static int cpu_temp;
/* Using module property, before introducing straps handler */
static bool fan_override;

static u8_t mapped_fan_speed(void)
{
	u8_t cpu_fan_speed;

	switch(cpu_temp) {
	case 00 ... 19 :
		cpu_fan_speed = 0;
		break;
	case 20 ... 29 :
		cpu_fan_speed = 20;
		break;
	case 30 ... 39:
		cpu_fan_speed = 30;
		break;
	case 40 ... 49:
		cpu_fan_speed = 50;
		break;
	case 50 ... 59:
		cpu_fan_speed = 75;
		break;
	case 60 ... 100:
	default:
		cpu_fan_speed = 100;
		break;
	}

	return cpu_fan_speed;
}

static void init_fans(void)
{
	int ret;

#ifdef CONFIG_THERMAL_FAN_OVERRIDE
	fan_override = true;
	LOG_WRN("Fan SW override enable: %d", fan_override);
#endif

	/* Get the list of fan devices supported for the board */
	board_fan_dev_tbl_init(&max_fan_dev, &fan_dev_tbl);
	LOG_DBG("Board has %d fans", max_fan_dev);

	ret = fan_init(max_fan_dev, fan_dev_tbl);
	if (ret) {
		LOG_ERR("Failed to init fan");
	}

	fan_duty_cycle[FAN_CPU] = CONFIG_THERMAL_FAN_OVERRIDE_VALUE;
	fan_duty_cycle_change = 1;
}

static void init_therm_sensors(void)
{
	u8_t adc_ch_bits = 0;

	board_therm_sensor_tbl_init(&max_adc_sensors, &therm_sensor_tbl);

	LOG_INF("Num of thermal sensors: %d", max_adc_sensors);

	for (u8_t idx = 0; idx < max_adc_sensors; idx++) {
		LOG_INF("adc ch: %d, for acpi sen: %d",
			therm_sensor_tbl[idx].adc_ch,
			therm_sensor_tbl[idx].acpi_loc);

		adc_ch_bits |= BIT(therm_sensor_tbl[idx].adc_ch);
	}

	LOG_INF("adc ch sensors bit: %x", adc_ch_bits);
	if (thermal_sensors_init(adc_ch_bits)) {
		LOG_WRN("Thermal Sensor module init failed!");
	} else {
		thermal_initialized = true;
	}

}

static void manage_fan(void)
{
	/* EC Self control fan based on CPU thermal info */
	u8_t cpu_fan_speed = mapped_fan_speed();
	LOG_WRN("Fan speed 0x%x", cpu_fan_speed);

	if (fan_duty_cycle[FAN_CPU] != cpu_fan_speed) {
		fan_duty_cycle[FAN_CPU] = cpu_fan_speed;
		fan_duty_cycle_change = 1;
	}

	/* HW/KConfig override takes precedence over every control method
	 * This is mostly used for PO entry on PO team request
	 */
	if (fan_override) {
		fan_duty_cycle[FAN_CPU] = CONFIG_THERMAL_FAN_OVERRIDE_VALUE;
		fan_duty_cycle_change = 1;
	}

	if (fan_duty_cycle_change) {
		fan_duty_cycle_change = 0;

		for (u8_t idx = 0; idx < max_fan_dev; idx++) {
			fan_set_duty_cycle(idx, fan_duty_cycle[idx]);
		}
	}

	for (u8_t idx = 0; idx < max_fan_dev; idx++) {
		u16_t rpm;

		fan_read_rpm(idx, &rpm);
	}
}

static void manage_thermal_sensors(void)
{
	/* Do not attempt to update if no sensors were detected */
	if (!thermal_initialized) {
		return;
	}

	thermal_sensors_update();
}

K_TIMER_DEFINE(peci_delay_timer, NULL, NULL);
static bool host_ready;

void peci_start_delay_timer(void)
{
	/* start a one-shot timer for prescribed seconds */
	k_timer_start(&peci_delay_timer, K_SECONDS(CPU_TEMP_ACCESS_DELAY_SEC),
		      K_NO_WAIT);
	LOG_WRN("PECI delay timer started");
	host_ready = true;
}

static void manage_cpu_thermal(void)
{
	int temp, ret;

	/* Manage CPU thermal only in S0 state */
	if (!peci_initialized || k_timer_remaining_get(&peci_delay_timer) ||
	    (pwrseq_system_state() != SYSTEM_S0_STATE)) {
		return;
	}

	/* Read cpu temperature using peci. */
	ret = peci_get_temp(&temp);
	if (ret) {
		LOG_ERR("Failed to get cpu temperature host_ready: %d, ret-%x", host_ready, ret);
		temp = CPU_FAIL_CRITICAL_TEMPERATURE;
	}

	cpu_temp = temp;

	LOG_WRN("%s: Cpu Temp=%d", __func__, temp);
}

int thermalmgmt_task_init(void)
{
	int err;

	init_fans();
	init_therm_sensors();
	err = peci_init();
	if (!err) {
		peci_initialized = true;
	} else {
		LOG_WRN("Fail to initialize PECI");
		return -ENODEV;
	}

	return 0;
}

void thermalmgmt_thread(void *p1, void *p2, void *p3)
{
	int ret;
	u32_t period = *(u32_t *)p1;

	ret = thermalmgmt_task_init();
	if (ret) {
		LOG_ERR("Fail to initialize");
		return;
	}

	while (true) {
		k_msleep(period);

		manage_fan();
		manage_thermal_sensors();

		manage_cpu_thermal();
	}
}
