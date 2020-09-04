#include <zephyr.h>
#include <device.h>
#include <errno.h>
#include <logging/log.h>
#include "gpio_ec.h"

LOG_MODULE_REGISTER(gpio_ec, CONFIG_GPIO_EC_LOG_LEVEL);

struct gpio_device {
	char name[Z_DEVICE_MAX_NAME_LEN];
	struct device *port;
};

static struct gpio_device ports[] = {
	{ DT_LABEL(DT_NODELABEL(gpio_000_036)), NULL},
	{ DT_LABEL(DT_NODELABEL(gpio_040_076)), NULL},
	{ DT_LABEL(DT_NODELABEL(gpio_100_136)), NULL},
	{ DT_LABEL(DT_NODELABEL(gpio_140_176)), NULL},
	{ DT_LABEL(DT_NODELABEL(gpio_200_236)), NULL},
	{ DT_LABEL(DT_NODELABEL(gpio_240_276)), NULL},
	/* Handle 1 or more IO expanders */
#ifdef CONFIG_GPIO_PCA95XX
#if DT_NODE_HAS_STATUS(DT_INST(0, nxp_pca95xx), okay)
	{ DT_LABEL(DT_INST(0, nxp_pca95xx)), NULL},
#endif
#if DT_NODE_HAS_STATUS(DT_INST(1, nxp_pca95xx), okay)
	{ DT_LABEL(DT_INST(1, nxp_pca95xx)), NULL},
#endif
#endif
};

struct gpio_port_pin {
	struct device *gpio_dev;
	gpio_pin_t pin;
};

static int validate_device(u32_t port_pin, struct gpio_port_pin *pp)
{
	u32_t port_idx;
	gpio_pin_t pin;

	port_idx = gpio_get_port(port_pin);
	pin = gpio_get_pin(port_pin);

	/* Fail gracefully when GPIO table is misconfigured which easily
	 * results in a CPU fault
	 */
	if (port_idx >= ARRAY_SIZE(ports)) {
		return -EINVAL;
	}

	if (!ports[port_idx].port) {
		LOG_ERR("Invalid gpio dev");
		return -ENODEV;
	}

	LOG_DBG("%s: port idx: %d port ptr: %p pin: %d",
		__func__, port_idx, ports[port_idx].port, pin);

	pp->gpio_dev = ports[port_idx].port;
	pp->pin = pin;

	return 0;
}

int gpio_init(void)
{
	struct device *gpio_dev;

	for (int i = 0; i < ARRAY_SIZE(ports); i++) {
		gpio_dev = device_get_binding(ports[i].name);

		if (!gpio_dev) {
			LOG_ERR("Unable to find %s", ports[i].name);
		}

		ports[i].port = gpio_dev;
		LOG_DBG("[Port %c] %p\n", (i+0x31), gpio_dev);
	}

	return 0;
}

int gpio_configure_pin(u32_t port_pin, gpio_flags_t flags)
{
	int ret;
	struct gpio_port_pin pp;

	ret = validate_device(port_pin, &pp);
	if (ret) {
		LOG_ERR("Invalid pin %d", pp.pin);
		return ret;
	}

	ret = gpio_pin_configure(pp.gpio_dev, pp.pin, flags);

	if (ret) {
		LOG_ERR("Failed to configure pin %d", pp.pin);
		return ret;
	}

	return 0;
}

int gpio_configure_array(struct gpio_ec_config *gpios, u32_t len)
{
	int ret;
	struct gpio_port_pin pp;

	/* Configure static board gpios */
	for (int i = 0; i < len; i++) {
		ret = validate_device(gpios[i].port_pin, &pp);
		if (ret) {
			return ret;
		}

		LOG_DBG("i: %d port: %p pin: %d cfg: %x", i, pp.gpio_dev,
			pp.pin,
			gpios[i].cfg);

		if (!pp.gpio_dev) {
			LOG_ERR("Invalid port at %d", i);
			continue;
		}

		ret = gpio_pin_configure(pp.gpio_dev, pp.pin,
					 gpios[i].cfg);

		if (ret) {
			LOG_ERR("Config fail: %d i:%d port:%p pin: %d cfg: %d",
				ret, i, pp.gpio_dev, pp.pin,
				gpios[i].cfg);
		}
	}

	return 0;
}

int gpio_write_pin(u32_t port_pin, int value)
{
	int ret;
	struct gpio_port_pin pp;

	ret = validate_device(port_pin, &pp);
	if (ret) {
		return ret;
	}

	return gpio_pin_set_raw(pp.gpio_dev, pp.pin, value);
}

int gpio_read_pin(u32_t port_pin)
{
	int ret;
	struct gpio_port_pin pp;

	ret = validate_device(port_pin, &pp);
	if (ret) {
		return ret;
	}

	return gpio_pin_get_raw(pp.gpio_dev, pp.pin);
}

int gpio_init_callback_pin(u32_t port_pin,
			   struct gpio_callback *callback,
			   gpio_callback_handler_t handler)
{
	int ret;
	struct gpio_port_pin pp;

	ret = validate_device(port_pin, &pp);
	if (ret) {
		return ret;
	}

	/* zephyr function is void */
	gpio_init_callback(callback, handler, BIT(pp.pin));

	return 0;
}

int gpio_add_callback_pin(u32_t port_pin,
			  struct gpio_callback *callback)
{
	int ret;
	struct gpio_port_pin pp;

	ret = validate_device(port_pin, &pp);
	if (ret) {
		return ret;
	}

	return gpio_add_callback(pp.gpio_dev, callback);
}

int gpio_remove_callback_pin(u32_t port_pin,
			     struct gpio_callback *callback)
{
	int ret;
	struct gpio_port_pin pp;

	ret = validate_device(port_pin, &pp);
	if (ret) {
		return ret;
	}

	return gpio_remove_callback(pp.gpio_dev, callback);
}

int gpio_interrupt_configure_pin(u32_t port_pin, gpio_flags_t flags)
{
	int ret;
	struct gpio_port_pin pp;

	ret = validate_device(port_pin, &pp);
	if (ret) {
		return ret;
	}

	return gpio_pin_interrupt_configure(pp.gpio_dev, pp.pin, flags);
}

bool gpio_port_enabled(u32_t port_pin)
{
	int ret;
	struct gpio_port_pin pp;

	ret = validate_device(port_pin, &pp);

	return ret == 0;
}
