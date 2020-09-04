/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <logging/log_ctrl.h>
#include <logging/log.h>
#include "postcodemgmt.h"
#include "port80display.h"
LOG_MODULE_REGISTER(postcode, CONFIG_POSTCODE_LOG_LEVEL);

/* Current postcode values displayed */
static u8_t port80_code;
static u8_t port81_code;
static u8_t update_pending;

/* Indicates a PCH/board error condition was detected */
static u8_t err_code;

#ifdef CONFIG_POWER_SEQUENCE_ERROR_LED
static u8_t led_cntr;
#endif

#define BOARD_ERR_INDICATOR 0xEC
#define ERR_LED_FAST_BLINK_MASK        BIT(0)
#define ERR_LED_SLOW_BLINK_MASK        BIT(2)
#define ERR_LED_FAST_BLINK_BIT(led_n)  BIT((led_n - 1) * 2)
#define ERR_LED_SLOW_BLINK_BIT(led_n)  BIT(((led_n - 1) * 2 + 1))
#define ERR_LED_MASK(led_n)            (0x3 << ((led_n - 1) * 2))

/* Port80 display format */
#define WORD_FROM_PORTS(p81, p80) ((p81 << 8) | p80)

void update_error(u8_t errcode)
{
	err_code = errcode;
	LOG_DBG("EC%02x", err_code);
}

void update_progress(u8_t port_index, u8_t code)
{
	if (port_index == POSTCODE_PORT80) {
		if (port80_code != code) {
			port80_code = code;
			update_pending = 1;
		}
	} else {
		if (port81_code != code) {
			port81_code = code;
			update_pending = 1;
		}
	}
	LOG_DBG("P %04x", WORD_FROM_PORTS(port81_code, port80_code));
}

#ifdef CONFIG_POWER_SEQUENCE_ERROR_LED
static void update_error_leds(void)
{
	u8_t led1_val, led2_val, led3_val;

	led_cntr++;

	/* Process error code
	 * Don't use error codes - 0x03,0x0C,0x30,0x0F,0x33,0x3C,0x3F
	 * Difficult for user to understand
	 */
	if (err_code == 0) {
		return;
	}

	/* Always ON */
	led1_val = ((err_code & ERR_LED_MASK(1)) == ERR_LED_MASK(1))?BIT(0):0;
	led2_val = ((err_code & ERR_LED_MASK(2)) == ERR_LED_MASK(2))?BIT(2):0;
	led3_val = ((err_code & ERR_LED_MASK(3)) == ERR_LED_MASK(3))?BIT(4):0;

	/* Fast blink */
	if (led_cntr & ERR_LED_FAST_BLINK_MASK) {
		led1_val |= err_code & ERR_LED_FAST_BLINK_BIT(1);
		led2_val |= err_code & ERR_LED_FAST_BLINK_BIT(2);
		led3_val |= err_code & ERR_LED_FAST_BLINK_BIT(3);
	}
	/* Slow blink */
	if (led_cntr & ERR_LED_SLOW_BLINK_MASK) {
		led1_val |= err_code & ERR_LED_SLOW_BLINK_BIT(1);
		led2_val |= err_code & ERR_LED_SLOW_BLINK_BIT(2);
		led3_val |= err_code & ERR_LED_SLOW_BLINK_BIT(3);
	}

	led1_val = led1_val ? 0x00 : 0x01;
	led2_val = led2_val ? 0x00 : 0x01;
	led3_val = led3_val ? 0x00 : 0x01;

	/* TODO: Set the physical LEDs */
}
#endif

void postcode_thread(void *p1, void *p2, void *p3)
{
	u32_t disp_word;
	u32_t period = *(u32_t *)p1;
	int ret;

	ret = port80_display_init();
	if (ret) {
		LOG_ERR("port80 init failed %d", ret);
		return;
	}

	ret = port80_display_on();
	if (ret) {
		LOG_ERR("port80 display failed %d", ret);
		return;
	}

	while (true) {
		k_msleep(period);

		if (err_code) {
			port80_code = err_code;
			port81_code = BOARD_ERR_INDICATOR;
			port80_display_on();
			disp_word = WORD_FROM_PORTS(port81_code, port80_code);
			port80_display_word(disp_word);

			/* Flush the log buffer */
			LOG_PANIC();
#ifdef CONFIG_POWER_SEQUENCE_ERROR_LED
			update_error_leds();
#endif
		}

		/* Update postcode in port80 display */
		else if (update_pending) {
			disp_word = WORD_FROM_PORTS(port81_code, port80_code);
			port80_display_word(disp_word);
			LOG_DBG("P %x", disp_word);
			update_pending = 0;
		}
	}
}
