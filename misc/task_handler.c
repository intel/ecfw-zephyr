/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <kernel.h>
#include <zephyr.h>
#include <device.h>
#include <logging/log.h>
#include "pwrplane.h"
#include "postcodemgmt.h"
#include "smchost.h"
#include "periphmgmt.h"
#include "kbchost.h"
#ifdef CONFIG_PWRMGMT_DEEP_IDLE
#include "pwrmgmt.h"
#endif
#ifdef CONFIG_THERMAL_MANAGEMENT
#include "thermalmgmt.h"
#endif
#ifdef CONFIG_BATTERY_MANAGEMENT
#include "bmc.h"
#endif
#ifdef CONFIG_USBC_POWER_DELIVERY
#include "usbc.h"
#endif

LOG_MODULE_DECLARE(pwrmgmt, CONFIG_PWRMGT_LOG_LEVEL);

#define ESSENTIAL_EC_TASKS	3

#define NUM_EC_TASKS		(ESSENTIAL_EC_TASKS +			\
				 CONFIG_POSTCODE_TASK +			\
				 CONFIG_PWRMGMT_DEEP_IDLE_TASK +	\
				 CONFIG_THERMAL_MANAGEMENT_TASK +	\
				 CONFIG_BATTERY_MANAGEMENT_TASK +	\
				 CONFIG_USBC_PD_TASK +			\
				 CONFIG_8042_IBF_KEYBOARD_TASKS)

#define EC_TASK_PRIORITY	K_PRIO_COOP(5)

#define EC_TASK_STACK_SIZE	1024
#define KBC_TASK_STACK_SIZE	320
#define KB_TASK_STACK_SIZE	384

/* K_FOREVER can no longer be used with K_THREAD_DEFINE
 * Zephyr community is using user-defined macros instead to achieve
 * the same behavior than with K_FOREVER.
 * For EC define macro until the new flag K_THREAD_NO_START is available.
 */
#define EC_WAIT_FOREVER (-1)

const u32_t postcode_thrd_period = 125;
const u32_t periph_thrd_period = 1;
const u32_t pwrseq_thrd_period = 10;
const u32_t smchost_thrd_period = 10;
const u32_t pwrmgmt_thrd_period = 100;
const u32_t thermal_thrd_period = 100;
const u32_t battery_thrd_period = 1000;
const u32_t usbc_thrd_period = 5;

#if defined(CONFIG_ESPI_PERIPHERAL_8042_KBC) && \
	defined(CONFIG_PS2_KEYBOARD_AND_MOUSE) || defined(CONFIG_KSCAN_EC)

K_THREAD_DEFINE(kbc_thrd_id, KBC_TASK_STACK_SIZE, to_from_host_thread,
		NULL, NULL, NULL, EC_TASK_PRIORITY, 0, EC_WAIT_FOREVER);
K_THREAD_DEFINE(kb_thrd_id, KB_TASK_STACK_SIZE, to_host_kb_thread,
		NULL, NULL, NULL, EC_TASK_PRIORITY, 0, EC_WAIT_FOREVER);
#endif

#ifdef CONFIG_POSTCODE_MANAGEMENT
K_THREAD_DEFINE(postcode_thrd_id, EC_TASK_STACK_SIZE, postcode_thread,
		&postcode_thrd_period, NULL, NULL, EC_TASK_PRIORITY,
		K_INHERIT_PERMS, EC_WAIT_FOREVER);
#endif

K_THREAD_DEFINE(periph_thrd_id, EC_TASK_STACK_SIZE, periph_thread,
		&periph_thrd_period, NULL, NULL, EC_TASK_PRIORITY,
		K_INHERIT_PERMS, EC_WAIT_FOREVER);

K_THREAD_DEFINE(pwrseq_thrd_id, EC_TASK_STACK_SIZE, pwrseq_thread,
		&pwrseq_thrd_period, NULL, NULL, EC_TASK_PRIORITY,
		K_INHERIT_PERMS, EC_WAIT_FOREVER);

K_THREAD_DEFINE(smchost_thrd_id, EC_TASK_STACK_SIZE, smchost_thread,
		&smchost_thrd_period, NULL, NULL, EC_TASK_PRIORITY,
		K_INHERIT_PERMS, EC_WAIT_FOREVER);

