/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SAF_CONFIG_H__
#define __SAF_CONFIG_H__

/**
 * @brief Performs eSPI controller initialization.
 *
 * Note: This operation should complete before eSPI flash channel negotiation
 * is started to avoid cases where eSPI master attempts flash operations
 * before SAF block is ready.
 *
 * It's recomended to perform this operation before RSMRST is de-asserted.
 *
 * @retval -EIO General input / output error, failed to configure device.
 * @retval -EINVAL invalid capabilities, failed to configure device.
 * @retval -ENOTSUP capability not supported by eSPI slave.
 * @retval 0 if success.
 */

int initialize_saf_bridge(void);

#endif /* __SAF_CONFIG_H__ */
