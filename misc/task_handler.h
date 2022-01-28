/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __TASK_HANDLER_H__
#define __TASK_HANDLER_H__

#define EC_TASK_PRIORITY	K_PRIO_COOP(5)

#define THRML_MGMT_TASK_NAME    "THRMLMGMT"

/**
 * @brief Set names for all tasks in the app.
 *
 */
void start_all_tasks(void);

/**
 * @brief Suspends all tasks currently running in the app.
 *
 */
void suspend_all_tasks(void);

/**
 * @brief Resumes all tasks currently suspend in the app.
 *
 */
void resume_all_tasks(void);

/**
 * @brief wake specified task from sleep in the app.
 *
 */
void wake_task(const char *tagname);

#endif /* __TASK_HANDLER_H__ */
