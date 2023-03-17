/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <soc.h>
#include <zephyr/logging/log.h>
#include "gpio_ec.h"
#include "periphmgmt.h"
#include "pwrbtnmgmt.h"
#include "board_config.h"
#include "acpi_region.h"
#include "smchost.h"
LOG_MODULE_DECLARE(periph, CONFIG_PERIPHERAL_LOG_LEVEL);

/* Debouncing is performed in 1 ms intervals.
 * Maximum debounce count is the debounce time specified in milliseconds.
 */
#define GPIO_DEBOUNCE_CNT	CONFIG_PERIPHERAL_DEBOUNCE_TIME

struct btn_info {
	uint32_t		port_pin;
	bool		debouncing;
	uint8_t		deb_cnt;
	btn_handler_t	handler;
	bool		prev_level;
	struct gpio_callback gpio_cb;
	char		*name;
};

static struct btn_info btn_lst[] = {
	{VOL_UP,          false, GPIO_DEBOUNCE_CNT, NULL, VOL_UP_INIT_POS,
					{{0}, NULL, 0}, "VolUp"},
	{PWRBTN_EC_IN_N,  false, GPIO_DEBOUNCE_CNT, NULL, PWR_BTN_INIT_POS,
					{{0}, NULL, 0}, "PwrBtn"},
	{VOL_DOWN,        false, GPIO_DEBOUNCE_CNT, NULL, VOL_DN_INIT_POS,
					{{0}, NULL, 0}, "VolDown"},
	{HOME_BUTTON,		false, GPIO_DEBOUNCE_CNT, NULL, HOME_INIT_POS,
					{{0}, NULL, 0}, "hmbtn"},
	{SMC_LID,        false, GPIO_DEBOUNCE_CNT, NULL, LID_INIT_POS,
					{{0}, NULL, 0}, "LidBtn"},
#if defined(VIRTUAL_BAT) || defined(VIRTUAL_DOCK)
	{VIRTUAL_BAT,    false, GPIO_DEBOUNCE_CNT, NULL, VIRTUAL_BAT_INIT_POS,
					{{0}, NULL, 0}, "VirBat"},
	{VIRTUAL_DOCK,   false, GPIO_DEBOUNCE_CNT, NULL, VIRTUAL_DOCK_INIT_POS,
					{{0}, NULL, 0}, "VirDock"},
#endif
#ifdef EC_SLATEMODE_HALLOUT_SNSR_R
	{EC_SLATEMODE_HALLOUT_SNSR_R, false, GPIO_DEBOUNCE_CNT, NULL,
				SLATEMODE_INIT_POS, {{0}, NULL, 0}, "Slatesw"},
#endif
#if defined(CONFIG_SOC_DEBUG_AWARENESS) && defined(TIMEOUT_DISABLE)
	{TIMEOUT_DISABLE, false, GPIO_DEBOUNCE_CNT, NULL, 1,
					{{0}, NULL, 0}, "Timeout"},
#endif
};

static int debouncing_ongoing;
static struct k_sem btn_debounce_lock;
static uint8_t io_sw_status;

static void notify_btn_handlers(uint8_t btn_idx)
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
static void gpio_level_change_callback(const struct device *dev,
		     struct gpio_callback *gpio_cb, uint32_t pins)
{
	struct btn_info *info = CONTAINER_OF(gpio_cb, struct btn_info, gpio_cb);

	LOG_DBG("%s level changed, starting debounce", info->name);
	info->debouncing = true;
	info->deb_cnt = GPIO_DEBOUNCE_CNT;


	k_sem_give(&btn_debounce_lock);
}

static void debounce_pins(void)
{
	debouncing_ongoing = 0;

	for (int i = 0; i < ARRAY_SIZE(btn_lst); i++) {
		if (btn_lst[i].debouncing) {
			btn_lst[i].deb_cnt--;

			if (btn_lst[i].deb_cnt == 0) {
				btn_lst[i].debouncing = false;
				btn_lst[i].deb_cnt = GPIO_DEBOUNCE_CNT;
				notify_btn_handlers(i);
			}
		}

		if (btn_lst[i].debouncing) {
			debouncing_ongoing++;
		}
	}
}

bool is_debouncing(void)
{
	LOG_INF("%s :%d ", __func__, debouncing_ongoing);
	return (debouncing_ongoing > 0);
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

int periph_register_button(uint32_t port_pin, btn_handler_t handler)
{
	int i;
	int ret;
	int level;

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

	/* Update button level from gpio pin status */
	level = gpio_read_pin(btn_lst[i].port_pin);

	if (level < 0) {
		LOG_ERR("Failed to read %x",
				gpio_get_pin(btn_lst[i].port_pin));
		return level;
	}

	btn_lst[i].prev_level = level;

	return ret;
}

void update_virtual_bat_dock_status(void)
{
	int level;

	level = gpio_read_pin(VIRTUAL_BAT);
	if (level < 0) {
		LOG_ERR("Fail to read virtual battery io expander");
	} else {
		g_acpi_tbl.acpi_flags2.vb_sw_closed = (level > 0) ? level : 0;
	}

	level = gpio_read_pin(VIRTUAL_DOCK);
	if (level < 0) {
		LOG_ERR("Fail to read virtual dock io expander");
	} else {
		g_acpi_tbl.acpi_flags2.pcie_docked =
			(level > 0) ?
			VIRTUAL_DOCK_CONNECTED : VIRTUAL_DOCK_DISCONNECTED;
	}
}

void update_switch_status(void)
{
	io_sw_status = 0;
	WRITE_BIT(io_sw_status, SWITCH_STATUS_TESTCRD_DET_POS,
		  gpio_read_pin(THERM_STRAP));
	WRITE_BIT(io_sw_status, SWITCH_STATUS_VIRTUAL_DOCK_POS,
		  gpio_read_pin(VIRTUAL_DOCK));
	WRITE_BIT(io_sw_status, SWITCH_STATUS_AC_POWER_POS,
		  gpio_read_pin(BC_ACOK));
	WRITE_BIT(io_sw_status, SWITCH_STATUS_HOME_BTN_POS,
		  (gpio_read_pin(HOME_BUTTON) == LOW));
	WRITE_BIT(io_sw_status, SWITCH_STATUS_VIRTUAL_BATT_POS,
		  gpio_read_pin(VIRTUAL_BAT));
	WRITE_BIT(io_sw_status, SWITCH_STATUS_LEGACY_LID,
		  gpio_read_pin(SMC_LID));
	LOG_DBG("%s io_sw_status:0x%X", __func__, io_sw_status);
}

uint8_t read_io_switch_status(void)
{
	return io_sw_status;
}

bool is_virtual_battery_prsnt(void)
{
	return g_acpi_tbl.acpi_flags2.vb_sw_closed ? false : true;
}

bool is_virtual_dock_prsnt(void)
{
	return g_acpi_tbl.acpi_flags2.pcie_docked;
}

void periph_thread(void *p1, void *p2, void *p3)
{
	uint32_t period = *(uint32_t *)p1;

	pwrbtn_init();

	/* Update the switch status */
	update_switch_status();

	k_sem_init(&btn_debounce_lock, 0, 1);
	while (true) {
		/* Wait until ISR occurs to start debouncing */
		k_sem_take(&btn_debounce_lock, K_FOREVER);

		do {
			/* Perform debounce for all buttons */
			k_msleep(period);
			debounce_pins();
		} while (is_debouncing());
	}
}
