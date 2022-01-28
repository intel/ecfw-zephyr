/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DNX_H__
#define __DNX_H__

/** @brief Handle DnX host status early on sequence.
 *
 */
void dnx_handle_early_handshake(void);

#ifdef CONFIG_DNX_EC_ASSISTED_TRIGGER
#include "dnx_ec_assisted_trigger.h"
#endif

#endif /* __DNX_H__ */
