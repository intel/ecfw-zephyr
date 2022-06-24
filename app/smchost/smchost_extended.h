/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SMCHOST_EXTENDED_H__
#define __SMCHOST_EXTENDED_H__


/**
 * @brief Handle extended SMC commands to retrieve EC information.
 *
 * @param command identifier for the operation requested.
 */
void smchost_cmd_info_handler(uint8_t command);

/**
 * @brief Handle extended SMC commands for power management operations.
 *
 * @param command identifier for the operation requested.
 */
void smchost_cmd_pm_handler(uint8_t command);

#ifdef CONFIG_THERMAL_MANAGEMENT
/**
 * @brief Handle extended SMC commands for thermal management.
 *
 * @param command identifier for the operation requested.
 */
void smchost_cmd_thermal_handler(uint8_t command);
#endif

/**
 * @brief Handle power button events.
 *
 * @param pwrbtn_sts the current power button status.
 */
void smchost_pwrbtn_handler(uint8_t pwrbtn_sts);

/**
 * @brief Get connected standby state.
 */
bool smchost_is_system_in_cs(void);

/**
 * @brief Handle Power button press Power Loss Notification (PLN) events to SSD
 *
 * @param pwrbtn_sts the current power button status.
 */
void smchost_pwrbtn_pln_handler(uint8_t pwrbtn_sts);

/**
 * @brief platform reset deassert notification for pln.
 */
void smchost_pln_pltreset_handler(void);


#endif /* __SMCHOST_EXTENDED_H__ */
