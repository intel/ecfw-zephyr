/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/ps2.h>
#include "board_config.h"
#include "keyboard_utility.h"
#include "ps2kbaux.h"
#include "kbs_keymap.h"
#include "sci.h"
#include "scicodes.h"
#include "smc.h"
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(kbchost, CONFIG_KBCHOST_LOG_LEVEL);

#define SCAN_CODE_SET_TWO		2U
#define KB_MB_RETRIES			3U
/* Time units in msec */
#define PS2_RETRY_PERIOD		1U

#define KEY_RELEASED_POS		7U
#define SC1_WITHOUT_MAKE_BREAK		0x7FU
#define LEFT_CTRL			0x1DU
#define LEFT_ALT			0x38U
#define LEFT_SHIFT			0x2AU
#define ESCAPE_CODE			0xE0U
#define F1_SC1				0x3BU
#define F2_SC1				0x3CU
#define F3_SC1				0x3DU
#define F4_SC1				0x3EU
#define F5_SC1				0x3FU
#define F6_SC1				0x40U
#define F7_SC1				0x41U
#define F8_SC1				0x42U
#define F9_SC1				0x43U
#define F10_SC1				0x44U
#define F11_SC1				0x57U
#define F12_SC1				0x58U
#define PS2_ACK				0xFAU

static ps2_callback keyboard_callback;
static ps2_callback mouse_callback;
static const struct device *keyboard_dev;
static const struct device *mouse_dev;
static struct km_api *keymap_api;
static const uint8_t *current_set;

enum ps2_cmd {
	ENABLE_CALLBACK,
	DISABLE_CALLBACK,
};

typedef int (*ps2_func)(const struct device *);

static ps2_func cb_ops[] = {
	[ENABLE_CALLBACK] = ps2_enable_callback,
	[DISABLE_CALLBACK] = ps2_disable_callback,
};

