/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <device.h>
#include <drivers/i2c.h>
#include "board_config.h"
#include <logging/log.h>
LOG_MODULE_REGISTER(i2c_hub, CONFIG_I2C_HUB_LOG_LEVEL);


struct i2c_hub_struct {
	const struct device *device;
	struct k_mutex mutex;
};

struct i2c_dev_inst {
	char *i2c_inst;
	uint8_t speed;
};

static struct i2c_dev_inst i2c_inst[] = {
	{
		.i2c_inst = I2C_BUS_0,
		/* Standard speed - 100kHz */
		.speed = I2C_SPEED_STANDARD,
	},
	{
		.i2c_inst = I2C_BUS_1,
		/* Fast speed - 400kHz */
		.speed = I2C_SPEED_FAST,
	},
#ifdef I2C_BUS_2
	{
		.i2c_inst = I2C_BUS_2,
		/* Standard speed - 100kHz */
		.speed = I2C_SPEED_STANDARD,
	},
#endif
};

#define NUM_OF_I2C_BUS		ARRAY_SIZE(i2c_inst)

static struct i2c_hub_struct i2c_dev[NUM_OF_I2C_BUS];


int i2c_hub_config(uint8_t instance)
{
	int ret;
	const struct device *dev;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}

	dev = device_get_binding(i2c_inst[instance].i2c_inst);
	if (!dev) {
		LOG_ERR("%s not found", i2c_inst[instance].i2c_inst);
		return -EINVAL;
	}

	ret = i2c_configure(dev,
			    I2C_SPEED_SET(i2c_inst[instance].speed) |
			    I2C_MODE_MASTER);
	if (ret) {
		LOG_ERR("Error:%d failed to configure i2c device", ret);
		return ret;
	}

	k_mutex_init(&i2c_dev[instance].mutex);
	i2c_dev[instance].device = dev;

	return 0;
}

int i2c_hub_set_speed(uint8_t instance, uint8_t speed)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_dev[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_dev[instance].mutex, K_FOREVER);
	i2c_inst[instance].speed = speed;
	ret = i2c_configure(i2c_dev[instance].device,
			    I2C_SPEED_SET(i2c_inst[instance].speed) |
			    I2C_MODE_MASTER);
	if (ret) {
		LOG_ERR("Error:%d failed to set i2c device speed", ret);
	}

	k_mutex_unlock(&i2c_dev[instance].mutex);
	return ret;
}

int i2c_hub_write(uint8_t instance, const uint8_t *buf,
		  uint32_t num_bytes, uint16_t addr)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_dev[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_dev[instance].mutex, K_FOREVER);
	ret = i2c_write(i2c_dev[instance].device, buf, num_bytes, addr);
	k_mutex_unlock(&i2c_dev[instance].mutex);

	return ret;
}

int i2c_hub_write_read(uint8_t instance, uint16_t addr, const void *write_buf,
		       size_t num_write, void *read_buf, size_t num_read)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_dev[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_dev[instance].mutex, K_FOREVER);
	ret = i2c_write_read(i2c_dev[instance].device, addr, write_buf,
			     num_write, read_buf, num_read);
	k_mutex_unlock(&i2c_dev[instance].mutex);

	return ret;
}

int i2c_hub_burst_read(uint8_t instance, uint16_t dev_addr,
		  uint8_t reg_addr, uint8_t *value, uint32_t len)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_dev[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_dev[instance].mutex, K_FOREVER);
	ret = i2c_burst_read(i2c_dev[instance].device, dev_addr,
				reg_addr, value, len);
	k_mutex_unlock(&i2c_dev[instance].mutex);

	return ret;
}

int i2c_hub_burst_write(uint8_t instance, uint16_t dev_addr,
		  uint8_t reg_addr, uint8_t *value, uint32_t len)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_dev[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_dev[instance].mutex, K_FOREVER);
	ret = i2c_burst_write(i2c_dev[instance].device, dev_addr,
				reg_addr, value, len);
	k_mutex_unlock(&i2c_dev[instance].mutex);

	return ret;
}

int i2c_hub_slave_register(uint8_t instance, struct i2c_slave_config *cfg)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_dev[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_dev[instance].mutex, K_FOREVER);
	ret = i2c_slave_register(i2c_dev[instance].device, cfg);
	k_mutex_unlock(&i2c_dev[instance].mutex);

	return ret;
}

int i2c_hub_slave_unregister(uint8_t instance, struct i2c_slave_config *cfg)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_dev[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_dev[instance].mutex, K_FOREVER);
	ret = i2c_slave_unregister(i2c_dev[instance].device, cfg);
	k_mutex_unlock(&i2c_dev[instance].mutex);

	return ret;
}
