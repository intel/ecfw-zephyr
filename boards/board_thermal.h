/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BOARD_THERMAL_H__
#define __BOARD_THERMAL_H__

#include "adc_sensors.h"

/**
 * @brief Initialize thermal sensor list as per board id identified at runtime.
 *
 * @param therm_sensors list of supported thermal sensors in the ACPI table.
 *
 * @note therm_sensors list is an array of ACPI_THRM_SEN_TOTAL (5) sensors.
 * All elements of array are initialized to default ADC_CH_UNDEF (0xFF).
 * Each sensor index purpose is listed in the enum comments, for e.g.
 * ACPI_THRM_SEN_4 is for VR temperature monitoring.
 */
void board_therm_sensor_list_init(uint8_t therm_sensors[]);

/**
 * @brief Get the fan device list defined for the board.
 *
 * @param pmax_fan pointer to update max number of fan devices supported.
 * @param pfan_tbl pointer to the fan device list.
 */
void board_fan_dev_tbl_init(uint8_t *pmax_fan, struct fan_dev **pfan_tbl);

#endif /* __BOARD_THERMAL_H__ */

