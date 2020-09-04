/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __PWRSEQ_TIMEOUTS_H__
#define __PWRSEQ_TIMEOUTS_H__

/**
 * @brief Power sequence default timeout values expresed in 100us periods.
 */

/* 6 seconds */
#define SUSWARN_TIMEOUT             60000
#define SLPS5_TIMEOUT               60000
#define SLPS4_TIMEOUT               60000
#define PLT_RESET_TIMEOUT           60000
/* 3 s */
#define SLPS3_TIMEOUT               30000
#define SLP_M_TIMEOUT               30000
/* 1 s */
#define ALL_SYS_PWRGD_TIMEOUT       10000
/* 500 ms */
#define RSMRST_PWRDG_TIMEOUT        5000
/* 150 ms */
#define ESPI_RST_TIMEOUT            1500
/* 6s */
#define PM_SLP_SUS_TIMEOUT          60000

#endif /* __PWRSEQ_TIMEOUTS_H__ */
