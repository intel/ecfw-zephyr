/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DNX_EC_ASSISTED_TRIGGER_H__
#define __DNX_EC_ASSISTED_TRIGGER_H__

/**
 * @brief Initialize any system hooks required to detect early user activity.
 *
 */
void dnx_ec_assisted_init(void);

/**
 * @brief Trigger DnX entry using Intel SoC DnX strap.
 *
 */
void dnx_soc_handshake(void);

/**
 * @brief Perform system restart.
 *
 * Note: Requires Intel PMC OOB support.
 */
void dnx_ec_assisted_restart(void);

/**
 * @brief Perform DnX-assisted entry management during power sequencing.
 *
 */
void dnx_ec_assisted_manage(void);

#endif /* __DNX_EC_ASSISTED_TRIGGER_H__ */
