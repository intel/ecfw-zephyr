/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SOC_DEBUG_H__
#define __SOC_DEBUG_H__

/**
 * @brief Performs low initialization of platform hooks to allow SoC-debug.
 *
 */
void soc_debug_init(void);

/**
 * @brief Reset SoC-debug awareness.
 *
 */
void soc_debug_reset(void);

/**
 * @brief Check alternative method to disable timeout via keyboard matrix.
 *
 */
void soc_debug_consent_kbs(void);

#endif /* __SOC_DEBUG_H__ */
