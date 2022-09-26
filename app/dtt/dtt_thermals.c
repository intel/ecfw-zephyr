/*
 * Copyright (c) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <logging/log.h>
#include "board_config.h"
#include "smc.h"
#include "dtt.h"
#include "memops.h"

LOG_MODULE_REGISTER(dtt_thrm, CONFIG_DTT_MODULE_LOG_LEVEL);

static uint8_t *therm_sensors;
static struct dtt_threshold dtt_sensor_trip_tbl[ACPI_THRM_SEN_TOTAL];

/**
 * DTT threshold default values upon init in degree celsius:
 * - low trip temp = 95c
 * - high trip temp = 100c
 * - temp hysteresis = 2c
 *
 * Note: All values are expressed in centigrades.
 */
#define DTT_LOW_TRIP_DEFAULT			950
#define	DTT_HIGH_TRIP_DEFAULT			1000
#define	DTT_TEMP_HYST_DEFAULT			20

#define DTT_THRSHLD_STATUS_BIT_INIT		0u
#define DTT_THRSHLD_STATUS_BIT_LOW_TRIP		4u
#define DTT_THRSHLD_STATUS_BIT_HIGH_TRIP	5u


void dtt_init_thermals(uint8_t *therm_sensors_list)
{
	therm_sensors = therm_sensors_list;

	for (uint8_t idx = 0; idx < ACPI_THRM_SEN_TOTAL; idx++) {
		if (therm_sensors[idx] < ADC_CH_TOTAL) {
			struct dtt_threshold *thrshld = &dtt_sensor_trip_tbl[idx];

			thrshld->low_temp = DTT_LOW_TRIP_DEFAULT;
			thrshld->high_temp = DTT_HIGH_TRIP_DEFAULT;
			thrshld->temp_hyst = DTT_TEMP_HYST_DEFAULT;
			thrshld->status = BIT(DTT_THRSHLD_STATUS_BIT_INIT);
		}
	}
}

void smc_update_dtt_threshold_limits(enum acpi_thrm_sens_idx acpi_sen_idx,
				     struct dtt_threshold thrshld)
{
	if (acpi_sen_idx >= ACPI_THRM_SEN_TOTAL) {
		LOG_WRN("Wrong sensor number");
		return;
	}

	if (!(dtt_sensor_trip_tbl[acpi_sen_idx].status & BIT(DTT_THRSHLD_STATUS_BIT_INIT))) {
		LOG_WRN("Sensor not initialized / supported");
		return;
	}

	struct dtt_threshold *thrsh = &dtt_sensor_trip_tbl[acpi_sen_idx];

	thrsh->high_temp = thrshld.high_temp;
	thrsh->low_temp = thrshld.low_temp;
	thrsh->temp_hyst = thrshld.temp_hyst;
}

void dtt_therm_sensor_trip(void)
{
	uint16_t acpi_thrm_stat = 0;

	for (uint8_t idx = 0; idx < ACPI_THRM_SEN_TOTAL; idx++) {
		struct dtt_threshold *thrshld = &dtt_sensor_trip_tbl[idx];
		int16_t snstemp = adc_temp_val[therm_sensors[idx]];
		bool tripped;
		int16_t triplimit;
		uint8_t old_status = thrshld->status;

		if (!(dtt_sensor_trip_tbl[idx].status & BIT(DTT_THRSHLD_STATUS_BIT_INIT))) {
			continue;
		}

		/* Low temperature trip check */
		tripped = thrshld->status & BIT(DTT_THRSHLD_STATUS_BIT_LOW_TRIP);
		triplimit = (tripped ? (thrshld->low_temp + thrshld->temp_hyst) :
			(thrshld->low_temp));

		if (snstemp < triplimit) {
			thrshld->status |= BIT(DTT_THRSHLD_STATUS_BIT_LOW_TRIP);
		} else {
			thrshld->status &= ~BIT(DTT_THRSHLD_STATUS_BIT_LOW_TRIP);
		}

		/* High temperature trip check */
		tripped = thrshld->status & BIT(DTT_THRSHLD_STATUS_BIT_HIGH_TRIP);
		triplimit = (tripped ? (thrshld->high_temp - thrshld->temp_hyst) :
			(thrshld->high_temp));

		if (snstemp > triplimit) {
			thrshld->status |= BIT(DTT_THRSHLD_STATUS_BIT_HIGH_TRIP);
		} else {
			thrshld->status &= ~BIT(DTT_THRSHLD_STATUS_BIT_HIGH_TRIP);
		}

		if (thrshld->status ^ old_status) {
			acpi_thrm_stat |= BIT(idx);
		}
	}

	smc_update_therm_trip_status(acpi_thrm_stat);
}
