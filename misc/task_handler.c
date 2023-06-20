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
#include <zephyr/app_memory/app_memdomain.h>
#include "pwrplane.h"
#include "espioob_mngr.h"
#include "postcodemgmt.h"
#include "smchost.h"
#include "periphmgmt.h"
#include "kbchost.h"
#include "task_handler.h"
#include "board_config.h"
#include "i2c_hub.h"
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

#ifdef CONFIG_USERSPACE
K_APPMEM_PARTITION_DEFINE(ecfw_partition);
K_APPMEM_PARTITION_DEFINE(ecfw_kbc_partition);
K_APPMEM_PARTITION_DEFINE(ecfw_pwrseq_partition);
static struct k_mem_domain ecfw_domain;

/* TODO: Decide if can use this for all kernel objects */
static const struct device *const post_devices[] = {
	DEVICE_DT_GET(I2C_BUS_0),
};

#define DT_DRV_COMPAT   nxp_pca95xx

static const struct device *const pwrseq_devices[] = {
	DEVICE_DT_GET(I2C_BUS_0),
	DEVICE_DT_GET(ESPI_0),
	/*  TODO: Re-structure kbc_enable_interface so kbchost perform the operations */
#if defined(CONFIG_PS2_MOUSE)
	DEVICE_DT_GET(PS2_MOUSE),
#endif
#if defined(CONFIG_PS2_KEYBOARD)
	DEVICE_DT_GET(PS2_KEYBOARD),
#endif
#if defined(CONFIG_KSCAN_EC)
	DEVICE_DT_GET(KSCAN_MATRIX),
#endif
	DEVICE_DT_GET(DT_NODELABEL(gpio_000_036)),
	DEVICE_DT_GET(DT_NODELABEL(gpio_040_076)),
	DEVICE_DT_GET(DT_NODELABEL(gpio_100_136)),
	DEVICE_DT_GET(DT_NODELABEL(gpio_140_176)),
	DEVICE_DT_GET(DT_NODELABEL(gpio_200_236)),
	DEVICE_DT_GET(DT_NODELABEL(gpio_240_276)),
#if DT_NODE_HAS_STATUS(DT_DRV_INST(0), okay)
	DEVICE_DT_INST_GET(0),
#endif
#if DT_NODE_HAS_STATUS(DT_DRV_INST(1), okay)
	DEVICE_DT_INST_GET(1),
#endif
};

/* TODO: Find if possible to to avoid extern here towards variable defined in postcodemgmt.c */
extern struct k_sem update_lock;

#endif

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
static uint32_t postcode_thrd_period = 125;

K_THREAD_DEFINE(postcode_thrd_id, EC_TASK_STACK_SIZE, postcode_thread,
		&postcode_thrd_period, NULL, NULL, EC_TASK_PRIORITY,
		K_USER | K_INHERIT_PERMS, EC_WAIT_FOREVER);
K_THREAD_ACCESS_GRANT(postcode_thrd_id, &update_lock);

/* TODO: Check this with FMOS team, giving static access to device driver does not build */
/* K_THREAD_ACCESS_GRANT(postcode_thrd_id, i2c0_device); */
/* K_THREAD_ACCESS_GRANT(postcode_thrd_id, DEVICE_DT_GET(I2C_BUS_0)); */
/* K_THREAD_ACCESS_GRANT(postcode_thrd_id, &dummy_sem, &update_lock, i2c0_device); */
#endif

K_THREAD_DEFINE(periph_thrd_id, EC_TASK_STACK_SIZE, periph_thread,
		&periph_thrd_period, NULL, NULL, EC_TASK_PRIORITY,
		K_INHERIT_PERMS, EC_WAIT_FOREVER);

K_THREAD_DEFINE(pwrseq_thrd_id, EC_TASK_STACK_SIZE, pwrseq_thread,
		&pwrseq_thrd_period, NULL, NULL, EC_TASK_PRIORITY,
		K_USER | K_INHERIT_PERMS, EC_WAIT_FOREVER);
