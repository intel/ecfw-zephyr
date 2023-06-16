/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/mutex.h>
#include <zephyr/app_memory/app_memdomain.h>
#include "board_config.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(i2c_hub, CONFIG_I2C_HUB_LOG_LEVEL);

/* Use macro so object goes into kernel object hash, which is required for user space */
K_MUTEX_DEFINE(i2c0_mutex);
K_MUTEX_DEFINE(i2c1_mutex);
K_MUTEX_DEFINE(i2c2_mutex);

struct i2c_dev_inst {
	const struct device *device;
	/* TODO: Remove seems macro cannot be used within a structure */

	/* K_MUTEX_DEFINE(mutex); */
	struct k_mutex *mutex;
	uint8_t speed;
};

K_APP_DMEM(ecfw_partition) struct i2c_dev_inst i2c_inst[] = {
	{
		.device = DEVICE_DT_GET(I2C_BUS_0),
		.mutex = &i2c0_mutex,
		/* Standard speed - 100kHz */
		.speed = I2C_SPEED_STANDARD,
	},
	{
		.device = DEVICE_DT_GET(I2C_BUS_1),
		.mutex = &i2c1_mutex,
		/* Fast speed - 400kHz */
		.speed = I2C_SPEED_FAST,
	},
#ifdef I2C_BUS_2
	{
		.device = DEVICE_DT_GET(I2C_BUS_2),
		.mutex = &i2c2_mutex,
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

	k_mutex_init(i2c_inst[instance].mutex);
	LOG_DBG("%s I2C%d config successfully", __func__, instance);
	return 0;
}

/* TODO: Find a better way to perform this operation */
#ifdef CONFIG_USERSPACE
void i2c_hub_allow_access(uint8_t instance, struct k_thread *tid)
{
	k_object_access_grant(i2c_inst[instance].mutex, tid);

	/* TODO: Remove this can be done within task_handler */
	k_object_access_grant(i2c_inst[instance].device, tid);
}
#endif

int i2c_hub_set_speed(uint8_t instance, uint8_t speed)
{
	int ret;

	if (instance >= NUM_OF_I2C_BUS) {
		return -ENODEV;
	}

	if (!i2c_inst[instance].device) {
		return -ENODEV;
	}

	k_mutex_lock(i2c_inst[instance].mutex, K_FOREVER);
	i2c_inst[instance].speed = speed;
	ret = i2c_configure(i2c_inst[instance].device,
			    I2C_SPEED_SET(i2c_inst[instance].speed) |
			    I2C_MODE_CONTROLLER);
	if (ret) {
		LOG_ERR("Error:%d failed to set i2c device speed", ret);
	}

	k_mutex_unlock(i2c_inst[instance].mutex);
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

	k_mutex_lock(i2c_inst[instance].mutex, K_FOREVER);

	ret = i2c_write(i2c_inst[instance].device, buf, num_bytes, addr);

	k_mutex_unlock(i2c_inst[instance].mutex);
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

	k_mutex_lock(i2c_inst[instance].mutex, K_FOREVER);
	ret = i2c_read(i2c_inst[instance].device, buf, num_bytes, addr);
	k_mutex_unlock(i2c_inst[instance].mutex);
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

	k_mutex_lock(i2c_inst[instance].mutex, K_FOREVER);
	ret = i2c_write_read(i2c_inst[instance].device, addr, write_buf,
			     num_write, read_buf, num_read);
	k_mutex_unlock(i2c_inst[instance].mutex);
	LOG_WRN("%s err %d", __func__, ret);

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

	k_mutex_lock(i2c_inst[instance].mutex, K_FOREVER);
	ret = i2c_burst_read(i2c_inst[instance].device, dev_addr,
				reg_addr, value, len);
	k_mutex_unlock(i2c_inst[instance].mutex);

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

	k_mutex_lock(i2c_inst[instance].mutex, K_FOREVER);
	ret = i2c_burst_write(i2c_inst[instance].device, dev_addr,
				reg_addr, value, len);
	k_mutex_unlock(i2c_inst[instance].mutex);

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

	k_mutex_lock(i2c_inst[instance].mutex, K_FOREVER);
	ret = i2c_target_register(i2c_inst[instance].device, cfg);
	k_mutex_unlock(i2c_inst[instance].mutex);

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

	k_mutex_lock(i2c_inst[instance].mutex, K_FOREVER);
	ret = i2c_target_unregister(i2c_inst[instance].device, cfg);
	k_mutex_unlock(i2c_inst[instance].mutex);

	return ret;
}