static int convert_sc1_to_keynumber(uint8_t sc1, uint8_t *key_num)
{
	/* We ignore makes and breaks as we just want
	 * to now if there was an interaction with one of the
	 * FX keys
	 */
	switch (sc1 & ~BIT(KEY_RELEASED_POS)) {
	case F1_SC1:
		*key_num = KM_F1_KEY;
		break;
	case F2_SC1:
		*key_num = KM_F2_KEY;
		break;
	case F3_SC1:
		*key_num = KM_F3_KEY;
		break;
	case F4_SC1:
		*key_num = KM_F4_KEY;
		break;
	case F5_SC1:
		*key_num = KM_F5_KEY;
		break;
	case F6_SC1:
		*key_num = KM_F6_KEY;
		break;
	case F7_SC1:
		*key_num = KM_F7_KEY;
		break;
	case F8_SC1:
		*key_num = KM_F8_KEY;
		break;
	case F9_SC1:
		*key_num = KM_F9_KEY;
		break;
	case F10_SC1:
		*key_num = KM_F10_KEY;
		break;
	case F11_SC1:
		*key_num = KM_F11_KEY;
		break;
	case F12_SC1:
		*key_num = KM_F12_KEY;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static bool ctrl_alt_shift_sc1(uint8_t data)
{
	uint8_t filtered_data;
	static uint8_t prev_byte;
	static bool ctrl_pressed;
	static bool alt_pressed;
	static bool shift_pressed;

	/* Filters the last bit which indicates break or make code,
	 * this is because we want to detect key presses/releases
	 */
	filtered_data = data & SC1_WITHOUT_MAKE_BREAK;

	if (filtered_data == LEFT_CTRL) {
		ctrl_pressed = (data & BIT(KEY_RELEASED_POS)) == 0U;
	} else if (filtered_data == LEFT_ALT) {
		alt_pressed = (data & BIT(KEY_RELEASED_POS)) == 0U;
	} else if (filtered_data == LEFT_SHIFT) {
		if (prev_byte != ESCAPE_CODE) {
			shift_pressed = (data & BIT(KEY_RELEASED_POS)) == 0U;
		}
	}

	prev_byte = data;

	if (ctrl_pressed & alt_pressed & shift_pressed) {
		return true;
	}

	return false;
}

static int do_control_callback(enum ps2_cmd cmd, const struct device *dev)
{
	int err;
	int attempts = 0;

	do {
		err = cb_ops[(int)cmd](dev);
		if (!err) {
			break;
		}
		k_sleep(K_MSEC(PS2_RETRY_PERIOD));
		attempts++;
	} while (attempts < KB_MB_RETRIES);

	return err;
}

static void ps2_keyboard_callback(const struct device *dev, uint8_t value)
{
	int ret;

	if (translate_key(*current_set, &value) != 0U) {
		return;
	}

	/* Simulate Fn press via ctrl+alt+shift */
	if (ctrl_alt_shift_sc1(value) && value < PS2_ACK) {
		uint8_t key_sc = value & SC1_WITHOUT_MAKE_BREAK;
		/* sense both press/release events by ignoring bit 7 for sc1 */
		if (key_sc != LEFT_CTRL && key_sc != LEFT_ALT
		    && key_sc != LEFT_SHIFT) {
			uint8_t key_num = 0U;
			struct fn_data data;
			/* Convert scan code to IBM key number */
			convert_sc1_to_keynumber(key_sc, &key_num);

			/* Use key number to retrieve FN functionality
			 * from custom keyboard
			 */
			ret = keymap_get_fnkey(keymap_api, key_num, &data,
					 (value & BIT(KEY_RELEASED_POS)) == 0U);
			if (ret) {
				/* No Fn Key is pressed along with CAS.
				 * Since send CAS + Key Value.
				 */
				if (value & BIT(KEY_RELEASED_POS)) {
					g_acpi_tbl.acpi_hotkey_scan = 0U;
				} else {
					g_acpi_tbl.cas_hotkey = key_sc;
				}
				LOG_DBG("Sending CAS Key SCI for %x",
						g_acpi_tbl.cas_hotkey);
				enqueue_sci(SCI_HOTKEY_CAS);
				return;
			}

			/* Retranslate to scan code set 1 since we are
			 * using set 2 for FN keys. Or send an sci if a
			 * corresponding FX key has an sci assigned
			 */
			if (data.type == FN_SCAN_CODE) {
				for (int i = 0; i < data.sc.len; i++) {
					if (translate_key(*current_set,
						&data.sc.code[i]) == 0U) {

						keyboard_callback(
							data.sc.code[i]);
					}
				}
			} else {
				if (data.sci_code != 0U) {
					if (ctrl_alt_shift_sc1(value)) {
						g_acpi_tbl.acpi_hotkey_scan =
							data.sci_code;
					LOG_DBG("Sending HOT Key SCI for %x",
						g_acpi_tbl.acpi_hotkey_scan);
						enqueue_sci(SCI_HOTKEY);
					}
				}
			}
		}
	} else {
		keyboard_callback(value);
	}
}

static void ps2_mouse_callback(const struct device *dev, uint8_t value)
{
	mouse_callback(value);
}

int ps2_keyboard_init(const ps2_callback callback, uint8_t *initial_set)
{

	int ret;

	if (!callback) {
		LOG_ERR("Bad callback");
		return -EINVAL;
	}

	keyboard_dev = DEVICE_DT_GET(PS2_KEYBOARD);

	if (!device_is_ready(keyboard_dev)) {
		LOG_ERR("PS2 kbd device not ready");
		return -EIO;
	}

	ret = ps2_config(keyboard_dev, ps2_keyboard_callback);
	if (ret) {
		LOG_ERR("PS2 config failed: %d", ret);
		return ret;
	}

	keyboard_callback = callback;
	current_set = (const uint8_t *)initial_set;

	if (*current_set != SCAN_CODE_SET_TWO) {
		return -EINVAL;
	}

	return 0;
}

void ps2_keyboard_write(uint8_t data)
{
	if (ps2_write(keyboard_dev, data)) {
		LOG_ERR("PS/2 kb write failed");
	}
}

void ps2_keyboard_disable(void)
{
	if (do_control_callback(DISABLE_CALLBACK, keyboard_dev)) {
		LOG_ERR("PS/2 kb enable failed");
	}
}

void ps2_keyboard_enable(void)
{
	if (do_control_callback(ENABLE_CALLBACK, keyboard_dev)) {
		LOG_ERR("PS/2 kb enable failed");
	}
}

int ps2_mouse_init(ps2_callback callback)
{
	int ret;

	if (!callback) {
		LOG_ERR("Bad callback");
		return -EINVAL;
	}

	mouse_dev = DEVICE_DT_GET(PS2_MOUSE);

	if (!device_is_ready(mouse_dev)) {
		LOG_ERR("PS2 mouse device not ready");
		return -EIO;
	}

	ret = ps2_config(mouse_dev, ps2_mouse_callback);
	if (ret) {
		LOG_ERR("PS/2 mb config failed: %d", ret);
		return ret;
	}

	mouse_callback = callback;

	return 0;
}

void ps2_mouse_write(uint8_t data)
{
	if (ps2_write(mouse_dev, data)) {
		LOG_ERR("PS/2 mb write failed");
	}
}

void ps2_mouse_disable(void)
{
	if (do_control_callback(DISABLE_CALLBACK, mouse_dev)) {
		LOG_ERR("PS/2 mb disable failed");
	}
}

void ps2_mouse_enable(void)
{
	if (do_control_callback(ENABLE_CALLBACK, mouse_dev)) {
		LOG_ERR("PS/2 mb enable failed");
	}
}

