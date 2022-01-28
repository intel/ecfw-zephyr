/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __PMC_H__
#define __PMC_H__

/**
 * @brief Possible requests for Power management controller.
 *
 * Note: Wake depends on system state and how SoC Host is configured.
 */
enum pmc_request {
	PMC_SYSTEM_WAKE,
	PMC_SYSTEM_GLOBAL_RESET,
	PMC_SYSTEM_WARM_START,
	PMC_SYSTEM_COLD_RESET,
	PMC_SYSTEM_SHUTDOWN,
};

/**
 * @brief Resets the SoC (host) making use of eSPI OOB
 *
 * @param reset_type system reset requested. See enum pmc_request.
 *
 * @retval -EIO General input / output error, failed to configure device.
 * @retval -ENOTSUP capability not supported.
 * @retval 0 if success.
 *
 */

int pmc_reset_soc(enum pmc_request reset_type);

#endif /* __PMC_H__ */
