/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __PWRBTN_MGMT_H__
#define __PWRBTN_MGMT_H__

/**
 * @typedef pwrbtn_handler_t
 * @brief callback function for modules registered for power button events.
 * @param pwrbtn_sts current power button status.
 */
typedef void (*pwrbtn_handler_t)(uint8_t pwrbtn_sts);


/**
 * @brief Perform initialization of task that perform power button handling.
 *
 */
int pwrbtn_init(void);

/**
 * @brief Perform initialization of task that perform power button handling.
 *
 */
void pwrbtn_trigger_wake(void);

/**
 * @brief Allows to register modules interested in power button events.
 *
 */
void pwrbtn_register_handler(pwrbtn_handler_t handler);


#endif /* __PWRBTN_MGMT_H__ */
