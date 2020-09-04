/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include "mec150x_pin.h"

#ifndef __TGL_MEC1501_H__
#define __TGL_MEC1501_H__

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
#define BRD_ID_TGL_U_ERB                    0x0A

/* I2C addresses */
#define EEPROM_DRIVER_I2C_ADDR          0x50
#define IO_EXPANDER_0_I2C_ADDR          0x22

/* Signal to gpio mapping for MEC1501 card + ICL platform is described here */

/* Following blue wires are required
 * PWRBTN_EC_OUT - from MECC TP65 to MECC J50.11 (GPIO250)
 * PWRBTN_EC_IN  - from MECC TP64 to MECC J50.8  (GPIO163)
 * PM_SLP_SUS    - from MECC TP55 to MECC J50.16
 * SYS_PWROK     - from MECC J51.15 (GPIO034) to RVP
 * PM_DS3        - from MECC J50.4  (GPIO165) to RVP
 */

#define STD_ADP_PRSNT			-1

#define PROCHOT				EC_GPIO_002
#define RSMRST_PWRGD_G3SAF_P		EC_GPIO_012
#define RSMRST_PWRGD_MAF_P		EC_GPIO_227
#define RSMRST_PWRGD			((boot_mode_maf == 1) ? \
					 RSMRST_PWRGD_MAF_P : \
					 RSMRST_PWRGD_G3SAF_P)
#define PM_SLP_SUS			EC_GPIO_034
#define VOL_UP				EC_GPIO_036
#define SYS_PWROK			EC_GPIO_043
#define PM_RSMRST_G3SAF_P		EC_GPIO_054
#define PM_RSMRST_MAF_P			EC_GPIO_055
#define PM_RSMRST			((boot_mode_maf == 1) ? \
					 PM_RSMRST_MAF_P : PM_RSMRST_G3SAF_P)
#define ALL_SYS_PWRGD			EC_GPIO_057
/* We poll this GPIO in MAF mode in order to sense the input signal.
 * This pin was already configured in pinmux as ALT mode 1 NOT GPIO
 */
#define ESPI_RESET_MAF			EC_GPIO_061
#define PM_SLP_S0_CS			EC_GPIO_100
#define PCH_PWROK			EC_GPIO_106
#define VCCST_PWRGD			EC_GPIO_106
#define WAKE_SCI			EC_GPIO_114
#define PM_BATLOW			EC_GPIO_115
#define FAN_PWR_DISABLE_N		EC_GPIO_141
#define TYPEC_ALERT_2			EC_GPIO_153
#define BC_ACOK				EC_GPIO_156
#define TYPEC_ALERT_1			EC_GPIO_162
#define PWRBTN_EC_IN_N			EC_GPIO_163
#define PM_DS3				EC_GPIO_165
#define BATT_ID				EC_GPIO_206
#define SMC_LID				EC_GPIO_226
#define PM_SLP_S0_N			EC_GPIO_243
#define PM_PWRBTN			EC_GPIO_250
#define VOL_DOWN			EC_GPIO_254

/* Offset from valid gpios */
#define EC_EXP_PORT			(MCHP_GPIO_240_276 + 1U)
#define EC_EXP_PORT2			(MCHP_GPIO_240_276 + 2U)

/* IO expander HW strap pins IO expander 1 */
#define G3_SAF_DETECT			-1
#define SPD_PRSNT			EC_GPIO_PORT_PIN(EC_EXP_PORT, 0x03)
#define VIRTUAL_BAT			EC_GPIO_PORT_PIN(EC_EXP_PORT, 0x04)
#define VIRTUAL_DOCK			EC_GPIO_PORT_PIN(EC_EXP_PORT, 0x05)
#define RETIMER_BYPASS			EC_GPIO_PORT_PIN(EC_EXP_PORT, 0x06)
#define PECI_OVER_ESPI			EC_GPIO_PORT_PIN(EC_EXP_PORT, 0x07)
#define THERM_STRAP			-1

/* IO expander HW strap pins IO expander 2 */
#define PD_AIC_DETECT1			EC_GPIO_PORT_PIN(EC_EXP_PORT, 0x00)
#define PD_AIC_DETECT2			EC_GPIO_PORT_PIN(EC_EXP_PORT, 0x01)
#define PNP_NPN_SKU			EC_GPIO_PORT_PIN(EC_EXP_PORT, 0x02)
#define TIMEOUT_DISABLE			EC_GPIO_PORT_PIN(EC_EXP_PORT, 0x03)
#define SOC_VR_CNTRL			EC_GPIO_PORT_PIN(EC_EXP_PORT, 0x04)
#define M2_SSD_PLN_DELAY		EC_GPIO_PORT_PIN(EC_EXP_PORT, 0x05)

/* Device instance names */
#define I2C_BUS_0			DT_LABEL(DT_NODELABEL(i2c_smb_0))
#define I2C_BUS_1			DT_LABEL(DT_NODELABEL(i2c_smb_1))
#define PS2_KEYBOARD			DT_LABEL(DT_NODELABEL(ps2_0))
#define PS2_MOUSE			DT_LABEL(DT_NODELABEL(ps2_1))
#define ESPI_0				DT_LABEL(DT_NODELABEL(espi0))
#define ADC_CH_BASE			DT_LABEL(DT_NODELABEL(adc0))
#define PECI_0_INST			DT_LABEL(DT_NODELABEL(peci0))

/* Button/Switch Initial positions */
#define PWR_BTN_INIT_POS		1
#define VOL_UP_INIT_POS			1
#define VOL_DN_INIT_POS			1

#endif /* __TGL_MEC1501_H__ */
