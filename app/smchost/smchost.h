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

/* SMC host definitions */
#define SMC_CAPS_INDEX            3u

/* Geyserville supported */
#define GEYSERVILLE_SUPPORT       7u
#define THERMAL_STATES_LOCKED     6u
/* Thermal sensor status */
#define EXTENDED_THERMAL_SENSORS  5u
#define BOOT_CONFIG_MAF           4u
#define BOOT_CONFIG_SAF           3u
#define LEGACY_SUPPORT            2u
#define ACPI_MODE                 0u

#define SMCHOST_MAX_BUF_SIZE        10

#include "smchost_extended.h"

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
void send_to_host(u8_t *pdata, u8_t len);

extern u8_t host_req[SMCHOST_MAX_BUF_SIZE];
extern u8_t host_res[SMCHOST_MAX_BUF_SIZE];
extern u8_t host_req_len;
extern u8_t host_res_len;
extern u8_t host_res_idx;

#endif /* __SMCHOST_H__ */