#ifdef CONFIG_PWRMGMT_DEEP_IDLE
K_THREAD_DEFINE(pwrmgmt_thrd_id, EC_TASK_STACK_SIZE, pwrmgmt_thread,
		&pwrmgmt_thrd_period, NULL, NULL, EC_TASK_PRIORITY,
		K_INHERIT_PERMS, EC_WAIT_FOREVER);
#endif

#ifdef CONFIG_THERMAL_MANAGEMENT
K_THREAD_DEFINE(thermal_thrd_id, EC_TASK_STACK_SIZE, thermalmgmt_thread,
		&thermal_thrd_period, NULL, NULL, EC_TASK_PRIORITY,
		K_INHERIT_PERMS, EC_WAIT_FOREVER);
#endif

#ifdef CONFIG_BATTERY_MANAGEMENT
K_THREAD_DEFINE(battery_thrd_id, EC_TASK_STACK_SIZE, batterymgmt_thread,
		&battery_thrd_period, NULL, NULL, EC_TASK_PRIORITY,
		K_INHERIT_PERMS, EC_WAIT_FOREVER);
#endif

#ifdef CONFIG_USBC_POWER_DELIVERY
K_THREAD_DEFINE(usbc_thrd_id, EC_TASK_STACK_SIZE, usbc_thread,
		&usbc_thrd_period, NULL, NULL, EC_TASK_PRIORITY,
		K_INHERIT_PERMS, EC_WAIT_FOREVER);
#endif

struct task_info {
	k_tid_t thread_id;
	bool can_suspend;
	const char *tagname;
};

static struct task_info tasks[NUM_EC_TASKS] = {

#if defined(CONFIG_ESPI_PERIPHERAL_8042_KBC) && \
	defined(CONFIG_PS2_KEYBOARD_AND_MOUSE) || defined(CONFIG_KSCAN_EC)

	{ .thread_id = kbc_thrd_id, .can_suspend = true,
	  .tagname = "KBC" },

	{ .thread_id = kb_thrd_id, .can_suspend = true,
	  .tagname = "KB" },
#endif

#ifdef CONFIG_POSTCODE_MANAGEMENT
	{ .thread_id = postcode_thrd_id, .can_suspend = true,
	  .tagname = "POST" },
#endif

	{ .thread_id = periph_thrd_id, .can_suspend = true,
	  .tagname = "PERIPH" },

	{ .thread_id = pwrseq_thrd_id, .can_suspend = true,
	  .tagname = "PWR" },

	{ .thread_id = smchost_thrd_id, .can_suspend = true,
	  .tagname = "SMC" },

#ifdef CONFIG_PWRMGMT_DEEP_IDLE
	{ .thread_id = pwrmgmt_thrd_id, .can_suspend = false,
	  .tagname = "PWRMGMT" },
#endif

#ifdef CONFIG_THERMAL_MANAGEMENT
	{ .thread_id = thermal_thrd_id, .can_suspend = false,
	  .tagname = "THRMLMGMT" },
#endif

#ifdef CONFIG_BATTERY_MANAGEMENT
	{ .thread_id = battery_thrd_id, .can_suspend = false,
	  .tagname = "BATTMGMT" },
#endif

#ifdef CONFIG_USBC_POWER_DELIVERY
	{ .thread_id = usbc_thrd_id, .can_suspend = false,
	  .tagname = "USBC" },
#endif
};

void start_all_tasks(void)
{
	for (int i = 0; i < NUM_EC_TASKS; i++) {
		if (tasks[i].thread_id) {
#ifdef CONFIG_THREAD_NAME
			k_thread_name_set(tasks[i].thread_id, tasks[i].tagname);
#endif
			k_thread_start(tasks[i].thread_id);
		}
	}
}

void suspend_all_tasks(void)
{
	for (int i = 0; i < NUM_EC_TASKS; i++) {
		if (tasks[i].can_suspend) {
			k_thread_suspend(tasks[i].thread_id);
			/* TODO: Remove this when issue #20033 is addressed */
			k_sleep(K_MSEC(100));
			LOG_INF("%p suspended", tasks[i].thread_id);
		}
	}
}

void resume_all_tasks(void)
{
	for (int i = 0; i < NUM_EC_TASKS; i++) {
		if (tasks[i].can_suspend) {
			k_thread_resume(tasks[i].thread_id);
			/* TODO: Remove this when issue #20033 is addressed */
			k_sleep(K_MSEC(100));
		}
	}
}
