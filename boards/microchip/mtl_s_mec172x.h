/*
 * Copyright (c) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include "mec172x_pin.h"
#include "common_mec172x.h"

#ifndef __MEC172x_MTL_S__
#define __MEC172x_MTL_S__

#define KSC_PLAT_NAME                   "MTLS"
extern uint8_t platformskutype;

#define PLATFORM_DATA(x, y)  ((x) | ((y) << 8))

/* In MTL board id is 6-bits */
#define BOARD_ID_MASK        0x003Fu
#define BOM_ID_MASK          0x01C0u
#define FAB_ID_MASK          0x0600u
#define HW_STRAP_MASK        0xF800u
#define HW_ID_MASK           (FAB_ID_MASK|BOM_ID_MASK|BOARD_ID_MASK)
#define BOARD_ID_OFFSET      0u
#define BOM_ID_OFFSET        6u
#define FAB_ID_OFFSET        9u
#define HW_STRAP_OFFSET      11u

/* Support RPL skus */
enum platform_skus {
	PLATFORM_MTL_S_ERB_SKUs = 0,
	PLATFORM_MTL_S_CRB_SKUs = 1,
};

/* Support board ids */
#define BRD_ID_MTL_S_UDIMM_1DPC_ERB		0x20u
#define BRD_ID_MTL_S_UDIMM_1DPC_CRB		0x21u
#define BRD_ID_MTL_S_UDIMM_2DPC_ERB		0x22u
#define BRD_ID_MTL_S_SODIMM_1DPC_ERB		0x23u
#define BRD_ID_MTL_S_SODIMM_1DPC_CRB		0x24u
#define BRD_ID_MTL_S_OC_ERB		0x25u
#define BRD_ID_MTL_S_OC_EV_CRB		0x26u
#define BRD_ID_MTL_S_HSIO_RVP		0x27u
#define BRD_ID_MTL_S_SODIMM_2DPC_CRB		0x29u
#define BRD_ID_MTL_S_uATX_6L_CRB		0x2A

/* I2C addresses */
#define EEPROM_DRIVER_I2C_ADDR          0x50
#define IO_EXPANDER_0_I2C_ADDR          0x22

/* Signal to gpio mapping for MEC1728 based MTL-S is described here */

#define PM_SLP_SUS			EC_GPIO_000
#define EC_SPI_CS1_N			EC_GPIO_002
#define PS2_KB_DATA			EC_GPIO_010
#define RSMRST_PWRGD_G3SAF_P		EC_GPIO_012
#define RSMRST_PWRGD_MAF_P		EC_GPIO_227

#define RSMRST_PWRGD			((boot_mode_maf == 1) ? \
					 RSMRST_PWRGD_MAF_P : \
					 RSMRST_PWRGD_G3SAF_P)

#define ATX_DETECT			EC_GPIO_013

#define POWER_STATE			EC_GPIO_022
#define RECOVERY_INDICATOR_N		EC_GPIO_023
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
#define PWRBTN_EC_IN_N			EC_GPIO_067
#define PCH_PWROK			EC_GPIO_106
#define PS2_MB_DATA			EC_GPIO_115
#define KBC_CAPS_LOCK			EC_GPIO_127
#define TYPEC_EC_SMBUS_ALERT_0_R	EC_GPIO_143
#define CATERR_LED_DRV			EC_GPIO_153
#define WAKE_SCI			EC_GPIO_155
#define CS_INDICATE_LED			EC_GPIO_156
#define PS_ON_IN_EC_N			EC_GPIO_157
#define PECI_MUX_CTRL			EC_GPIO_162
#define PROCHOT				EC_GPIO_171
#define BC_ACOK				EC_DUMMY_GPIO_HIGH
#define PS_ON_OUT			EC_GPIO_175

#define CPU_C10_GATE			EC_GPIO_204
#define EC_SMI				EC_GPIO_207
#define PM_SLP_S0_CS			EC_GPIO_221
#define SLATE_MODE			EC_GPIO_222

#define DNX_FORCE_RELOAD_EC		EC_GPIO_226

