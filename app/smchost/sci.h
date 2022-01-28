/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SCI_H__
#define __SCI_H__

/* Size of SMC to SCI host buffer */
#define SCIQ_SIZE               32

/**
 * @brief  Initialize the SCI Queue.
 */
void sci_queue_init(void);

/**
 * @brief Flushes all pending SCI notifications.
 *
 * Note: This is particular relevant in eSPI-based platforms where no SCI
 * notifications should be sent after eSPI host indicates warning about ongoing
 * system reset.
 */
void sci_queue_flush(void);

/**
 * @brief Generates an SCI.
 *
 * The SCI signal can implemented using an output pin or a eSPI virtual wire.
 */
void generate_sci(void);

/**
 * @brief Generates an SCI event if there are any pending in the SCI queue.
 *
 */
void check_sci_queue(void);

/**
 * @brief Indicates if there are any SCI pending that can be sent.
 *
 * @return true if SCI are pending and can be sent.
 */
bool sci_pending(void);

/**
 * @brief Sends any pending notifications to the operating system.
 *
 * Sends pending notifications to OS handler in response to a query command.
 */
void send_sci_events(void);

/**
 * @brief Stores data to send to the operating system in the SCI queue.
 *
 * @param code the byte to push onto queue.
 */
void enqueue_sci(uint8_t Code);

/**
 * @brief Check if system is in ACPI mode or not.
 *
 * @return 1 - system is in ACPI mode, else 0.
 */
bool is_system_in_acpi_mode(void);

#endif /* __SCI_H__ */
