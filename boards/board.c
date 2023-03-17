/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
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
