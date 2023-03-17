/*
 * Copyright (c) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __LED_DEBUG_H__
#define __LED_DEBUG_H__

#include <zephyr/kernel.h>
#include "gpio_ec.h"
#include "board_config.h"

/* Check if leds are defined for board, if not map them
 * to a dummy gpio
 */
#ifndef KBC_CAPS_LOCK
#define KBC_CAPS_LOCK			EC_DUMMY_GPIO_HIGH
#endif
#ifndef KBC_NUM_LOCK
#define KBC_NUM_LOCK			EC_DUMMY_GPIO_HIGH
#endif
#ifndef KBC_SCROLL_LOCK
#define KBC_SCROLL_LOCK			EC_DUMMY_GPIO_HIGH
#endif

/* Only if LED debug mechanism enabled macro maps to gpio write */
#ifdef CONFIG_DEBUG_LED
#define DEBUG_LED1                      KBC_CAPS_LOCK
#define DEBUG_LED2                      KBC_NUM_LOCK
#define DEBUG_LED3                      KBC_SCROLL_LOCK
#define LED_MARKER_HIGH(led_num)	gpio_write_pin(led_num, 1)
#define LED_MARKER_LOW(led_num)		gpio_write_pin(led_num, 0)
#else
#define LED_MARKER_HIGH(led_num)
#define LED_MARKER_LOW(led_num)
#endif /* CONFIG_DEBUG_LED */

#endif /* __LED_DEBUG_H__ */
