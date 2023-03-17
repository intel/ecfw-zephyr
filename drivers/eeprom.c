/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include "i2c_hub.h"
#include "board.h"
#include "board_config.h"
#include "eeprom.h"
#include "memops.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(eeprom, CONFIG_EEPROM_LOG_LEVEL);

/* LAN enable/disable status */
#define EEPROM_LANSTS               0x00
/* EEPROM write delay 5ms */
#define EEPROM_WR_MSDELAY           5

#define EEPROM_DEFAULT_DATA         0xFFu
#define DATA_MAX_LEN                31

#define OFS_MSB(word)  (((word & 0xFF00) >> 8))
#define OFS_LSB(word)  (word & 0xFF)

/* EEPROM access for offset greater than 255.
 * Following the 4-bit device type identifier in the bits 3-1 of the device
 * slave address byte are bits A10, A9 and A8 which are the three MSB of the
 * memory array word address
 * i.e. offset 0x0006 correspond to device address 0x50, offset 0x06
 * i.e. offset 0x0106 correspond to device address 0x51, offset 0x00
 */

int eeprom_read_byte(uint16_t offset, uint8_t *data)
{
	uint8_t ret;
	uint8_t buf = OFS_MSB(offset);

	ret = i2c_hub_write_read(I2C_0,
			     EEPROM_DRIVER_I2C_ADDR | OFS_MSB(offset),
			     &buf, sizeof(buf),
			     data, 1);
	if (ret) {
		LOG_ERR("Fail to read: %d", ret);
		return ret;
	}

	return 0;
}

int eeprom_write_byte(uint16_t offset, uint8_t data)
{
	uint8_t ret;
	uint8_t buf[] = { OFS_LSB(offset), data };

	ret = i2c_hub_write(I2C_0, buf, sizeof(buf),
			EEPROM_DRIVER_I2C_ADDR | OFS_MSB(offset));
	if (ret) {
		LOG_ERR("Fail to write: %d", ret);
		return ret;
	}

	k_msleep(EEPROM_WR_MSDELAY);

	return 0;
}

int eeprom_read_word(uint16_t offset, uint16_t *data)
{
	uint8_t ret;
	uint8_t buf = { OFS_LSB(offset) };
	uint8_t rbuf[] = {EEPROM_DEFAULT_DATA, EEPROM_DEFAULT_DATA};

	ret = i2c_hub_write_read(I2C_0,
			     EEPROM_DRIVER_I2C_ADDR | OFS_MSB(offset),
			     &buf, sizeof(buf),
			     rbuf, sizeof(rbuf));
	if (ret) {
		LOG_ERR("Fail to read: %d", ret);
		return ret;
	}

	/* Adjust endianness */
	*data = ((rbuf[0] << 8) | rbuf[1]);

	return 0;
}

int eeprom_write_word(uint16_t offset, uint16_t data)
{
	uint16_t ret;
	uint8_t buf[] = { OFS_LSB(offset),
		       OFS_MSB(data), OFS_LSB(data) };

	ret = i2c_hub_write(I2C_0, buf, sizeof(buf),
			EEPROM_DRIVER_I2C_ADDR | OFS_MSB(offset));
	if (ret) {
		LOG_ERR("Fail to write: %d", ret);
		return ret;
	}

	/* Delay to wait for write completion */
	k_msleep(EEPROM_WR_MSDELAY);

	return 0;
}

int eeprom_read_block(uint16_t offset, uint8_t len, uint8_t *data)
{
	uint16_t ret;
	uint8_t buf = { OFS_LSB(offset) };

	if (len > DATA_MAX_LEN) {
		return -EINVAL;
	}

	ret = i2c_hub_write_read(I2C_0,
			     EEPROM_DRIVER_I2C_ADDR | OFS_MSB(offset),
			     &buf, sizeof(buf),
			     data, len);
	if (ret) {
		LOG_ERR("Fail to read: %d", ret);
		return -EIO;
	}

	return ret;
}

int eeprom_write_block(uint16_t offset, uint8_t len, uint8_t *data)
{
	int ret;
	uint8_t buf[DATA_MAX_LEN + sizeof(offset)];

	if (len > DATA_MAX_LEN) {
		return -EINVAL;
	}

	buf[0] = OFS_LSB(offset);
	ret = memcpys(&buf[1], data, len);

	if (ret) {
		LOG_ERR("Fail during buffer copy: %d", ret);
		return ret;
	}

	ret = i2c_hub_write(I2C_0, buf, len + sizeof(offset),
			EEPROM_DRIVER_I2C_ADDR | OFS_MSB(offset));
	if (ret) {
		LOG_ERR("Fail to write: %d", ret);
		return ret;
	}

	k_msleep(EEPROM_WR_MSDELAY);

	return ret;
}
