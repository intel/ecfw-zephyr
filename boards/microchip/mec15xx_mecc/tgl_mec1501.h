/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __TGL_MEC1501_H__
#define __TGL_MEC1501_H__

#include <soc.h>
#include "mec150x_pin.h"

#define KSC_PLAT_NAME                   "TGL"

#define PLATFORM_DATA(x, y)  ((x) | ((y) << 8))


/* In TGL (N-1) board id is 6-bits */
#define BOARD_ID_MASK        0x003Fu
#define BOM_ID_MASK          0x01C0u
#define FAB_ID_MASK          0x0600u
#define HW_STRAP_MASK        0xFC00u
#define HW_ID_MASK           (FAB_ID_MASK|BOM_ID_MASK|BOARD_ID_MASK)
#define BOARD_ID_OFFSET      0u
#define BOM_ID_OFFSET        6u
#define FAB_ID_OFFSET        9u
#define HW_STRAP_OFFSET      11u

/* Supported board ids */
#define BRD_ID_TGL_U_ERB                0x0Au
#define BRD_ID_TGL_U_CRB                0x01u

/* I2C addresses */
#define EEPROM_DRIVER_I2C_ADDR          0x50
#define IO_EXPANDER_0_I2C_ADDR          0x22

/* IO expander HW strap pins IO expander 1 */
#define SPD_PRSNT			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x03)
#define VIRTUAL_BAT			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x04)
#define VIRTUAL_DOCK			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x05)
#define RETIMER_BYPASS			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x06)
#define PECI_OVER_ESPI			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x07)

/* Not available */
#define THERM_STRAP			EC_DUMMY_GPIO_HIGH
#define G3_SAF_DETECT			EC_DUMMY_GPIO_HIGH
#define HOME_BUTTON			EC_DUMMY_GPIO_HIGH
#define EC_PWRBTN_LED			EC_DUMMY_GPIO_LOW
#define EC_PG3_EXIT			EC_DUMMY_GPIO_LOW
#define DG2_PRESENT			EC_DUMMY_GPIO_LOW
#define PEG_RTD3_COLD_MOD_SW_R		EC_DUMMY_GPIO_LOW
#define CPU_C10_GATE			EC_DUMMY_GPIO_HIGH

/* IO expander HW strap pins IO expander 2 */
#define PD_AIC_DETECT1			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x00)
#define PD_AIC_DETECT2			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x01)
#define PNP_NPN_SKU			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x02)
#define TIMEOUT_DISABLE			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x03)
#define SOC_VR_CNTRL			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x04)
#define EC_M_2_SSD_PLN			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x05)


#endif /* __TGL_MEC1501_H__ */
