/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __ICL_MEC1501_H__
#define __ICL_MEC1501_H__

#include <soc.h>
#include "mec150x_pin.h"

#define KSC_PLAT_NAME                   "ADL_N_1"

#define PLATFORM_DATA(x, y)  ((x) | ((y) << 8))

/* In ICL (N-1) board id is 5-bits */
#define BOARD_ID_MASK        0x001Fu
#define BOM_ID_MASK          0x00E0u
#define FAB_ID_MASK          0x0300u
#define HW_STRAP_MASK        0xF800u
#define HW_ID_MASK           (FAB_ID_MASK|BOM_ID_MASK|BOARD_ID_MASK)
#define BOARD_ID_OFFSET      0u
#define BOM_ID_OFFSET        5u
#define FAB_ID_OFFSET        8u
#define HW_STRAP_OFFSET      10u

/* Supported board ids */
#define BRD_ID_ICLY_LPDDR4_TYPE4_RVP_PPV    0x06u

/* I2C addresses */
#define EEPROM_DRIVER_I2C_ADDR          0x50
#define IO_EXPANDER_0_I2C_ADDR          0x22

/* IO expander HW strap pins IO expander 1 */
#define G3_SAF_DETECT			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x02)
#define SPD_PRSNT			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x03)
#define VIRTUAL_BAT			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x04)
#define VIRTUAL_DOCK			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x05)
#define RETIMER_BYPASS			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x06)
#define PECI_OVER_ESPI			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x07)
#define THERM_STRAP			EC_DUMMY_GPIO_HIGH
#define TIMEOUT_DISABLE			EC_DUMMY_GPIO_HIGH
#define DG2_PRESENT			EC_DUMMY_GPIO_LOW
#define PEG_RTD3_COLD_MOD_SW_R		EC_DUMMY_GPIO_LOW
#define EC_PWRBTN_LED			EC_DUMMY_GPIO_LOW
#define CPU_C10_GATE			EC_DUMMY_GPIO_HIGH

/* Not available */
#define HOME_BUTTON			EC_DUMMY_GPIO_HIGH
#define EC_PG3_EXIT			EC_DUMMY_GPIO_LOW

#endif /* __ICL_MEC1501_H__ */
