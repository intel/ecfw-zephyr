/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/logging/log.h>
#include "espi_hub.h"
#include "postcodemgmt.h"
#include "port80display.h"
LOG_MODULE_REGISTER(postcode, CONFIG_POSTCODE_LOG_LEVEL);

static struct k_sem update_lock;
/* Postcode requested to be displayed */
static uint8_t port80_code;
static uint8_t port81_code;

/* Indicates a PCH/board error condition was detected */
static uint8_t err_code;

#ifdef CONFIG_POWER_SEQUENCE_ERROR_LED
static uint8_t led_cntr;
#endif

#define BOARD_ERR_INDICATOR 0xEC
#define ERR_LED_FAST_BLINK_MASK        BIT(0)
#define ERR_LED_SLOW_BLINK_MASK        BIT(2)
#define ERR_LED_FAST_BLINK_BIT(led_n)  BIT((led_n - 1) * 2)
#define ERR_LED_SLOW_BLINK_BIT(led_n)  BIT(((led_n - 1) * 2 + 1))
#define ERR_LED_MASK(led_n)            (0x3 << ((led_n - 1) * 2))

/* Port80 display format */
#define WORD_FROM_PORTS(p81, p80) ((p81 << 8) | p80)

static void signal_request(void)
{
	if (k_sem_count_get(&update_lock) == 0) {
		k_sem_give(&update_lock);
	}
}

void update_error(uint8_t errcode)
{
	err_code = errcode;
	LOG_DBG("EC%02x", err_code);
	signal_request();
}

static void update_postcode(uint8_t port_index, uint8_t code)
{
	bool update_pending = false;

	switch (port_index) {
	case POSTCODE_PORT80:
		if (port80_code != code) {
			port80_code = code;
			LOG_DBG("port80:%02x", code);
			update_pending = true;
		}
		break;
	case POSTCODE_PORT81:
		if (port81_code != code) {
			port81_code = code;
			LOG_DBG("port81:%02x", code);
			update_pending = true;
		}
		break;
	default:
		break;
	}

	if (update_pending) {
		signal_request();
	}
}

#ifdef CONFIG_POWER_SEQUENCE_ERROR_LED
static void update_error_leds(void)
{
	uint8_t led1_val, led2_val, led3_val;

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
	uint32_t disp_word;
	int ret;

	ret = port80_display_init();
	if (ret) {
		LOG_ERR("port80 init failed %d", ret);
		return;
	}

	espihub_add_postcode_handler(update_postcode);
	k_sem_init(&update_lock, 0, 1);

	while (true) {
		/* Wait until postcode update is received */
		k_sem_take(&update_lock, K_FOREVER);
		if (err_code) {
			port80_code = err_code;
			port81_code = BOARD_ERR_INDICATOR;
			port80_display_on();
			disp_word = WORD_FROM_PORTS(port81_code, port80_code);
			port80_display_word(disp_word);
			LOG_DBG("Post:%04x", disp_word);

			/* Flush the log buffer */
			LOG_PANIC();
#ifdef CONFIG_POWER_SEQUENCE_ERROR_LED
			update_error_leds();
#endif
		}

		/* Update postcode in port80 display */
		else {
			disp_word = WORD_FROM_PORTS(port81_code, port80_code);
			port80_display_word(disp_word);
			LOG_DBG("PostCode:%04x", disp_word);
		}
	}
}
