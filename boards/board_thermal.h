/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BOARD_THERMAL_H__
#define __BOARD_THERMAL_H__


/**
 * @brief Get the pointer to thermal sensor list table defined for the board.
 *
 * @param p_max_adc_sensors pointer to update thermal sensor table size.
 * @param p_therm_sensor_tbl pointer to thermal sensor list table.
 */
void board_therm_sensor_tbl_init(uint8_t *p_max_adc_sensors,
		struct therm_sensor **p_therm_sensor_tbl);

/**
 * @brief Get the fan device list defined for the board.
 *
 * @param pmax_fan pointer to update max number of fan devices supported.
 * @param pfan_tbl pointer to the fan device list.
 */
void board_fan_dev_tbl_init(uint8_t *pmax_fan, struct fan_dev **pfan_tbl);

#endif /* __BOARD_THERMAL_H__ */

