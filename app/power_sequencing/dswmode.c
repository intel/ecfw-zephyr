/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include "eeprom.h"
#include "errcodes.h"
#include "dswmode.h"
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(pwrmgmt, CONFIG_PWRMGT_LOG_LEVEL);

#define DSW_VALID_MASK    0xAA00
#define DSW_MODE_MASK     0x00FF
#define DSW_NO_DATA       0x0000

/* Cached configuration data */
static uint8_t dsw_valid_config;

/* New configuration data not yet stored */
static uint8_t dsw_tmp_config;
static bool dsw_mode_update;

bool dsw_enabled(void)
{
#ifndef CONFIG_PWRMGMT_DEEPSX
	return false;
#endif
	return dsw_valid_config != 0;
}

uint8_t dsw_mode(void)
{
#ifndef CONFIG_PWRMGMT_DEEPSX
	return DSW_DISABLED;
#endif
	LOG_DBG("Current Dsx mode %x", dsw_valid_config);

	return dsw_valid_config;
}

void dsw_update_mode(uint8_t mode)
{
	LOG_DBG("Update DSx mode %x", mode);

	dsw_tmp_config = mode;
	dsw_mode_update = true;
}

void dsw_read_mode(void)
{
	int ret;
	uint16_t value;

	ret = eeprom_read_word(EEPROM_DSW_OFFSET, &value);
	if (ret) {
		LOG_ERR("Unable to read EEPROM %d", ret);
		return;
	}

	LOG_DBG("Read EEPROM dsw config: %x", value);

	if (value & DSW_VALID_MASK) {
		dsw_valid_config = value & DSW_MODE_MASK;

		LOG_DBG("dsw_mode %x", dsw_valid_config);
		LOG_DBG("dsw_enabled %x", dsw_enabled());
	}
}

void dsw_save_mode(void)
{
	uint16_t data;

	if (dsw_mode_update) {
		dsw_mode_update = false;

		if (dsw_tmp_config != dsw_valid_config) {
			dsw_valid_config = dsw_tmp_config;

			data = (dsw_valid_config & DSW_MODE_MASK) |
				DSW_VALID_MASK;
			LOG_DBG("dsw_enabled %x", dsw_enabled());
			LOG_DBG("Save %x to EEPROM", data);
			eeprom_write_word(EEPROM_DSW_OFFSET, data);
		}
	}
}
