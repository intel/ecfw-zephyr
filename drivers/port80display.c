/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/logging/log.h>
#include "max6958.h"
#include "port80display.h"
LOG_MODULE_REGISTER(port80, CONFIG_POSTCODE_LOG_LEVEL);

/* Scan Display digits 0-3 */
#define PORT80_DISPLAY_DIGITS  3

/* Maximum retries in case of an I2C failure */
#define MAX_RETRIES	3

int port80_display_init(void)
{
	int ret;

	/* Hexadecimal decode for digits 3-0 */
	uint8_t dig_sel = MAX6958_DIG_DECODE_SEL(MAX6958_HEX_DECODE, 0) |
			MAX6958_DIG_DECODE_SEL(MAX6958_HEX_DECODE, 1) |
			MAX6958_DIG_DECODE_SEL(MAX6958_HEX_DECODE, 2) |
			MAX6958_DIG_DECODE_SEL(MAX6958_HEX_DECODE, 3);

	ret = max6958_set_decode_mode(dig_sel);
	if (ret) {
		LOG_ERR("Failed to select decode mode: %d", ret);
		return ret;
	}

	ret = max6958_set_brightness(MAX6958_QUARTER_INTENSITY);
	if (ret) {
		LOG_ERR("Failed to set brightness: %d", ret);
		return ret;
	}

	max6958_set_digits(PORT80_DISPLAY_DIGITS);
	return ret;
}

int port80_display_on(void)
{
	int ret, retries = 0;

	do {
		LOG_DBG("Setting display power on");
		ret = max6958_set_power(true);
		if (!ret) {
			break;
		}

		LOG_ERR("Failed to power on display: %d", ret);
		retries++;
	} while (retries <= MAX_RETRIES);

	return ret;
}

int port80_display_off(void)
{
	int ret;

	ret = max6958_set_power(false);
	if (ret) {
		LOG_ERR("Failed to power off display: %d", ret);
	}

	return ret;
}

void port80_display_word(uint16_t word)
{
	int ret;

	ret = max6958_display_digits(word);
	if (ret) {
		LOG_ERR("Failed to display postcode: %d", ret);
	}
}
