/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __LED_H__
#define __LED_H__

#define ONLY_ON		100u
#define ONLY_OFF	0u

enum led_num {
	LED2,
	LED_KBD_BKLT,
	LED_TOTAL,
	LED_UNDEF = 0xFF,
};

/**
 * @brief Make the LED blink at the prescribed "duty cycle" percentage.
 *
 * @param idx Number representing the LED device.
 * @param duty_cycle pwm duty_cycle in % value can be between 0 to 100.
 */
void led_blink(enum led_num idx, uint8_t duty_cycle);

/*
 * @brief Turn on the LED
 *
 * @param idx Number representing the LED device.
 */
static inline void led_on(enum led_num idx)
{
	return led_blink(idx, ONLY_ON);
}

/**
 * @brief Turn off the LED
 *
 * @param idx Number representing the LED device.
 */
static inline void led_off(enum led_num idx)
{
	return led_blink(idx, ONLY_OFF);
}

/**
 * @brief Init the LEDs
 */
void led_init(enum led_num idx);

#endif	/* __LED_H__ */
