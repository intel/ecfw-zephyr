/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/ps2.h>
#include "board_config.h"
#include "keyboard_utility.h"
#include "ps2kbaux.h"

#include <logging/log.h>
LOG_MODULE_DECLARE(kbchost, CONFIG_KBCHOST_LOG_LEVEL);

#define SCAN_CODE_SET_TWO	2U
#define KB_MB_RETRIES		3U
/* Time units in msec */
#define PS2_RETRY_PERIOD	1U

static ps2_callback keyboard_callback;
static ps2_callback mouse_callback;
static struct device *keyboard_dev;
static struct device *mouse_dev;

static const u8_t *current_set;

enum ps2_cmd {
	ENABLE_CALLBACK,
	DISABLE_CALLBACK,
};

typedef int (*ps2_func)(struct device *);

static ps2_func cb_ops[] = {
	[ENABLE_CALLBACK] = ps2_enable_callback,
	[DISABLE_CALLBACK] = ps2_disable_callback,
};

static int do_control_callback(enum ps2_cmd cmd, struct device *dev)
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

static void ps2_keyboard_callback(struct device *dev, u8_t value)
{
	if (translate_key(*current_set, &value)) {
		return;
	}

	keyboard_callback(value);
}

static void ps2_mouse_callback(struct device *dev, u8_t value)
{
	mouse_callback(value);
}

int ps2_keyboard_init(const ps2_callback callback, u8_t *initial_set)
{

	int ret;

	if (!callback) {
		LOG_ERR("Bad callback");
		return -EINVAL;
	}

	keyboard_dev = device_get_binding(PS2_KEYBOARD);

	if (!keyboard_dev) {
		LOG_ERR("PS2 kbd binding failed");
		return -EIO;
	}

	ret = ps2_config(keyboard_dev, ps2_keyboard_callback);
	if (ret) {
		LOG_ERR("PS2 config failed: %d", ret);
		return ret;
	}

	keyboard_callback = callback;
	current_set = (const u8_t *)initial_set;

	if (*current_set != SCAN_CODE_SET_TWO) {
		return -EINVAL;
	}

	return 0;
}

void ps2_keyboard_write(u8_t data)
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

	mouse_dev = device_get_binding(PS2_MOUSE);

	if (!mouse_dev) {
		LOG_ERR("PS/2 mb binding failed");
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

void ps2_mouse_write(u8_t data)
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
