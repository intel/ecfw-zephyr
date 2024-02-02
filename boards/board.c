/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pinctrl.h>
#include <soc.h>
#include "i2c_hub.h"
#include <zephyr/logging/log.h>
#include "gpio_ec.h"
#include "board_config.h"
#include "board.h"
#include "flashhdr.h"
#include "errcodes.h"

LOG_MODULE_REGISTER(board, CONFIG_BOARD_LOG_LEVEL);

static uint16_t plat_data;
static uint16_t io_data;

#if DT_NODE_EXISTS(DT_PATH(zephyr_user))
/* constant (code space) PINCTRL entry referencing our zephyr_user node */
PINCTRL_DT_DEFINE(DT_PATH(zephyr_user));

/* constant (code space) pointer to constant PINCTRL device structure */
const struct pinctrl_dev_config *app_pinctrl_cfg =
	PINCTRL_DT_DEV_CONFIG_GET(DT_PATH(zephyr_user));
#endif

inline int board_dts_pin_muxing(void)
{
#if DT_NODE_EXISTS(DT_PATH(zephyr_user))
	int ret = pinctrl_apply_state(app_pinctrl_cfg, PINCTRL_STATE_DEFAULT);

	LOG_DBG("User-defined dts node executed :%d", ret);
	return ret;
#else
	return 0;
#endif
}

int board_devices_check(void)
{
	const struct device *dev;
	const struct device *devlist_end;
	size_t ndevs;

	ndevs = z_device_get_all_static(&dev);
	devlist_end = dev + ndevs;

	if (ndevs == 0) {
		LOG_ERR("No devices initialized!");
		return -ENODEV;
	}

	while (dev < devlist_end) {
		if ((dev->name != NULL) && (strlen(dev->name) != 0)) {
			if (z_device_is_ready(dev)) {
				LOG_DBG("%s ready", dev->name);
			} else {
				LOG_WRN("%s not ready. Check dts", dev->name);
				printk("%s not ready. Check dts\n", dev->name);
				return -ENODEV;
			}
		} else {
			LOG_WRN("Device with no name");
		}

		dev++;
	}

	/* Apply board user-define pinctrl */
	return board_dts_pin_muxing();
}

static inline int read_rvp_board_id(uint8_t *data)
{
	int ret = 0;
	uint8_t reg_addr[] = { 0x00 };

	ret = i2c_hub_write_read(I2C_0, IO_EXPANDER_0_I2C_ADDR,
				 reg_addr, 1, data, 2);
	if (ret) {
		LOG_ERR("Board id fail (%d)", ret);
		/* TODO: Generate postcode error */
	}

	return ret;
}

int read_board_id(void)
{
	int ret = 0;
	uint8_t data[] = { 0x00, 0x00 };

	/* Read the board id */
#ifdef CONFIG_BOARD_ID_IO_EXPANDER
	ret = read_rvp_board_id(data);
#endif

	/* Platform information for internal consumption */
	io_data = PLATFORM_DATA(data[1], data[0]);

	/* Platform information BIOS consumption */
	plat_data = PLATFORM_DATA(data[0], data[1]);

	LOG_INF("plat_data for BIOS %x", plat_data);
	LOG_INF("HW id: %x", get_hw_id());
	LOG_INF("Board id: %x", get_board_id());
	LOG_INF("Fab id: %x", get_fab_id());
	LOG_INF("HW straps: %x", get_hw_straps());
	LOG_INF("Family %s", KSC_PLAT_NAME);
	LOG_INF("EC FW version %d.%02d", major_version(), minor_version());

	return ret;
}

uint8_t get_hw_id(void)
{
	return io_data & HW_ID_MASK;
}

uint8_t get_board_id(void)
{
	return (io_data & BOARD_ID_MASK) >> BOARD_ID_OFFSET;
}

uint8_t get_fab_id(void)
{
	return (io_data & FAB_ID_MASK) >> FAB_ID_OFFSET;
}

uint8_t get_hw_straps(void)
{
	return (io_data & HW_STRAP_MASK) >> HW_STRAP_OFFSET;
}

uint16_t get_platform_id(void)
{
	return plat_data;
}
