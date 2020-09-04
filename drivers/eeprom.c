/*
 * Copyright (c) 2019 Intel Corporation
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/i2c.h>
#include "board.h"
#include "board_config.h"
#include "eeprom.h"
#include "memops.h"
#include <logging/log.h>
LOG_MODULE_REGISTER(eeprom, CONFIG_EEPROM_LOG_LEVEL);

/* LAN enable/disable status */
#define EEPROM_LANSTS               0x00
/* EEPROM write delay 5ms */
#define EEPROM_WR_MSDELAY           5

#define EEPROM_DEFAULT_DATA         0xFFu
#define DATA_MAX_LEN                31

#define OFS_MSB(word)  (((word & 0xFF00) >> 8))
#define OFS_LSB(word)  (word & 0xFF)

static struct device *i2c_dev;

void eeprom_init(void)
{
	i2c_dev = device_get_binding(I2C_BUS_0);
	if (!i2c_dev) {
		LOG_WRN("%s not found", I2C_BUS_0);
	}
}

/* EEPROM access for offset greater than 255.
 * Following the 4-bit device type identifier in the bits 3-1 of the device
 * slave address byte are bits A10, A9 and A8 which are the three MSB of the
 * memory array word address
 * i.e. offset 0x0006 correspond to device address 0x50, offset 0x06
 * i.e. offset 0x0106 correspond to device address 0x51, offset 0x00
 */

int eeprom_read_byte(u16_t offset, u8_t *data)
{
	u8_t ret;
	u8_t buf = OFS_MSB(offset);

	ret = i2c_write_read(i2c_dev, EEPROM_DRIVER_I2C_ADDR | OFS_MSB(offset),
			     &buf, sizeof(buf),
			     data, 1);
	if (ret) {
		LOG_ERR("Fail to read: %d", ret);
		return ret;
	}

	return 0;
}

int eeprom_write_byte(u16_t offset, u8_t data)
{
	u8_t ret;
	u8_t buf[] = { OFS_LSB(offset), data };

	ret = i2c_write(i2c_dev, buf, sizeof(buf),
			EEPROM_DRIVER_I2C_ADDR | OFS_MSB(offset));
	if (ret) {
		LOG_ERR("Fail to write: %d", ret);
		return ret;
	}

	k_msleep(EEPROM_WR_MSDELAY);

	return 0;
}

int eeprom_read_word(u16_t offset, u16_t *data)
{
	u8_t ret;
	u8_t buf = { OFS_LSB(offset) };
	u8_t rbuf[] = {EEPROM_DEFAULT_DATA, EEPROM_DEFAULT_DATA};

	ret = i2c_write_read(i2c_dev, EEPROM_DRIVER_I2C_ADDR | OFS_MSB(offset),
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

int eeprom_write_word(u16_t offset, u16_t data)
{
	u16_t ret;
	u8_t buf[] = { OFS_LSB(offset),
		       OFS_MSB(data), OFS_LSB(data) };

	ret = i2c_write(i2c_dev, buf, sizeof(buf),
			EEPROM_DRIVER_I2C_ADDR | OFS_MSB(offset));
	if (ret) {
		LOG_ERR("Fail to write: %d", ret);
		return ret;
	}

	/* Delay to wait for write completion */
	k_msleep(EEPROM_WR_MSDELAY);

	return 0;
}

int eeprom_read_block(u16_t offset, u8_t len, u8_t *data)
{
	u16_t ret;
	u8_t buf = { OFS_LSB(offset) };

	if (len > DATA_MAX_LEN) {
		return -EINVAL;
	}

	ret = i2c_write_read(i2c_dev, EEPROM_DRIVER_I2C_ADDR | OFS_MSB(offset),
			     &buf, sizeof(buf),
			     data, len);
	if (ret) {
		LOG_ERR("Fail to read: %d", ret);
		return -EIO;
	}

	return ret;
}

int eeprom_write_block(u16_t offset, u8_t len, u8_t *data)
{
	int ret;
	u8_t buf[DATA_MAX_LEN + sizeof(offset)];

	if (len > DATA_MAX_LEN) {
		return -EINVAL;
	}

	buf[0] = OFS_LSB(offset);
	ret = memcpys(&buf[1], data, len);

	if (ret) {
		LOG_ERR("Fail during buffer copy: %d", ret);
		return ret;
	}

	ret = i2c_write(i2c_dev, buf, len + sizeof(offset),
			EEPROM_DRIVER_I2C_ADDR | OFS_MSB(offset));
	if (ret) {
		LOG_ERR("Fail to write: %d", ret);
		return ret;
	}

	k_msleep(EEPROM_WR_MSDELAY);

	return ret;
}
