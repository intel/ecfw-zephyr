/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief eSPI hub APIs.
 *
 */

#ifndef __ESPI_HUB_H__
#define __ESPI_HUB_H__

#include <drivers/espi.h>
#include "system.h"

#define ESPI_WAIT_TRUE            1u

/* Minimum and maximum eSPI frequencies */
#define MIN_ESPI_FREQ             20u
#define MAX_ESPI_FREQ             66u

#define MAX_WARN_HANDLERS         3u
#define MAX_ACPI_HANDLERS         2u
#define MAX_PERIPH_HANDLERS       2u

/* TODO: Check if we can replace these macros */
#define KBC_IBF_DATA(x)           (((x) >> E8042_ISR_DATA_POS) & 0xFFU)
#define KBC_CMD_DATA(x)           ((x) & 0xFU)

/* TODO: Replace these macros with Zephyr byte order */
#define ESPI_PERIPHERAL_TYPE(x)   ((x) & 0x0000FFFF)
#define ESPI_PERIPHERAL_INDEX(x)  (((x) & 0xFFFF0000) >> 16)

typedef void (*espi_acpi_handler_t)(void);
typedef void (*espi_warn_handler_t)(u8_t status);
typedef void (*espi_state_handler_t)(u32_t signal, u32_t status);
typedef void (*espi_kbc_handler_t)(u8_t data, u8_t status);

enum espihub_vw_level {
	ESPIHUB_VW_LOW,
	ESPIHUB_VW_HIGH,
};

enum espihub_handler {
	ESPIHUB_RESET_WARNING,
	ESPIHUB_PLATFORM_RESET,
	ESPIHUB_SUSPEND_WARNING,
};

enum espihub_acpi_handler {
	ESPIHUB_ACPI_PUBLIC,
	ESPIHUB_ACPI_PRIVATE_0,
	ESPIHUB_ACPI_PRIVATE_1,
	ESPIHUB_ACPI_PRIVATE_2,
};

/**
 * @brief Helds context for all espi operations.
 *
 */
struct espihub_context {
	struct espi_callback espi_bus_cb;
	struct espi_callback vw_rdy_cb;
	struct espi_callback vw_cb;
	struct espi_callback p80_cb;
#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC
	struct espi_callback kbc_cb;
#endif
	u8_t espi_rst_sts;
	bool host_vw_ready;

	/* SPI flash sharing config detection */
	enum boot_config_mode spi_boot_mode;
	bool boot_config_detected;
};

/**
 * @brief Perform eSPI controllower initialization.
 *
 * @retval -EIO General input / output error, failed to configure device.
 * @retval -EINVAL invalid capabilities, failed to configure device.
 * @retval -ENOTSUP capability not supported by eSPI slave.
 * @retval 0 if success.
 */
int espihub_init(void);

/**
 * @brief Return boot configuration mode.
 *
 * @retval see enum boot_config_mode.
 */
enum boot_config_mode espihub_boot_mode(void);

/**
 * @brief Return current status of eSPI reset.
 *
 * @retval true if eSPI reset is de-asserted, false otherwise.
 */
bool espihub_reset_status(void);

/**
 * @brief Poll eSPI reset status until is asserted/de-asserted.
 *
 * @param exp_sts the expected status.
 * @param timeout value expressed in multiple of 100us.
 *
 * @retval -ETIMEDOUT or 0 if success.
 */
int wait_for_espi_reset(u8_t exp_sts, u16_t timeout);

/**
 * @brief Poll eSPI virtual wire until asserted/de-asserted.
 *
 * @param signal the virtual wire to monitor.
 * @param timeout value expressed in multiple of 100us.
 * @param exp_level the expected level of the virtual wire.
 * @param ack_required indicate if acknowledge when received.
 *
 * @retval -ETIMEDOUT or success.
 */
int wait_for_vwire(enum espi_vwire_signal signal, u16_t timeout,
		   enum espihub_vw_level exp_level, bool ack_required);

/**
 * @brief Poll signal while monitoring eSPI virtual wire.
 *
 * Note: This is used to detect glitches or when VW indicate abort.
 *
 * @param port_pin a EC GPIO. See @ec_gpio.h.
 * @param exp_sts the expected pin value.
 * @param timeout value expressed in multiple of 100us.
 * @param signal the virtual wire to monitor.
 * @param abort_sts the virtual wire status that indicates to abort the wait.
 *
 * @retval -ETIMEDOUT or 0 if success.
 */
