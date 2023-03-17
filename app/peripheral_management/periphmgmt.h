/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __PERIPH_MGMT_H__
#define __PERIPH_MGMT_H__

/* GET_SWITCH_STS definitions */
#define SWITCH_STATUS_LEGACY_POS	7u
#define SWITCH_STATUS_TESTCRD_DET_POS	6u
#define SWITCH_STATUS_VIRTUAL_DOCK_POS	5u
#define SWITCH_STATUS_AC_POWER_POS	4u
#define SWITCH_STATUS_HOME_BTN_POS	3u
#define SWITCH_STATUS_NMI_POS		2u
#define SWITCH_STATUS_VIRTUAL_BATT_POS	1u
#define SWITCH_STATUS_LEGACY_LID	0u

/**
 * @brief Returns virtual battery presence status.
 *
 * @return 1 if virtual battery present otherwise 0.
 */
bool is_virtual_battery_prsnt(void);

/**
 * @brief Returns virtual dock presence status.
 *
 * @return 1 if virtual dock present otherwise 0.
 */
bool is_virtual_dock_prsnt(void);

/**
 * @brief update the  virtual bat and doc presence status.
 *
 */
void update_virtual_bat_dock_status(void);

/**
 * @brief update the switch status of virtual bat, dock and therm strap.
 *
 */
void  update_switch_status(void);

/**
 * @brief Returns the switch status.
 *
 */
uint8_t read_io_switch_status(void);

typedef void (*btn_handler_t)(uint8_t btn_sts);

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
int periph_register_button(uint32_t port_pin, btn_handler_t handler);

#endif /* __PWRBTN_MGMT_H__ */
