/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MEC15XX_AIC_RVP_SELECTION_H__
#define __MEC15XX_AIC_RVP_SELECTION_H__

#include "common_mec1501.h"
#include "mec15xx_aic_defs.h"

#ifdef CONFIG_MEC15XX_AIC_ON_ICL
#include "icl_mec1501.h"
#elif CONFIG_MEC15XX_AIC_ON_TGL
#include "tgl_mec1501.h"
#elif CONFIG_MEC15XX_AIC_ON_ADL_S
#include "mec15xx_aic_on_adl_s.h"
#elif CONFIG_MEC15XX_AIC_ON_ADL_P
#include "mec15xx_aic_on_adl_p.h"
#else
#error "No RVP supported"
#endif


#endif /* __MEC15XX_AIC_RVP_SELECTION_H__ */
