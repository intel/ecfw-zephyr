/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include "mec150x_pin.h"
#include "common_mec1501.h"

#ifndef __ADL_MEC1501_H__
#define __ADL_MEC1501_H__

#define KSC_PLAT_NAME                   "ADL"

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

/* Signal to gpio mapping for MEC1501 based ADL-S is described here */

#define PM_SLP_SUS			EC_GPIO_000
#define REAR_FAN_CTRL			EC_GPIO_002
#define RSMRST_PWRGD_G3SAF_P		EC_GPIO_012
#define RSMRST_PWRGD_MAF_P		EC_GPIO_227

#define RSMRST_PWRGD			((boot_mode_maf == 1) ? \
					 RSMRST_PWRGD_MAF_P : \
					 RSMRST_PWRGD_G3SAF_P)

#define PS2_KB_DATA			EC_GPIO_010
#define ATX_DETECT			EC_GPIO_013
#define KBD_BKLT_CTRL			EC_GPIO_014
#define SYS_PWROK			EC_GPIO_043
#define SLP_S0_PLT_EC_N			EC_GPIO_051

#define PM_RSMRST_G3SAF_P		EC_GPIO_054
#define PM_RSMRST_MAF_P			EC_GPIO_055
#define PM_RSMRST			((boot_mode_maf == 1) ? \
					 PM_RSMRST_MAF_P : \
					 PM_RSMRST_G3SAF_P)

#define ALL_SYS_PWRGD			EC_GPIO_057
#define FAN_PWR_DISABLE_N		EC_GPIO_060
/* We poll this GPIO in MAF mode in order to sense the input signal040_076.
 * This pin was already configured in pinmux as ALT mode 1 NOT GPIO
 */
#define ESPI_RESET_MAF			EC_GPIO_061
#define KBC_SCROLL_LOCK			EC_GPIO_062

#define PCH_PWROK			EC_GPIO_106
#define WAKE_SCI			EC_GPIO_114
#define DNX_FORCE_RELOAD_EC		EC_GPIO_115
#define KBC_CAPS_LOCK			EC_GPIO_127
#define I2C_ALERT_P1			EC_GPIO_132
#define PM_BATLOW			EC_GPIO_140
#define CATERR_LED_DRV			EC_GPIO_153
#define PS2_MB_DATA			EC_GPIO_155
#define CS_INDICATE_LED			EC_GPIO_156
#define C10_GATE_LED			EC_GPIO_157
#define PECI_MUX_CTRL			EC_GPIO_162
#define PWRBTN_EC_IN_N			EC_GPIO_163
#define SOC_VR_CNTRL_PE			EC_GPIO_165
#define BC_ACOK				EC_GPIO_172
#define PS_ON_OUT			EC_GPIO_175

#define CPU_C10_GATE			EC_GPIO_204
#define EC_SMI				EC_GPIO_207
#define PM_SLP_S0_CS			EC_GPIO_221
#define PM_DS3				EC_GPIO_226
#define WAKE_CLK			EC_GPIO_241
#define VOL_UP				EC_GPIO_242
#define TOP_SWAP_OVERRIDE_GPIO		EC_GPIO_244
#define VOL_DOWN			EC_GPIO_246
#define PM_PWRBTN			EC_GPIO_250
#define PROCHOT				EC_GPIO_253
#define EC_M_2_SSD_PLN			EC_GPIO_254
#define KBC_NUM_LOCK			EC_GPIO_255

#define STD_ADP_PRSNT			EC_DUMMY_GPIO_LOW
#define EC_PG3_EXIT			EC_DUMMY_GPIO_LOW
#define VIRTUAL_BAT			EC_DUMMY_GPIO_LOW
#define VIRTUAL_DOCK			EC_DUMMY_GPIO_LOW
#define HOME_BUTTON			EC_DUMMY_GPIO_HIGH
#define SMC_LID				EC_DUMMY_GPIO_HIGH
#define DG2_PRESENT			EC_DUMMY_GPIO_LOW
#define PEG_RTD3_COLD_MOD_SW_R		EC_DUMMY_GPIO_LOW
#define EC_PWRBTN_LED			EC_DUMMY_GPIO_LOW

/* IO expander HW strap pins */
#define SPD_PRSNT			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x03)
#define G3_SAF_DETECT			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x04)
#define THERM_STRAP			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x05)
#define PECI_OVER_ESPI			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x06)
#define TIMEOUT_DISABLE			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x07)

/* Device instance names */
#define I2C_BUS_0			DT_NODELABEL(i2c_smb_0)
#define I2C_BUS_1			DT_NODELABEL(i2c_smb_1)
#define PS2_KEYBOARD			DT_NODELABEL(ps2_0)
#define PS2_MOUSE			DT_NODELABEL(ps2_1)
#define ESPI_0				DT_NODELABEL(espi0)
#define ESPI_SAF_0			DT_NODELABEL(espi_saf0)
#define SPI_0				DT_NODELABEL(spi0)
#define ADC_CH_BASE			DT_NODELABEL(adc0)
#define PECI_0_INST			DT_NODELABEL(peci0)
#define WDT_0				DT_NODELABEL(wdog)

/* Button/Switch Initial positions */
#define PWR_BTN_INIT_POS		1
#define VOL_UP_INIT_POS			1
#define VOL_DN_INIT_POS			1
#define LID_INIT_POS			1
#define HOME_INIT_POS			1
#define VIRTUAL_BAT_INIT_POS		1
#define VIRTUAL_DOCK_INIT_POS		1

/* TIPD UCSI version details
 * UCSI Version format is 0xJJMN (2 Bytes)
 * JJ – Manjor Version
 * M – Minor Version
 * N – Sub Minor Version
 */
#define TIPD_UCSI_MAJOR_VERSION 0x01
#define TIPD_UCSI_MINOR_VERSION 0x2
#define TIPD_UCSI_SUB_MINOR_VERSION 0x0

#endif /* __ADL_MEC1501_H__ */
