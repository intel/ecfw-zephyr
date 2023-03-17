/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include "mec150x_pin.h"
#include "common_mec1501.h"

#ifndef __ADL_P_MEC1501_H__
#define __ADL_P_MEC1501_H__

extern uint8_t platformskutype;

#define KSC_PLAT_NAME                   "ADLP"

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


/* Support adl skus */
enum platform_skus {
	PLATFORM_ADL_P_SKUs = 0,
	PLATFORM_ADL_M_SKUs = 1,
	PLATFORM_ADL_N_SKUs = 2,
};

/* Support board ids */
#define BRD_ID_ADL_P_ERB		0x10u
#define BRD_ID_ADL_P_LP4_T3		0x10u
#define BRD_ID_ADL_P_LP4_T3_HSIO		0x11u
#define BRD_ID_ADL_P_DDR5_T3		0x12u
#define BRD_ID_ADL_P_LP5_T4		0x13u
#define BRD_ID_ADL_P_DDR4_T3		0x14u
#define BRD_ID_ADL_P_LP5_T3		0x15u
#define BRD_ID_ADL_P_LP4_BEP		0x19u
#define BRD_ID_ADL_P_LP5_AEP		0x1Au
#define BRD_ID_ADL_P_GCS		0x1Bu
#define BRD_ID_ADL_P_DG2_384EU_AEP		0x1Eu
#define BRD_ID_ADL_P_DG2_128EU_AEP		0x1Fu

#define BRD_ID_ADL_M_LP4		0x01u
#define BRD_ID_ADL_M_LP5		0x02u
#define BRD_ID_ADL_M_LP5_2PMIC		0x03u
#define BRD_ID_ADL_M_RVP2A_PPV		0x04u

#define BRD_ID_ADL_N_ERB		0x06u
#define BRD_ID_ADL_N_LP5		0x07u
#define BRD_ID_ADL_N_DDR4		0x08u
#define BRD_ID_ADL_N_DDR5		0x09u

/* I2C addresses */
#define EEPROM_DRIVER_I2C_ADDR          0x50
#define IO_EXPANDER_0_I2C_ADDR          0x22

/* Signal to gpio mapping for MEC1501 based ADL-P is described here */

#define PM_SLP_SUS			EC_GPIO_000
#define EC_SPI_CS1_N			EC_GPIO_002
#define RSMRST_PWRGD_G3SAF_P		EC_GPIO_012
#define RSMRST_PWRGD_MAF_P		EC_GPIO_227

#define RSMRST_PWRGD			((boot_mode_maf == 1) ? \
					 RSMRST_PWRGD_MAF_P : \
					 RSMRST_PWRGD_G3SAF_P)

#define PS2_KB_DATA			EC_GPIO_010
#define G3_SAF_DETECT			EC_GPIO_013
#define KBD_BKLT_CTRL			EC_GPIO_014
#define PM_BAT_STATUS_LED1		EC_GPIO_015
#define CPU_C10_GATE			EC_GPIO_022
#define HOME_BUTTON			EC_GPIO_023
#define EC_PCH_SPI_OE_N			EC_GPIO_024
#define PECI_MUX_CTRL			EC_GPIO_025
#define SMC_LID				EC_GPIO_033
#define PM_BAT_STATUS_LED2		EC_GPIO_035
#define PEG_PLI_N_DG2			EC_GPIO_036
#define SYS_PWROK			EC_DUMMY_GPIO_HIGH
#define PCA9555_0_R_INT_N		EC_GPIO_051
#define STD_ADP_PRSNT			EC_GPIO_052

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
#define PCA9555_1_R_INT_N		EC_GPIO_062
#define EC_SLATEMODE_HALLOUT_SNSR_R	EC_GPIO_064

#define PM_PWRBTN			EC_GPIO_101
#define EC_SMI				EC_GPIO_102
#define PCH_PWROK			EC_GPIO_106
#define WAKE_SCI			EC_GPIO_114
#define DNX_FORCE_RELOAD_EC		EC_GPIO_115
#define KBC_CAPS_LOCK			EC_GPIO_127
#define TYPEC_EC_SMBUS_ALERT_0_R	EC_GPIO_132
#define PM_BATLOW			EC_GPIO_140
#define CATERR_LED_DRV			EC_GPIO_153
#define PS2_MB_DATA                     EC_GPIO_155
#define PEG_RTD3_COLD_MOD_SW_R		EC_GPIO_156
#define CS_INDICATE_LED			EC_GPIO_156		/* ADL-M */
#define C10_GATE_LED			EC_GPIO_157		/* ADL-M */
#define HB_NVDC_SEL			EC_GPIO_161
#define BATT_ID_N			EC_GPIO_162
#define PWRBTN_EC_IN_N			EC_GPIO_163
#define DG2_PRESENT			EC_GPIO_165
#define RST_MECC			EC_GPIO_165		/* ADL-M */
#define STD_ADPT_CNTRL_GPIO		EC_GPIO_171
#define BC_ACOK				EC_GPIO_172
#define SX_EXIT_HOLDOFF_N		EC_GPIO_175

#define PM_SLP_S0_CS			EC_GPIO_221
#define RETIMER_FORCE_PWR_BTP_EC_R	EC_GPIO_222
#define PM_DS3				EC_GPIO_226
#define PEG_PIM_DG2			EC_GPIO_240
#define PM_SLP_S0_EC_N		EC_GPIO_240		/* ADL-M */
#define WAKE_CLK			EC_GPIO_241
#define VOL_UP				EC_GPIO_242
#define TOP_SWAP_OVERRIDE_GPIO		EC_GPIO_244
#define TYPEC_EC_SMBUS_ALERT_1_R	EC_GPIO_245
#define VOL_DOWN			EC_GPIO_246
#define EC_PG3_EXIT			EC_GPIO_250
#define PROCHOT				EC_GPIO_253
#define EC_PWRBTN_LED			EC_GPIO_254
#define KBC_NUM_LOCK			EC_GPIO_255

