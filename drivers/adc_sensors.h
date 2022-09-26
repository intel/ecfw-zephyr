/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __ADC_SENSORS_H__
#define __ADC_SENSORS_H__

enum adc_ch_num {
	ADC_CH_00,
	ADC_CH_01,
	ADC_CH_02,
	ADC_CH_03,
	ADC_CH_04,
	ADC_CH_05,
	ADC_CH_06,
	ADC_CH_07,

	ADC_CH_TOTAL,
	ADC_CH_UNDEF = 0xFF
};

extern int16_t adc_temp_val[ADC_CH_TOTAL];

/**
 * @brief Initialize thermal sensor module.
 *
 * @param adc_channel_bits bit field value for the adc channels to be enabled.
 *
 * @return 0 if success, otherwise error code.
 */
int adc_sensors_init(uint8_t adc_channel_bits);


/**
 * @brief  Read all the thermal sensors.
 *
 * This function call reads all ADC thermal sensors enabled in init, and
 * updates adc_temp_val array for respective ADC channel reads.
 */
void adc_sensors_read_all(void);

#endif	/* __ADC_SENSORS_H__ */
