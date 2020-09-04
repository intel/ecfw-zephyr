/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SMCHOST_EXTENDED_H__
#define __SMCHOST_EXTENDED_H__

#ifdef CONFIG_ESPI_PERIPHERAL_HOST_IO_PVT
#include "smchost_pvt.h"
#endif

#define CONV_BYTES_TO_WORD(msb, lsb)	(((msb) << 8) | (lsb))

/**
 * @brief Handle extended SMC commands to retrieve EC information.
 *
 * @param command identifier for the operation requested.
 */
void smchost_cmd_info_handler(u8_t command);

/**
 * @brief Handle extended SMC commands for power management operations.
 *
 * @param command identifier for the operation requested.
 */
void smchost_cmd_pm_handler(u8_t command);

/**
 * @brief Handle extended SMC commands for Platform Flash Armoring Technology.
 *
 * @param command identifier for the operation requested.
 */
void smchost_cmd_pfat_handler(u8_t command);

/**
 * @brief Handle extended SMC commands for battery management.
 *
 * @param command identifier for the operation requested.
 */
void smchost_cmd_bmc_handler(u8_t command);

/**
 * @brief Handle extended SMC commands for thermal management.
 *
 * @param command identifier for the operation requested.
 */
void smchost_cmd_thermal_handler(u8_t command);

/**
 * @brief Handle power button events.
 *
 * @param pwrbtn_sts the current power button status.
 */
void smchost_pwrbtn_handler(u8_t pwrbtn_sts);

/**
 * @brief Get connected standby state.
 */
bool smchost_is_system_in_cs(void);


#endif /* __SMCHOST_EXTENDED_H__ */