/* IO expander HW strap pins IO expander 1 */
#define SPD_PRSNT			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x03)
#define DISPLAY_ID_0			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x04)
#define DISPLAY_ID_1			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x05)
#define DISPLAY_ID_2			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x06)
#define DISPLAY_ID_3			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x07)

/* IO expander HW strap pins IO expander 2 */
#define PD_AIC_DETECT1			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x00)
#define PD_AIC_DETECT2			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x01)
/* Pin2 is named as PNP_NPNP_SKU in schematics but using this PECI_OVER_ESPI */
#define PECI_OVER_ESPI			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x02)
#define TIMEOUT_DISABLE			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x03)
#define SOC_VR_CNTRL			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x04)
#define EC_M_2_SSD_PLN			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x05)
#define VIRTUAL_BAT			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x06)
#define VIRTUAL_DOCK			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x07)
/* IO expander pin mapping is in hex decimal format instead of octal*/
/* Net name TP_ESPI_TESTCRD_DET */
#define THERM_STRAP			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x08)
#define KBC_SCROLL_LOCK_P		EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x0D)

/* Net name TP_RETIMER_BYPASS_STRAP */
#define RETIMER_BYPASS			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x06)

#define KBC_SCROLL_LOCK_M		EC_GPIO_245	/* ADL-M */
#define KBC_SCROLL_LOCK		((platformskutype == PLATFORM_ADL_M_SKUs) ? \
					KBC_SCROLL_LOCK_M : KBC_SCROLL_LOCK_P)

/* Device instance names */
#define I2C_BUS_0			DT_NODELABEL(i2c_smb_0)
#define I2C_BUS_1			DT_NODELABEL(i2c_smb_1)
#if DT_NODE_HAS_STATUS(DT_NODELABEL(i2c_smb_2), okay)
#define I2C_BUS_2			DT_NODELABEL(i2c_smb_2)
#endif
#define PS2_KEYBOARD			DT_NODELABEL(ps2_0)
#define PS2_MOUSE			DT_NODELABEL(ps2_1)
#define ESPI_0				DT_NODELABEL(espi0)
#define ESPI_SAF_0			DT_NODELABEL(espi_saf0)
#define SPI_0				DT_NODELABEL(spi0)
#define ADC_CH_BASE			DT_NODELABEL(adc0)
#define PECI_0_INST			DT_NODELABEL(peci0)
#define KSCAN_MATRIX			DT_NODELABEL(kscan0)
#define WDT_0				DT_NODELABEL(wdog)
#define BAT_LED2			DT_ALIAS(pwm_led8)
#define KBD_BKLT_LED			DT_ALIAS(pwm_led6)

/* Button/Switch Initial positions */
#define PWR_BTN_INIT_POS		1
#define VOL_UP_INIT_POS			1
#define VOL_DN_INIT_POS			1
#define LID_INIT_POS			1
#define HOME_INIT_POS			1
#define SLATEMODE_INIT_POS		1
#define IOEXP_INIT_POS			1
#define VIRTUAL_BAT_INIT_POS		1
#define VIRTUAL_DOCK_INIT_POS		1

#define PM_BAT_STATUS_LED2_PWM_CHANNEL	8

#define CHARGER_CURRENT_LIMIT1_PERCENTAGE \
	((platformskutype == PLATFORM_ADL_M_SKUs) ? 90 : 90)
#define CHARGER_CURRENT_LIMIT2_PERCENTAGE \
	((platformskutype == PLATFORM_ADL_M_SKUs) ? 100 : 100)
#define CHARGER_AC_PROCHOT_PERCENTAGE \
	((platformskutype == PLATFORM_ADL_M_SKUs) ? 110 : 110)

#define CHARGER_FAST_CHARGING_CURRENT \
	((platformskutype == PLATFORM_ADL_M_SKUs) ? 0x1E14 : \
	(platformskutype == PLATFORM_ADL_N_SKUs) ? 0x1EC0 : 0x2FF8)

/* As per ISL charger spec, ISL92XX_CURRENT_LIMIT register needs to be
 * programmed based on the value of Rs2 (input current sense resistor).
 *
 * Same case with ISL92XX_ADP_CURRENT_LIMIT1, ISL92XX_ADP_CURRENT_LIMIT2,
 * ISL92XX_AC_PROCHOT and ISL92XX_DC_PROCHOT registers which need to be
 * programmed based on the value of Rs1.
 *
 * RS1/RS2 is always configured in pairs either as 20/10 mohms or as 10/5
 * mohms. This may differ from platform to platform. If it is 20/10, the
 * DIVISOR value should be 2. If it is 10/5, the DIVISOR value should be 1.
 *
 * Note: The Divisor value is provided by the PD team.
 */
#define CHARGER_CURRENT_LIMIT_DIVISOR \
	((platformskutype == PLATFORM_ADL_N_SKUs) ? 1U : 2U)

/* Minimum Adapter power(Milli Watts) for proceeding with boot */
#define ADP_CRIT_POWERUP		26000

#define TIPD_PORT_0_I2C_ADDR	0x20U
#define TIPD_PORT_1_I2C_ADDR	0x24U
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
#define TIPD_UCSI_MAJOR_VERSION 0x01
#define TIPD_UCSI_MINOR_VERSION 0x2
#define TIPD_UCSI_SUB_MINOR_VERSION 0x0

#endif /* __ADL_P_MEC1501_H__ */
