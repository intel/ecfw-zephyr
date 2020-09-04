/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __TASK_HANDLER_H__
#define __TASK_HANDLER_H__

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

#endif /* __TASK_HANDLER_H__ */
