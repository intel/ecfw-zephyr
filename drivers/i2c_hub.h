/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef I2C_HUB_H_
#define I2C_HUB_H_

#include <zephyr/types.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>

enum i2c_bus_num {
	I2C_0,
	I2C_1,
	I2C_2,
	I2C_3,
	I2C_4,
};

/**
 * @brief Set up the i2c controller
 *
 * @param instance I2C port number which needs to be configured
 *
 * @retval 0 If successful.
 * @retval -EIO General input / output error, failed to configure device.
 * @retval -EINVAL Invalid device handler, failed to configure device.
 */
int i2c_hub_config(uint8_t instance);

/**
 * @brief Set up the i2c controller Speed
 *
 * @param instance I2C port number whose speed needs to be changed
 *
 * @retval 0 If successful.
 * @retval -EIO General input / output error.
 */
int i2c_hub_set_speed(uint8_t instance, uint8_t speed);

/**
 * @brief Write a set amount of data to an I2C device.
 *
 * This routine writes a set amount of data synchronously.
 *
 * @param instance I2C port number to be used for writing.
 * @param buf Memory pool from which the data is transferred.
 * @param num_bytes Number of bytes to write.
 * @param addr Address to the target I2C device for writing.
 *
 * @retval 0 If successful.
 * @retval -EIO General input / output error.
 */
int i2c_hub_write(uint8_t instance, const uint8_t *buf, uint32_t num_bytes,
		  uint16_t addr);

/**
 * @brief Read a set amount of data from an I2C device.
 *
 * This routine reads a set amount of data synchronously.
 *
 * @param instance I2C port number to be used for reading.
 * @param buf Memory pool to which the data is transferred.
 * @param num_bytes Number of bytes to read..
 * @param addr Address to the target I2C device for reading.
 *
 * @retval 0 If successful.
 * @retval -EIO General input / output error.
 */
int i2c_hub_read(uint8_t instance, uint8_t *buf, uint32_t num_bytes,
		  uint16_t addr);

/**
 * @brief Write then read data from an I2C device.
 *
 * This supports the common operation "this is what I want", "now give
 * it to me" transaction pair through a combined write-then-read bus
 * transaction.
 *
 * @param instance I2C port number to be used for read/write access.
 * @param addr Address of the I2C device
 * @param write_buf Pointer to the data to be written
 * @param num_write Number of bytes to write
 * @param read_buf Pointer to storage for read data
 * @param num_read Number of bytes to read
 *
 * @retval 0 if successful
 * @retval negative on error.
 */
int i2c_hub_write_read(uint8_t instance, uint16_t addr, const void *write_buf,
			size_t num_write, void *read_buf, size_t num_read);

/**
 * @brief read a set amount of data to an I2C device.
 *
 * This routine read a set amount of data synchronously.
 *
 * @param instance I2C port number to be used for reading.
 * @param dev_addr Address of the I2C device
 * @param reg_addr from where the data is transferred.
 * @param value Memory pool from which the data is transferred.
 * @param len Number of bytes to read.
 *
 * @retval 0 If successful.
 * @retval -EIO General input / output error.
 */
int i2c_hub_burst_read(uint8_t instance, uint16_t dev_addr,
		  uint8_t reg_addr, uint8_t *value, uint32_t len);

/**
 * @brief write a set amount of data to an I2C device.
 *
 * This routine write a set amount of data synchronously.
 *
 * @param instance I2C port number to be used for reading.
 * @param dev_addr Address of the I2C device
 * @param reg_addr from where the data is transferred.
 * @param value Memory pool from which the data is transferred.
 * @param len Number of bytes to write.
 *
 * @retval 0 If successful.
 * @retval -EIO General input / output error.
 */

int i2c_hub_burst_write(uint8_t instance, uint16_t dev_addr,
		  uint8_t reg_addr, uint8_t *value, uint32_t len);

/*
 * @brief Registers the provided I2C port number as Slave device
 *
 * Enable I2C slave mode for the supplied I2C bus using the provided
 * 'config' struct containing the functions and parameters to send bus
 * events. The I2C slave will be registered at the address provided as 'address'
 * struct member. Addressing mode - 7 or 10 bit - depends on the 'flags'
 * struct member. Any I2C bus events related to the slave mode will be passed
 * onto I2C slave device driver via a set of callback functions provided in
 * the 'callbacks' struct member.
 *
 * Most of the existing hardware allows simultaneous support for master
 * and slave mode. This is however not guaranteed.
 *
 * @param instance I2C port number which needs to be put in slave mode.
 * @param cfg Config struct with functions and parameters used by the I2C driver
 * to send bus events
 *
 * @retval 0 Is successful
 * @retval -EINVAL If parameters are invalid
 * @retval -EIO General input / output error.
 * @retval -ENOTSUP If slave mode is not supported
 */
int i2c_hub_slave_register(uint8_t instance, struct i2c_target_config *cfg);

/**
 * @brief Unregisters the provided config as Slave device
 *
 * This routine disables I2C slave mode for the supplied I2C bus using
 * the provided 'config' struct containing the functions and parameters
 * to send bus events.
 *
 * @param instance I2C port number
 * @param cfg Config struct with functions and parameters used by the I2C driver
 * to send bus events
 *
 * @retval 0 Is successful
 * @retval -EINVAL If parameters are invalid
 * @retval -ENOTSUP If slave mode is not supported
 */
int i2c_hub_slave_unregister(uint8_t instance, struct i2c_target_config *cfg);
#endif
