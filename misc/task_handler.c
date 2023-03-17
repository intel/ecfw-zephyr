/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include "pwrplane.h"
#include "espioob_mngr.h"
#include "postcodemgmt.h"
#include "smchost.h"
#include "periphmgmt.h"
#include "kbchost.h"
#include "task_handler.h"
#ifdef CONFIG_THERMAL_MANAGEMENT
#include "thermalmgmt.h"
#endif

LOG_MODULE_DECLARE(pwrmgmt, CONFIG_PWRMGT_LOG_LEVEL);

#define EC_TASK_STACK_SIZE	1024

/* K_FOREVER can no longer be used with K_THREAD_DEFINE
 * Zephyr community is using user-defined macros instead to achieve
 * the same behavior than with K_FOREVER.
 * For EC define macro until the new flag K_THREAD_NO_START is available.
 */
#define EC_WAIT_FOREVER (-1)

const uint32_t periph_thrd_period = 1;
const uint32_t pwrseq_thrd_period = 10;
const uint32_t smchost_thrd_period = 10;

#if defined(CONFIG_ESPI_PERIPHERAL_8042_KBC) && \
	(defined(CONFIG_PS2_KEYBOARD) || defined(CONFIG_PS2_MOUSE) || \
	defined(CONFIG_KSCAN_EC))

#define KBC_TASK_STACK_SIZE	500
#define KB_TASK_STACK_SIZE	384
K_THREAD_DEFINE(kbc_thrd_id, KBC_TASK_STACK_SIZE, to_from_host_thread,
		NULL, NULL, NULL, EC_TASK_PRIORITY, 0, EC_WAIT_FOREVER);
K_THREAD_DEFINE(kb_thrd_id, KB_TASK_STACK_SIZE, to_host_kb_thread,
		NULL, NULL, NULL, EC_TASK_PRIORITY, 0, EC_WAIT_FOREVER);
#endif

#ifdef CONFIG_POSTCODE_MANAGEMENT
const uint32_t postcode_thrd_period = 125;
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

#define OOBMNGR_TASK_STACK_SIZE		512U
K_THREAD_DEFINE(oobmngr_thrd_id, OOBMNGR_TASK_STACK_SIZE, oobmngr_thread,
		NULL, NULL, NULL, EC_TASK_PRIORITY,
		K_INHERIT_PERMS, EC_WAIT_FOREVER);

K_THREAD_DEFINE(smchost_thrd_id, EC_TASK_STACK_SIZE, smchost_thread,
		&smchost_thrd_period, NULL, NULL, EC_TASK_PRIORITY,
		K_INHERIT_PERMS, EC_WAIT_FOREVER);

#ifdef CONFIG_THERMAL_MANAGEMENT
const uint32_t thermal_thrd_period = 250;
K_THREAD_DEFINE(thermal_thrd_id, EC_TASK_STACK_SIZE, thermalmgmt_thread,
		&thermal_thrd_period, NULL, NULL, EC_TASK_PRIORITY,
		K_INHERIT_PERMS, EC_WAIT_FOREVER);
#endif




struct task_info {
	k_tid_t thread_id;
	bool can_suspend;
	const char *tagname;
};

static struct task_info tasks[] = {

#if defined(CONFIG_ESPI_PERIPHERAL_8042_KBC) && \
	(defined(CONFIG_PS2_KEYBOARD) || defined(CONFIG_PS2_MOUSE) || \
	defined(CONFIG_KSCAN_EC))

	{ .thread_id = kbc_thrd_id, .can_suspend = false,
	  .tagname = "KBC" },

	{ .thread_id = kb_thrd_id, .can_suspend = false,
	  .tagname = "KB" },
#endif

#ifdef CONFIG_POSTCODE_MANAGEMENT
	{ .thread_id = postcode_thrd_id, .can_suspend = false,
	  .tagname = "POST" },
#endif

	{ .thread_id = periph_thrd_id, .can_suspend = false,
	  .tagname = "PERIPH" },

	{ .thread_id = pwrseq_thrd_id, .can_suspend = true,
	  .tagname = "PWR" },

	{ .thread_id = oobmngr_thrd_id, .can_suspend = false,
	  .tagname = "OOB" },

	{ .thread_id = smchost_thrd_id, .can_suspend = false,
	  .tagname = "SMC" },

#ifdef CONFIG_THERMAL_MANAGEMENT
	{ .thread_id = thermal_thrd_id, .can_suspend = false,
	  .tagname = THRML_MGMT_TASK_NAME },
#endif
};

void start_all_tasks(void)
{
	for (int i = 0; i < ARRAY_SIZE(tasks); i++) {
		if (tasks[i].thread_id) {
#ifdef CONFIG_THREAD_NAME
			k_thread_name_set(tasks[i].thread_id, tasks[i].tagname);
			LOG_DBG("%s %s", __func__, tasks[i].tagname);
#endif
			k_thread_start(tasks[i].thread_id);
		}
	}
}

void suspend_all_tasks(void)
{
	for (int i = 0; i < ARRAY_SIZE(tasks); i++) {
		if (tasks[i].can_suspend) {
			k_thread_suspend(tasks[i].thread_id);
			LOG_INF("%p suspended", tasks[i].thread_id);
		}
	}
}

void resume_all_tasks(void)
{
	for (int i = 0; i < ARRAY_SIZE(tasks); i++) {
		if (tasks[i].can_suspend) {
			k_thread_resume(tasks[i].thread_id);
			LOG_INF("%p resumed", tasks[i].thread_id);
		}
	}
}

void wake_task(const char *tagname)
{
	for (int i = 0; i < ARRAY_SIZE(tasks); i++) {
		if (strcmp(tasks[i].tagname, tagname) == 0) {
			k_wakeup(tasks[i].thread_id);
			break;
		}
	}
}
