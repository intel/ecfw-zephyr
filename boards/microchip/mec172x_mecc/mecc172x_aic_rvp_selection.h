/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MEC17XX_AIC_RVP_SELECTION_H__
#define __MEC17XX_AIC_RVP_SELECTION_H__

#include "common_mec172x.h"
#include "mecc172x_aic_defs.h"

#if CONFIG_MEC172X_AIC_ON_ADL_S
#include "mecc172x_aic_on_adl_s.h"
#elif CONFIG_MEC172X_AIC_ON_ADL_P
#include "mecc172x_aic_on_adl_p.h"
#elif CONFIG_MEC172X_AIC_ON_MTL_P
#include "mecc172x_aic_on_mtl_p.h"
#else
#error "No RVP selected for MEC172x AIC"
#endif

#endif /* __MEC17XX_AIC_RVP_SELECTION_H__ */
