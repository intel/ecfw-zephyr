/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __THERMAL_SENSOR_H__
#define __THERMAL_SENSOR_H__

enum adc_ch_num {
	ADC_CH_00,
	ADC_CH_01,
	ADC_CH_02,
	ADC_CH_03,
	ADC_CH_04,
	ADC_CH_05,
	ADC_CH_06,
	ADC_CH_07,

	ADC_CH_TOTAL
};

extern int16_t adc_temp_val[ADC_CH_TOTAL];

/**
 * @brief Initialize thermal sensor module.
 *
 * @param adc_channel_bits bit field value for the adc channels to be enabled.
 *
 * @return 0 if success, otherwise error code.
 */
int thermal_sensors_init(uint8_t adc_channel_bits);


/**
 * @brief  Read all the thermal sensors.
 *
 * This function call reads all ADC thermal sensors enabled in init, and
 * updates adc_temp_val array for respective ADC channel reads.
 */
void thermal_sensors_update(void);

#endif	/* __THERMAL_SENSOR_H__ */
