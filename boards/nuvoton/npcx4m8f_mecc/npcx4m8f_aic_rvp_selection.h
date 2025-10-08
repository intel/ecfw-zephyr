/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __NPCX4M8F_AIC_RVP_SELECTION_H__
#define __NPCX4M8F_AIC_RVP_SELECTION_H__
#include "common_npcx.h"
#include "npcx4m8f_aic_defs.h"

#if CONFIG_BOARD_NPCX4M8F_PTL
#include "npcx4m8f_aic_on_ptl.h"
#else
#error "No RVP selected for NPCX4M8F AIC"
#endif

#endif /* __NPCX4M8F_AIC_RVP_SELECTION_H__ */
