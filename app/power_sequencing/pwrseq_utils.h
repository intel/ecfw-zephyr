/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __RESET_H__
#define __RESET_H__

/**
 * @brief Initiate the EC reset
 *
 * Note: This directly performs power sequencing handling required prior
 * to initiate the EC HW reset.
 */
void ec_reset(void);

/**
 * @brief Override EC timeout mechanism.
 *
 */
void disable_ec_timeout(void);

/**
 * @brief Perform evaluation of EC timeout HW strap.
 *
 */
void ec_evaluate_timeout(void);

/**
 * @brief Indicate if EC timeout mechanism (WDT) is enabled.
 *
 */
bool ec_timeout_status(void);


#endif /* __RESET_H__ */