#define PM_BATLOW			EC_DUMMY_GPIO_HIGH


#define PM_DS3				EC_DUMMY_GPIO_LOW
#define EC_S0IX_ENTRY_REQ		EC_GPIO_240
#define WAKE_CLK			EC_GPIO_241
#define VOL_UP				EC_GPIO_242
#define TOP_SWAP_OVERRIDE_GPIO		EC_GPIO_244
#define VOL_DOWN			EC_GPIO_246
#define PM_PWRBTN_ERB			EC_GPIO_047
#define PM_PWRBTN_CRB			EC_GPIO_165
#define PM_PWRBTN			((platformskutype == PLATFORM_MTL_S_ERB_SKUs) ? \
					 PM_PWRBTN_ERB : \
					 PM_PWRBTN_CRB)

#define EC_M_2_SSD_PLN			EC_GPIO_254
#define EC_S0IX_ENTRY_ACK		EC_GPIO_255

#define KBC_NUM_LOCK_ERB		EC_GPIO_255
#define KBC_NUM_LOCK_CRB		EC_GPIO_161
#define KBC_NUM_LOCK			((platformskutype == PLATFORM_MTL_S_ERB_SKUs) ? \
					 KBC_NUM_LOCK_ERB : \
					 KBC_NUM_LOCK_CRB)

#define KBD_BKLT_CTRL			EC_DUMMY_GPIO_HIGH
#define STD_ADP_PRSNT			EC_DUMMY_GPIO_LOW
#define EC_PG3_EXIT			EC_DUMMY_GPIO_LOW
#define VIRTUAL_BAT			EC_DUMMY_GPIO_LOW
#define VIRTUAL_DOCK			EC_DUMMY_GPIO_LOW
#define HOME_BUTTON			EC_DUMMY_GPIO_HIGH
#define SMC_LID				EC_DUMMY_GPIO_HIGH
#define DG2_PRESENT			EC_DUMMY_GPIO_LOW
#define PEG_RTD3_COLD_MOD_SW_R		EC_DUMMY_GPIO_LOW
#define EC_PWRBTN_LED			EC_DUMMY_GPIO_LOW

/* IO expander HW strap pins IO expander 1 */
#define SPD_PRSNT			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x03)
#define DISPLAY_ID_0			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x04)
#define DISPLAY_ID_1			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x05)
#define DISPLAY_ID_2			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x06)
#define DISPLAY_ID_3			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x07)

/* IO expander HW strap pins IO expander 2 */
#define THERM_STRAP			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x08)
#define G3_SAF_DETECT			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x09)
/* Pin2 is named as PMC_GPIO_BOOT_HALT */
#define TIMEOUT_DISABLE			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x0A)
#define ESPI_TESTCRD_DET		EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x0B)
#define MEM_OC_N			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x0C)
/* Pin5 is named as VAL_PECI_ESPI */
#define PECI_OVER_ESPI			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x0D)

/* Device instance names */
#define I2C_BUS_0			DT_NODELABEL(i2c_smb_0)
#define I2C_BUS_1			DT_NODELABEL(i2c_smb_1)
#define PS2_KEYBOARD			DT_NODELABEL(ps2_0)
#define PS2_MOUSE			DT_NODELABEL(ps2_0)
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

#define TIPD_PORT_0_I2C_ADDR	0x23U
#define TIPD_PORT_1_I2C_ADDR	0x27U
#define TIPD_PORT_2_I2C_ADDR	0x21U
#define TIPD_PORT_3_I2C_ADDR	0x25U

/* PD version: MSB byte represent Major version and
 * LSB byte represent Minor version
 */
#define USB_PD_VERSION 0x0200

/* TIPD UCSI version details
 * UCSI Version format is 0xJJMN (2 Bytes)
 * JJ – Manjor Version
 * M – Minor Version
 * N – Sub Minor Version
 */
#define TIPD_UCSI_MAJOR_VERSION 0x02
#define TIPD_UCSI_MINOR_VERSION 0x0
#define TIPD_UCSI_SUB_MINOR_VERSION 0x0

#endif /* __MEC172x_MTL_S__ */
