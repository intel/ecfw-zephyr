/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __GPIO_DRIVER_H__
#define __GPIO_DRIVER_H__

#include <drivers/gpio.h>

/**
 * @brief GPIO driver wrapper APIS.
 */
#define EC_GPIO_PORT_POS	8U
#define EC_GPIO_PORT_MASK	((u32_t) 0xfU << EC_GPIO_PORT_POS)
#define EC_GPIO_PIN_POS		0U
#define EC_GPIO_PIN_MASK	((u32_t)0x1fU << EC_GPIO_PIN_POS)

#define EC_GPIO_PORT_PIN(_port, _pin)                   \
	(((u32_t)(_port) << EC_GPIO_PORT_POS)   |       \
	((u32_t)(_pin) << EC_GPIO_PIN_POS))             \

/*
 * This structure is used to pass an array of GPIOs and settings to
 * this driver wrapper. It encodes the gpio device and the pin in a
 * single variable.
 */
struct gpio_ec_config {
	u32_t port_pin;
	u32_t cfg;
};

static inline u32_t gpio_get_port(u16_t pin)
{
	return  ((pin & EC_GPIO_PORT_MASK) >> EC_GPIO_PORT_POS);
}

static inline u32_t gpio_get_pin(u16_t pin)
{
	return ((pin & EC_GPIO_PIN_MASK) >> EC_GPIO_PIN_POS);
}

/**
 * @brief Initialize the gpio ports.
 *
 * @retval 0 if successful, negative errno code on failure.
 */
int gpio_init(void);

/**
 * @brief Configure a specific gpio pin.
 *
 * Wrapper interface to configure a gpio pin using the standard Zephyr flags.
 * The first parameter hides the port number and pin.
 *
 * @param port_pin Encoded device and pin.
 * @param flags Standard GPIO driver flags to configure a pin.
 *
 * @retval 0 if successful, negative errno code on failure.
 */
int gpio_configure_pin(u32_t port_pin, gpio_flags_t flags);

/**
 * @brief Configure an array of gpios.
 *
 * Wrapper to configure several GPIOs by receiving
 * an array containing the encoded Port/Pin, flags and direction.
 *
 * @param gpios Array containing device and pin, direction.
 * @param len Array size.
 *
 * @retval 0 if successful, negative errno code on failure.
 */
int gpio_configure_array(struct gpio_ec_config *gpios, u32_t len);

/**
 * @brief Set the level for a pin.
 *
 * Wrapper interface to configure several GPIOs by receiving
 * an array containing the encoded Port/Pin, flags and direction.
 *
 * @param port_pin Encoded port/pin.
 * @param value Desired logical level.
 *
 * @retval -ENODEV error when internal device validation failed.
 * @retval 0 if successful, negative errno code on failure.
 */
int gpio_write_pin(u32_t port_pin, int value);

/**
 * @brief Read the level of a pin.
 *
 * Wrapper interface to configure several GPIOs by receiving
 * an array containing the encoded Port/Pin, flags and direction.
 *
 * @param port_pin Encoded port/pin.
 *
 * @retval 1 If pin physical level is high.
 * @retval 0 If pin physical level is low.
 * @retval -ENODEV error when internal device validation failed.
 * @retval Negative errno code on failure.
 */
int gpio_read_pin(u32_t port_pin);

/**
 * @brief Initialize a gpio_callback struct.
 *
 * This wrapper interface initializes a gpio_callback struct.
 * Note: The underlaying zephyr function is void.
 *
 * @param port_pin Encoded port/pin.
 * @param callback A valid Application's callback structure pointer.
 * @param handler A valid handler function pointer.
 *
 * @retval -ENODEV error when internal device validation failed.
 * @retval 0 if successful, negative errno code on failure.
 */
int gpio_init_callback_pin(u32_t port_pin,
			   struct gpio_callback *callback,
			   gpio_callback_handler_t handler);

/**
 * @brief Add an application callback.
 *
 * This wrapper interface adds a callback pointer to be triggered in the
 * application context.
 *
 * Note: Callbacks may be added to the device from within a callback
 * handler invocation
 *
 * @param port_pin Encoded port/pin.
 * @param callback A valid Application's callback structure pointer.
 *
 * @retval -ENODEV error when internal device validation failed.
 * @retval 0 if successful, negative errno code on failure.
 */
int gpio_add_callback_pin(u32_t port_pin,
			  struct gpio_callback *callback);

/**
 * @brief Remove an application callback.
 *
 * This wrapper interface adds a callback pointer to be triggered in the
 * application context.
 *
 * Note: It is explicitly permitted, within a callback handler, to
 * remove the registration for the callback that is running.
 *
 * @param port_pin Encoded port/pin.
 * @param callback A valid Application's callback structure pointer.
 *
 * @retval -ENODEV error when internal device validation failed.
 * @retval 0 if successful, negative errno code on failure.
 */
int gpio_remove_callback_pin(u32_t port_pin,
			     struct gpio_callback *callback);

/**
 * @brief Configure pin interrupt.
 *
 * This wrapper interface configures interrupt capabilities for a given pin.
 *
 * @param port_pin Encoded port/pin.
 * @param flags Interrupt configuration flags as defined by GPIO_INT_*.
 *
 * @retval -ENODEV error when internal device validation failed.
 * @retval 0 if successful, negative errno code on failure.
 */
int gpio_interrupt_configure_pin(u32_t port_pin, gpio_flags_t flags);

/**
 * @brief Validate if a specific gpio port was initialized.
 *
 * This function is expected to be called after gpio_init otherwise
 * it will always return -ENODEV.
 *
 * @param port_pin Encoded port/pin.
 *
 * @retval -ENODEV error when internal device validation failed.
 * @retval true  if successful, false otherwise.
 */
bool gpio_port_enabled(u32_t port_pin);


#endif /* __GPIO_DRIVER_H__*/
