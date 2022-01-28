/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief MAX6958 9-segment display driver APIs.
 *
 */

#ifndef __MAX6958_H__
#define __MAX6958_H__

#define MAX6958_NAME			"MAX6958"
#define MAX6958_NO_DECODE		(0ul << 0)
#define MAX6958_HEX_DECODE		(1ul << 0)
#define MAX6958_DIG_DECODE_SEL(m, d)	(m << d)

/**
 * @brief Set decode mode for each digit, hexadecimal or not decoding.
 *
 * @param mode_mask the bitmask indicating decode mode for each digit.
 *
 */
int max6958_set_decode_mode(uint8_t mode_mask);

/* Duty cycle
 * This is define x / 64
 * i.e quarter intensity uses ~5.76 mA
 */
#define MAX6958_DEFAULT_INTENSITY	0x4
#define MAX6958_QUARTER_INTENSITY	0x10
#define MAX6958_HALF_INTENSITY		0x20
#define MAX6958_MAX_INTENSITY		0x3F

/**
 * @brief Adjust brightness of 9-segment LED display.
 *
 * @param duty_cycle the duty cycle in 1/64th.
 *
 * @retval -EIO General input / output error, failed to configure device.
 *
 */
int max6958_set_brightness(uint8_t duty_cycle);

/**
 * @brief Sets the number of digits displayed.
 *
 * @param num the number of digits from one to four.
 *
 * @retval -EIO General input / output error, failed to configure device.
 *
 */
int max6958_set_digits(uint8_t num);

/**
 * @brief Set display mode of operation.
 *
 * @param power controls the display mode of operation.
 *
 * @retval -EIO General input / output error, failed to configure device.
 *
 */
int max6958_set_power(bool power);

/**
 * @brief Display digits.
 *
 * @param data the digits to display 3-0.
 *
 * @retval -EIO General input / output error, failed to configure device.
 *
 */
int max6958_display_digits(uint32_t digits);

#endif /* __MAX6958_H__ */
