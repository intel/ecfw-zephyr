/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief APIs for system management controller
 */

#ifndef __SMCHOST_H__
#define __SMCHOST_H__

/*  SMCHOST_GET_SMC_MODE definitions */
#define SMC_CAPS_INDEX			3u
#define GEYSERVILLE_SUPPORT		7u
#define THERMAL_STATES_LOCKED		6u
#define EXTENDED_THERMAL_SENSORS	5u
#define BOOT_CONFIG_MAF			4u
#define BOOT_CONFIG_SAF			3u
#define LEGACY_SUPPORT			2u
#define PECI_ACCESS_MODE_POS		1u
#define ACPI_MODE			0u

/* PLN definitions */
#define PLN_PIN_ASSERT			0u
#define PLN_PIN_DEASSERT		1u
#define PLN_PIN_NC			2u
/* PLN duration required by ssd for reset preparation */
#define MINIMUM_PLN_TIME		2u

/* PECI bus selection */
#define LEGACY_PECI_MODE 0
#define PECI_OVER_ESPI_MODE 1

/* EC identifier */
#define SMCHOST_MAX_BUF_SIZE		10

/* Virtual Dock Status */
#define VIRTUAL_DOCK_CONNECTED 0
#define VIRTUAL_DOCK_DISCONNECTED 1

#include "smchost_extended.h"

enum hid_btn_sci {
	HID_BTN_SCI_PWR = 0,
	HID_BTN_SCI_VOL_UP,
	HID_BTN_SCI_VOL_DOWN,
	HID_BTN_SCI_HOME,
	HID_BTN_SCI_ROT_LOCK,
};


/**
 * @brief SMC host management.
 *
 * This routine handles all commands received from BIOS in ACPI mode.
 *
 * @param p1 pointer to additional task-specific data.
 * @param p2 pointer to additional task-specific data.
 * @param p2 pointer to additional task-specific data.
 */
void smchost_thread(void *p1, void *p2, void *p3);

/**
 * @brief Send data to SMC host.
 *
 * @param pdata pointer to buffer holding the data.
 * @param len the amount of bytes to be sent.
 */
void send_to_host(uint8_t *pdata, uint8_t len);

#ifdef CONFIG_SMCHOST_EVENT_DRIVEN_TASK
/**
 * @brief Indicate smchost task there is an event that requires to be processed.
 */
void smchost_signal_request(void);
#endif

/**
 * @brief get PLN pin status.
 *
 * @return PLN Pin status 0 - assert, 1- deassert, 2 - no action
 */
uint8_t get_pln_pin_sts(void);

/**
 * @brief Update PLN pin status
 *
 * @param PLN pin update 0 - assert, 1- deassert, 2 - no action
 */
void set_pln_pin_sts(uint8_t sts);

/**
 * @brief get PLT_RST signal status.
 *
 * @return PLT_RST status 0 - assert, 1 - deassert
 */
uint8_t get_pltrst_signal_sts(void);

/**
 * @brief Manage PLN pin
 *
 * This routine manages pln signal and drive it based on current state of pin.
 */
void manage_pln_signal(void);

/**
 * @brief checks whether SCI events need to be triggered
 *
 * @return 0 - SCI events need not be triggered
 * 1 - SCI events need to be triggered.
 */
uint8_t check_btn_sci_sts(uint8_t btn_sci_en_dis);

extern uint8_t host_req[SMCHOST_MAX_BUF_SIZE];
extern uint8_t host_res[SMCHOST_MAX_BUF_SIZE];
extern uint8_t host_req_len;
extern uint8_t host_res_len;
extern uint8_t host_res_idx;
extern uint8_t peci_access_mode;


#endif /* __SMCHOST_H__ */
