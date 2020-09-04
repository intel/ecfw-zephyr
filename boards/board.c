/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr.h>
#include <device.h>
#include <soc.h>
#include <drivers/i2c.h>
#include <logging/log.h>
#include "gpio_ec.h"
#include "board_config.h"
#include "board.h"
#include "errcodes.h"
#include "pwrplane.h"

LOG_MODULE_REGISTER(board, CONFIG_BOARD_LOG_LEVEL);

static u16_t plat_data;
static u16_t io_data;

int read_board_id(void)
{
	int ret;
	u8_t reg_addr[] = { 0x00 };
	u8_t data[] = { 0x00, 0x00 };
	struct device *i2c_dev;

	i2c_dev = device_get_binding(I2C_BUS_0);
	if (!i2c_dev) {
		LOG_WRN("Fail to bind");
		return -EINVAL;
	}

	ret = i2c_write_read(i2c_dev, IO_EXPANDER_0_I2C_ADDR,
			     reg_addr, 1, data, 2);
	if (ret) {
		LOG_ERR("Board id fail (%d)", ret);
		/* TODO: Generate postcode error */
		return ret;
	}

	/* Platform information for internal consumption */
	io_data = PLATFORM_DATA(data[1], data[0]);

	/* Platform information BIOS consumption */
	plat_data = PLATFORM_DATA(data[0], data[1]);

	LOG_INF("plat_data for BIOS %x", plat_data);
	LOG_INF("HW id: %x", get_hw_id());
	LOG_INF("Board id: %x", get_board_id());
	LOG_INF("Fab id: %x", get_fab_id());
	LOG_INF("HW straps: %x", get_hw_straps());

	return ret;
}

u8_t get_hw_id(void)
{
	return io_data & HW_ID_MASK;
}

u8_t get_board_id(void)
{
	return (io_data & BOARD_ID_MASK) >> BOARD_ID_OFFSET;
}

u8_t get_fab_id(void)
{
	return (io_data & FAB_ID_MASK) >> FAB_ID_OFFSET;
}

u8_t get_hw_straps(void)
{
	return (io_data & HW_STRAP_MASK) >> HW_STRAP_OFFSET;
}

u16_t get_platform_id(void)
{
	return plat_data;
}
