/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include "mec150x_pin.h"
#include "common_mec1501.h"

#ifndef __ADL_S_MEC1501_H__
#define __ADL_S_MEC1501_H__

#define KSC_PLAT_NAME                   "ADL_MECC"

#define PLATFORM_DATA(x, y)  ((x) | ((y) << 8))

/* In ADL board id is 6-bits */
#define BOARD_ID_MASK        0x003Fu
#define BOM_ID_MASK          0x01C0u
#define FAB_ID_MASK          0x0600u
#define HW_STRAP_MASK        0xF800u
#define HW_ID_MASK           (FAB_ID_MASK|BOM_ID_MASK|BOARD_ID_MASK)
#define BOARD_ID_OFFSET      0u
#define BOM_ID_OFFSET        6u
#define FAB_ID_OFFSET        9u
#define HW_STRAP_OFFSET      11u

/* Support board ids */
#define BRD_ID_ADL_S_ERB                              0x19u
#define BRD_ID_ADL_S_S01_TGP_H_SODIMM_DRR4            0x20u
#define BRD_ID_ADL_S_S01_TGP_H_SODIMM_DRR4_CRB        0x21u
#define BRD_ID_ADL_S_S01_TGP_H_SODIMM_DRR4_PPV        0x22u
#define BRD_ID_ADL_S_S02_TGP_H_SODIMM_DRR4_CRB        0x24u
#define BRD_ID_ADL_S_S03_ADP_S_UDIMM_DRR4_ERB1        0x26u
#define BRD_ID_ADL_S_S03_ADP_S_UDIMM_DRR4_CRB         0x27u
#define BRD_ID_ADL_S_S04_ADP_S_UDIMM_DRR4_CRB_EVCRB   0x28u
#define BRD_ID_ADL_S_S05_ADP_S_UDIMM_DRR4_CRB_CPV     0x29u
#define BRD_ID_ADL_S_S06_ADP_S_UDIMM_DRR4_CRB         0x2Bu
#define BRD_ID_ADL_S_S09_ADP_S_UDIMM_DRR4_CRB_PPV     0x2Cu
#define BRD_ID_ADL_S_S07_ADP_S_UDIMM_DRR4_CPV         0x2Eu
#define BRD_ID_ADL_S_S08_ADP_S_SODIMM_DRR5_CRB        0x30u

/* I2C addresses */
#define EEPROM_DRIVER_I2C_ADDR          0x50
#define IO_EXPANDER_0_I2C_ADDR          0x22

#define VIRTUAL_BAT			EC_DUMMY_GPIO_LOW
#define VIRTUAL_DOCK			EC_DUMMY_GPIO_LOW
#define HOME_BUTTON			EC_DUMMY_GPIO_HIGH
#define EC_PWRBTN_LED			EC_DUMMY_GPIO_LOW
#define DG2_PRESENT			EC_DUMMY_GPIO_LOW
#define EC_PG3_EXIT			EC_DUMMY_GPIO_LOW
#define PEG_RTD3_COLD_MOD_SW_R		EC_DUMMY_GPIO_LOW
#define CPU_C10_GATE			EC_DUMMY_GPIO_HIGH

/* IO expander HW strap pins */
#define SPD_PRSNT			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x03)
#define G3_SAF_DETECT			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x04)
#define THERM_STRAP			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x05)
#define PECI_OVER_ESPI			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x06)
#define TIMEOUT_DISABLE			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x07)

#endif /* __ADL_S_MEC1501_H__ */
