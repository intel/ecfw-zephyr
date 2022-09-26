/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include "mec150x_pin.h"

#ifndef __COMMON_MEC1501_H__
#define __COMMON_MEC1501_H__



/* GPIO expander ports, offset from valid gpios */
#define EC_EXP_PORT_1			MCHP_GPIO_MAX_PORT
#define EC_EXP_PORT_2			(EC_EXP_PORT_1 + 1U)

/* Dummy gpio port */
#define EC_DUMMY_GPIO_PORT		0xFU

/* Dummy gpio default low */
#define EC_DUMMY_GPIO_LOW	EC_GPIO_PORT_PIN(EC_DUMMY_GPIO_PORT, 0x00)
/* Dummy gpio default high */
#define EC_DUMMY_GPIO_HIGH	EC_GPIO_PORT_PIN(EC_DUMMY_GPIO_PORT, 0x01)

#endif	/* __COMMON_MEC1501_H__ */