int wait_for_pin_monitor_vwire(u32_t port_pin, u32_t exp_sts, u16_t timeout,
			       enum espi_vwire_signal signal,
			       enum espihub_vw_level abort_sts);

/**
 * @brief Add a system state handler.
 * States are tracked via eSPI host virtual wire notifications for each
 * system transition.
 *
 * @param handler module handler called when system state is received.
 *
 * @retval -EINVAL if a handler already registered or 0 if success.
 */
int espihub_add_state_handler(espi_state_handler_t handler);

/**
 * @brief Detect SPI configuration boot mode.
 *
 * Uses eSPI flash channel status to distinguish between MAFS and SAF/G3
 * If channel is ready, this will indicate ROM bootloader booted in MAFS mode
 * and that eSPI channels negotiation was already performed.
 *
 * To distinguish SAF/G3 when supported HW strap is used.
 */
void detect_boot_mode(void);

/**
 * @brief Add a host warning handler.
 *
 * Host can send different warnings about system transitions or capabilities
 * temporarily unavailable.
 *
 * @param handler_type the type of the handler been registered.
 * @param handler module handler for host warnings received via eSPI bus.
 *
 * @retval -EINVAL if a handler already registered or 0 if success.
 */
int espihub_add_warn_handler(enum espihub_handler handler_type,
			     espi_warn_handler_t handler);

/**
 * @brief Add a host warning handler.
 *
 * Host can send different warnings about system transitions or capabilities
 * temporarily unavailable.
 *
 * @param handler_type the type of the handler been registered.
 * @param handler module handler for host warnings received via eSPI bus.
 *
 * @retval -EINVAL if a handler already registered or 0 if success.
 */
int espihub_add_acpi_handler(enum espihub_acpi_handler type,
			     espi_acpi_handler_t handler);

/**
 * @brief Add a keyboard subsystem handler.
 *
 * Host can send different traffic over designated ports for keyboards.
 *
 * @param handler module handler for host warnings received via eSPI bus.
 *
 * @retval -EINVAL if a handler already registered or  0 if success.
 */
int espihub_add_kbc_handler(espi_kbc_handler_t handler);

/**
 * @brief Retrieve a virtual wire ensuring the eSPI channel is ready.
 *
 * @param signal the virtual wire to monitor.
 * @param level the current value of the virtual wire.
 *
 * @retval -EINVAL if eSPI channel is not ready or 0 if success.
 */
int espihub_retrieve_vw(enum espi_vwire_signal signal,
			enum espihub_vw_level *level);

/**
 * @brief Send a virtual wire ensuring the eSPI channel is ready.
 *
 * @param signal the virtual wire to monitor.
 * @param level the desired value of the virtual wire.
 *
 * @retval -EINVAL if eSPI channel is not ready or 0 if success.
 */
int espihub_send_vw(enum espi_vwire_signal signal,
		    enum espihub_vw_level level);

/**
 * @brief Retrieve an OOB packet ensuring the eSPI OOB channel is ready.
 *
 * @param pckt representation of OOB response.
 *
 * @retval -EINVAL if eSPI channel is not ready or 0 if success.
 */
int espihub_retrieve_oob(struct espi_oob_packet *pckt);

/**
 * @brief Send an OOB packet ensuring the eSPI OOB channel is ready.
 *
 * @param pckt representation of OOB request.
 *
 * @retval -EINVAL if eSPI channel is not ready or 0 if success.
 */
int espihub_send_oob(struct espi_oob_packet *pckt);

/**
 * @brief Perform keyboard write operation over eSPI.
 *
 * @param cmd to be sent to keyboard over the bus.
 * @param payload for the command for the keyboard.
 *
 * @retval 0 if success, error otherwise.
 */
int espihub_kbc_write(enum lpc_peripheral_opcode cmd, u32_t payload);

/**
 * @brief Perform keyboard write operation over eSPI.
 *
 * @param cmd to be sent to keyboard over the bus.
 * @param data returned by the keyboard.
 *
 * @retval 0 if success, error otherwise.
 */
int espihub_kbc_read(enum lpc_peripheral_opcode cmd, u32_t *data);

#endif /* __ESPI_HUB_H__ */
