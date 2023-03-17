/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include "i2c_hub.h"
#include <zephyr/logging/log.h>
#include "max6958.h"
#include "board_config.h"
LOG_MODULE_REGISTER(max6958, CONFIG_MAX6958_LOG_LEVEL);

#define MAX6958_9SEG_DISP_DRIVER_I2C_ADDR 0x38

/* Registers */
#define MAX6958_DECODE_MODE	0x01
#define MAX6958_INTENSITY	0x02
#define MAX6958_SCAN_LIMIT	0x03
#define MAX6958_CONFIG		0x04
#define MAX6958_GPIO		0x06
#define MAX6958_TEST		0x07
#define MAX6958_KEY_DEBOUNCED	0x08
#define MAX6958_KEY_PRESSED	0x0C
#define MAX6958_DIGIT0		0x20
#define MAX6958_DIGIT1		0x21
#define MAX6958_DIGIT2		0x22
#define MAX6958_DIGIT3		0x23
#define MAX6958_SEGMENTS	0x24

/* Configuration bits */
#define MAX6958_CLEAR_DIGITS	(1 << 5)
#define MAX6958_HW_ID		(1 << 1)
#define MAX6958_SHUTDOWN	(1 << 0)

#define MAX6958_SCAN_MASK	0x07

int max6958_set_decode_mode(uint8_t mode_mask)
{
	uint8_t data[] = { MAX6958_DECODE_MODE, mode_mask };

	return i2c_hub_write(I2C_0, data, sizeof(data),
				MAX6958_9SEG_DISP_DRIVER_I2C_ADDR);
}

int max6958_set_brightness(uint8_t duty_cycle)
{
	uint8_t data[] = { MAX6958_INTENSITY,
			(duty_cycle & MAX6958_MAX_INTENSITY) };

	return i2c_hub_write(I2C_0, data, sizeof(data),
				MAX6958_9SEG_DISP_DRIVER_I2C_ADDR);
}

int max6958_set_digits(uint8_t num)
{
	uint8_t data[] = { MAX6958_SCAN_LIMIT, (num & MAX6958_SCAN_MASK)};

	return i2c_hub_write(I2C_0, data, sizeof(data),
				MAX6958_9SEG_DISP_DRIVER_I2C_ADDR);
}

int max6958_set_power(bool power)
{
	uint8_t data[] = { MAX6958_CONFIG, 0 };

	if (power) {
		data[1] |= MAX6958_SHUTDOWN;
	} else {
		data[1] &= ~MAX6958_SHUTDOWN;
	}

	return i2c_hub_write(I2C_0, data, sizeof(data),
				MAX6958_9SEG_DISP_DRIVER_I2C_ADDR);
}

int max6958_display_digits(uint32_t value)
{
	uint8_t data[] = { MAX6958_DIGIT0, 0, 0, 0, 0 };

	/* Copy bytes from msb to lsb */
	data[1] = (value & 0xF000) >> 12;
	data[2] = (value & 0xF00) >> 8;
	data[3] = (value & 0xF0) >> 4;
	data[4] = (value & 0x0F);

	return i2c_hub_write(I2C_0, data, sizeof(data),
				MAX6958_9SEG_DISP_DRIVER_I2C_ADDR);
}
