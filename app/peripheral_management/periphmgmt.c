/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr.h>
#include <device.h>
#include <soc.h>
#include <logging/log.h>
#include "gpio_ec.h"
#include "periphmgmt.h"
#include "pwrbtnmgmt.h"
#include "board_config.h"
#include "eeprom.h"
#include "dswmode.h"
LOG_MODULE_DECLARE(periph, CONFIG_PERIPHERAL_LOG_LEVEL);

/* Stack area used by button task thread */
#define GPIO_DEB_TASK_STACK_SIZE    1024
/* scheduling priority used by each thread */
#define GPIO_DEBOUNCE_PRIORITY      13
#define GPIO_DEBOUNCE_TIME          5000
#define GPIO_DEBOUNCE_CNT           5

#define NUM_MCHP_PINS_PER_PORT      32

struct btn_info {
	u32_t		port_pin;
	bool		debouncing;
	u8_t		deb_cnt;
	btn_handler_t	handler;
	bool		prev_level;
	struct gpio_callback gpio_cb;
	char		*name;
};

struct btn_info btn_lst[] = {
	{VOL_UP,          false, GPIO_DEBOUNCE_CNT, NULL, VOL_UP_INIT_POS,
					{{0}, NULL, 0}, "VolUp"},
	{PWRBTN_EC_IN_N,  false, GPIO_DEBOUNCE_CNT, NULL, PWR_BTN_INIT_POS,
					{{0}, NULL, 0}, "PwrBtn"},
	{VOL_DOWN,        false, GPIO_DEBOUNCE_CNT, NULL, VOL_DN_INIT_POS,
					{{0}, NULL, 0}, "VolDown"},
};

void notify_btn_handlers(u8_t btn_idx)
{
	int level;

	level = gpio_read_pin(btn_lst[btn_idx].port_pin);
	if (level < 0) {
		LOG_ERR("Failed to read %x",
			gpio_get_pin(btn_lst[btn_idx].port_pin));
		return;
	}

	LOG_DBG("Btn[%s] valid change: %x", btn_lst[btn_idx].name, level);

	/* Ignore the button events if the debounce timings are not satisfied.
	 * This will address cases when input signal is noisy.
	 */
	if (btn_lst[btn_idx].handler) {
		if (btn_lst[btn_idx].prev_level != level) {
			LOG_DBG("calling handler");
			btn_lst[btn_idx].handler(level);
			btn_lst[btn_idx].prev_level = level;
		}
	}
}

/* Single callback to handle all button change events.
 * Buttons can be identified using container of gpio_cb structure.
 */
static void gpio_level_change_callback(struct device *dev,
		     struct gpio_callback *gpio_cb, u32_t pins)
{
	struct btn_info *info = CONTAINER_OF(gpio_cb, struct btn_info, gpio_cb);

	LOG_DBG("%s level changed, starting debounce", info->name);
	info->debouncing = true;
	info->deb_cnt = GPIO_DEBOUNCE_CNT;
}

static int periph_add_gpio_cb_for_button(int btn_index)
{
	struct btn_info *info = &btn_lst[btn_index];
	int ret;

	ret = gpio_init_callback_pin(info->port_pin, &info->gpio_cb,
			       gpio_level_change_callback);
	if (ret) {
		LOG_ERR("Failed to init callback for %s", info->name);
		return ret;
	}

	ret = gpio_add_callback_pin(info->port_pin, &info->gpio_cb);
	if (ret) {
		LOG_ERR("Failed to add callback for %s", info->name);
		return ret;
	}

	ret = gpio_interrupt_configure_pin(info->port_pin,
						   GPIO_INT_EDGE_BOTH);
	if (ret) {
		LOG_ERR("Failed to configure isr for %s", info->name);
	}

	LOG_DBG("Gpio callback registered for %s", info->name);
	return ret;
}

int periph_register_button(u32_t port_pin, btn_handler_t handler)
{
	int i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(btn_lst); i++) {
		if (btn_lst[i].port_pin == port_pin) {
			break;
		}
	}

	if (i == ARRAY_SIZE(btn_lst)) {
		LOG_ERR("Unknown button port: %d pin: %d",
				gpio_get_port(port_pin),
				gpio_get_pin(port_pin));
		return -EINVAL;
	}

	ret = periph_add_gpio_cb_for_button(i);
	if (!ret) {
		btn_lst[i].handler = handler;
	}
	return ret;
}

void periph_thread(void *p1, void *p2, void *p3)
{
	u32_t period = *(u32_t *)p1;

	eeprom_init();
	dsw_read_mode();
	pwrbtn_init();

	while (true) {
		k_msleep(period);

		/* Perform debounce for all buttons */
		for (int i = 0; i < ARRAY_SIZE(btn_lst); i++) {
			if (gpio_port_enabled(btn_lst[i].port_pin) &&
			    btn_lst[i].debouncing) {

				btn_lst[i].deb_cnt--;

				if (btn_lst[i].deb_cnt == 0) {
					btn_lst[i].debouncing = false;
					btn_lst[i].deb_cnt = GPIO_DEBOUNCE_CNT;
					notify_btn_handlers(i);
				}
			}
		}

		/* Update EEPROM if required */
		dsw_save_mode();
	}
}