/* TODO: Restructure postcode mgmt API so no need to grant access */
K_THREAD_ACCESS_GRANT(pwrseq_thrd_id, &update_lock);

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

#ifdef CONFIG_USERSPACE
void init_tasks_memory_domain(void)
{
	int ret;
	struct k_mem_partition *ecfw_parts[] = {
		&ecfw_partition, &ecfw_kbc_partition, &ecfw_pwrseq_partition
	};

	/* Initialize a memory domain with the specified partitions
	 * and add ourself to this domain
	 */
	ret = k_mem_domain_init(&ecfw_domain, ARRAY_SIZE(ecfw_parts), ecfw_parts);
	if (ret) {
		LOG_WRN("mem domain init failed %d", ret);
	}

	/* There is a default domain k_mem_domain_default which will be assigned
	 * to threads if they have not been specifically assigned to a domain,
	 * or inherited a memory domain membership from their parent thread.
	 * The main thread starts as a member of the default domain.
	 */
	k_mem_domain_add_thread(&ecfw_domain, k_current_get());
}
#endif


void grant_thread_device_access(uint8_t tid, const void *devices, int n)
{
	k_mem_domain_add_thread(&ecfw_domain, tasks[tid].thread_id);

	/* TODO: See if we can avoid dynamic object access grant and use
	 * static macros.
	 */
	for (int d_idx = 0; d_idx < n; d_idx++) {
		//LOG_WRN("Grant %s access to %p", tasks[i].tagname, devices[d_idx]);

		/* TODO: Decide if grant access individually or via hub APIs */
		//k_object_access_grant(devices[d_idx], tasks[index].thread_id);
	}

}

void start_all_tasks(void)
{
	for (int i = 0; i < ARRAY_SIZE(tasks); i++) {
		if (tasks[i].thread_id) {

#ifdef CONFIG_THREAD_NAME
			k_thread_name_set(tasks[i].thread_id, tasks[i].tagname);
			LOG_DBG("Set thread name %s", tasks[i].tagname);
#endif
			LOG_DBG("Start thread %s", tasks[i].tagname);


#ifdef CONFIG_USERSPACE
			if (strcmp(tasks[i].tagname, "POST") == 0) {
				//grant_thread_device_access(i, post_devices, ARRAY_SIZE(post_devices));
				k_mem_domain_add_thread(&ecfw_domain, tasks[i].thread_id);
				i2c_hub_allow_access(0, tasks[i].thread_id);
				k_object_access_grant(post_devices[0], tasks[i].thread_id);
			} else if (strcmp(tasks[i].tagname, "PWR") == 0) {
				//grant_thread_device_access(i, pwrseq_devices, ARRAY_SIZE(pwrseq_devices));
				k_mem_domain_add_thread(&ecfw_domain, tasks[i].thread_id);
				i2c_hub_allow_access(0, tasks[i].thread_id);
				k_object_access_grant(pwrseq_devices[0], tasks[i].thread_id);
				k_object_access_grant(pwrseq_devices[1], tasks[i].thread_id);
				k_object_access_grant(pwrseq_devices[2], tasks[i].thread_id);
				k_object_access_grant(pwrseq_devices[3], tasks[i].thread_id);
				k_object_access_grant(pwrseq_devices[4], tasks[i].thread_id);
				k_object_access_grant(pwrseq_devices[5], tasks[i].thread_id);
				k_object_access_grant(pwrseq_devices[6], tasks[i].thread_id);
				k_object_access_grant(pwrseq_devices[7], tasks[i].thread_id);
				k_object_access_grant(pwrseq_devices[8], tasks[i].thread_id);
				k_object_access_grant(pwrseq_devices[9], tasks[i].thread_id);
				k_object_access_grant(pwrseq_devices[10], tasks[i].thread_id);
#if DT_NODE_HAS_STATUS(DT_DRV_INST(1), okay)
				k_object_access_grant(pwrseq_devices[11], tasks[i].thread_id);
#endif
#if DT_NODE_HAS_STATUS(DT_DRV_INST(1), okay)
				k_object_access_grant(pwrseq_devices[12], tasks[i].thread_id);
#endif
			}
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
