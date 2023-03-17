/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include "board_config.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(i2c_hub, CONFIG_I2C_HUB_LOG_LEVEL);

struct i2c_dev_inst {
	const struct device *device;
	uint8_t speed;
	struct k_mutex mutex;
};

static struct i2c_dev_inst i2c_inst[] = {
	{
		.device = DEVICE_DT_GET(I2C_BUS_0),
		/* Standard speed - 100kHz */
		.speed = I2C_SPEED_STANDARD,
	},
	{
		.device = DEVICE_DT_GET(I2C_BUS_1),
		/* Fast speed - 400kHz */
		.speed = I2C_SPEED_FAST,
	},
#ifdef I2C_BUS_2
	{
		.device = DEVICE_DT_GET(I2C_BUS_2),
		/* Standard speed - 100kHz */
		.speed = I2C_SPEED_STANDARD,
	},
#endif
};

#define NUM_OF_I2C_BUS		ARRAY_SIZE(i2c_inst)

int i2c_hub_config(uint8_t instance)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}

	if (!device_is_ready(i2c_inst[instance].device)) {
		LOG_ERR("i2c%d not ready", instance);
		return -EINVAL;
	}

	ret = i2c_configure(i2c_inst[instance].device,
			    I2C_SPEED_SET(i2c_inst[instance].speed) |
			    I2C_MODE_CONTROLLER);
	if (ret) {
		LOG_ERR("Error:%d failed to configure i2c device", ret);
		return ret;
	}

	k_mutex_init(&i2c_inst[instance].mutex);

	return 0;
}

int i2c_hub_set_speed(uint8_t instance, uint8_t speed)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_inst[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_inst[instance].mutex, K_FOREVER);
	i2c_inst[instance].speed = speed;
	ret = i2c_configure(i2c_inst[instance].device,
			    I2C_SPEED_SET(i2c_inst[instance].speed) |
			    I2C_MODE_CONTROLLER);
	if (ret) {
		LOG_ERR("Error:%d failed to set i2c device speed", ret);
	}

	k_mutex_unlock(&i2c_inst[instance].mutex);
	return ret;
}

int i2c_hub_write(uint8_t instance, const uint8_t *buf,
		  uint32_t num_bytes, uint16_t addr)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_inst[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_inst[instance].mutex, K_FOREVER);
	ret = i2c_write(i2c_inst[instance].device, buf, num_bytes, addr);
	k_mutex_unlock(&i2c_inst[instance].mutex);

	return ret;
}

int i2c_hub_read(uint8_t instance, uint8_t *buf,
		  uint32_t num_bytes, uint16_t addr)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_inst[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_inst[instance].mutex, K_FOREVER);
	ret = i2c_read(i2c_inst[instance].device, buf, num_bytes, addr);
	k_mutex_unlock(&i2c_inst[instance].mutex);

	return ret;
}

int i2c_hub_write_read(uint8_t instance, uint16_t addr, const void *write_buf,
		       size_t num_write, void *read_buf, size_t num_read)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_inst[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_inst[instance].mutex, K_FOREVER);
	ret = i2c_write_read(i2c_inst[instance].device, addr, write_buf,
			     num_write, read_buf, num_read);
	k_mutex_unlock(&i2c_inst[instance].mutex);

	return ret;
}

int i2c_hub_burst_read(uint8_t instance, uint16_t dev_addr,
		  uint8_t reg_addr, uint8_t *value, uint32_t len)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_inst[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_inst[instance].mutex, K_FOREVER);
	ret = i2c_burst_read(i2c_inst[instance].device, dev_addr,
				reg_addr, value, len);
	k_mutex_unlock(&i2c_inst[instance].mutex);

	return ret;
}

int i2c_hub_burst_write(uint8_t instance, uint16_t dev_addr,
		  uint8_t reg_addr, uint8_t *value, uint32_t len)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_inst[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_inst[instance].mutex, K_FOREVER);
	ret = i2c_burst_write(i2c_inst[instance].device, dev_addr,
				reg_addr, value, len);
	k_mutex_unlock(&i2c_inst[instance].mutex);

	return ret;
}

int i2c_hub_slave_register(uint8_t instance, struct i2c_target_config *cfg)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_inst[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_inst[instance].mutex, K_FOREVER);
	ret = i2c_target_register(i2c_inst[instance].device, cfg);
	k_mutex_unlock(&i2c_inst[instance].mutex);

	return ret;
}

int i2c_hub_slave_unregister(uint8_t instance, struct i2c_target_config *cfg)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}
	if (!i2c_inst[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(&i2c_inst[instance].mutex, K_FOREVER);
	ret = i2c_target_unregister(i2c_inst[instance].device, cfg);
	k_mutex_unlock(&i2c_inst[instance].mutex);

	return ret;
}
