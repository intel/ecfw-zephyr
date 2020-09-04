/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __PERIPH_MGMT_H__
#define __PERIPH_MGMT_H__

typedef void (*btn_handler_t)(u8_t btn_sts);

/**
 * @brief Handles the power button and determine the next power state.
 *
 * This routines doesn't perform debouncing but tracks short and long press
 * cases. It also sends notifications to Host.
 *
 * If the power button is down for the de-bounce period then set
 * flags to transition to a new power state. If it stays down
 * for 4 seconds power off the system.
 *
 * @param p1 pointer to additional task-specific data.
 * @param p2 pointer to additional task-specific data.
 * @param p2 pointer to additional task-specific data.
 *
 */
void periph_thread(void *p1, void *p2, void *p3);

/**
 * @brief Allow to register for button press/released events.
 *
 * @param port_id the gpio port identifier.
 * @param gpio_port the gpio port handle.
 * @param btn_pin the pin number.
 * @param btn_pin the function to be called when there is a button event.
 */
int periph_register_button(u32_t port_pin, btn_handler_t handler);

#endif /* __PWRBTN_MGMT_H__ */
