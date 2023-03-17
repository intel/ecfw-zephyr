/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include "mec150x_pin.h"
#include "common_mec1501.h"

#ifndef __MTL_P_MEC1501_H__
#define __MTL_P_MEC1501_H__

#define KSC_PLAT_NAME                   "MTLP"

extern uint8_t platformskutype;
extern uint8_t pd_i2c_addr_set;

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

/* Support MTL SKUs */
enum platform_skus {
	PLATFORM_MTL_P_SKUs = 0,
	PLATFORM_MTL_P_SBS_SKUs = 1,
	PLATFORM_MTL_M_SKUs = 2,
};

/* MTL PD I2C Address Group*/
enum i2c_addr_set {
	MTL_P_I2C_SET1 = 0,
	MTL_P_I2C_SET2 = 1,
};

/* Support board ids */
#define BRD_ID_MTL_P_DDR5_SBS		0x01
#define BRD_ID_MTL_P_LP5_T3		0x02
#define BRD_ID_MTL_P_DDR5_B2B_HSIO	0x03
#define BRD_ID_MTL_P_LP5_T4		0x04
#define BRD_ID_MTL_P_DDR5_SOLDERED_DTBT	0x05
#define BRD_ID_MTL_M_LP5_CONF1		0x06
#define BRD_ID_MTL_M_LP5_CONF2		0x07
#define BRD_ID_MTL_M_LP5_PMIC		0x08
#define BRD_ID_MTL_P_DDR5_SBS_ERB	0x0D
#define BRD_ID_MTL_M_LP5_CONF1_ERB	0x0E

/* I2C addresses */
#define EEPROM_DRIVER_I2C_ADDR          0x50
#define IO_EXPANDER_0_I2C_ADDR          0x22

/* Signal to GPIO mapping for MEC1501 based MTL-P described here */
#define HOME_BUTTON			EC_GPIO_000
#define EC_SPI_CS1_N			EC_GPIO_002
#define PS2_KB_DATA			EC_GPIO_010
#define RSMRST_PWRGD_G3SAF_P		EC_GPIO_012
#define CPU_C10_GATE			EC_GPIO_013
#define DNX_FORCE_RELOAD_EC		EC_GPIO_014
#define PM_BAT_STATUS_LED1		EC_GPIO_015
#define G3_SAF_DETECT			EC_GPIO_022
#define RECOVERY_INDICATOR_N		EC_GPIO_023
#define EC_PCH_SPI_OE_N			EC_GPIO_024
#define PECI_MUX_CTRL			EC_GPIO_025
#define SMC_LID				EC_GPIO_033
#define PEG_PLI_N_DG2			EC_GPIO_036
#define PCA9555_0_R_INT_N		EC_GPIO_051
#define PM_SLP_S0_CS			EC_GPIO_222
#define PM_BAT_STATUS_LED2		EC_GPIO_053
#define PM_RSMRST_G3SAF_P		EC_GPIO_054
#define PM_RSMRST_MAF_P			EC_GPIO_055
#define ALL_SYS_PWRGD			EC_GPIO_057
#define FAN_PWR_DISABLE_N		EC_GPIO_060

/*
 * We poll this GPIO in MAF mode in order to sense the input signal040_076.
 * This pin was already configured in pinmux as ALT mode 1 NOT GPIO.
 */
#define ESPI_RESET_MAF			EC_GPIO_061
#define PCA9555_1_R_INT_N		EC_GPIO_062
#define EC_SLATEMODE_HALLOUT_SNSR_R	EC_GPIO_064
#define PM_PWRBTN_D			EC_GPIO_101
#define PM_PWRBTN_P			EC_GPIO_024
#define PM_PWRBTN			((platformskutype == PLATFORM_MTL_P_SBS_SKUs) ? \
					 PM_PWRBTN_D : \
					 PM_PWRBTN_P)
#define EC_SMI				EC_GPIO_102
#define STD_ADP_PRSNT			EC_GPIO_106
#define WAKE_SCI			EC_GPIO_114
#define KBD_BKLT_CTRL_D			EC_GPIO_115
#define KBD_BKLT_CTRL_P			EC_GPIO_011
#define KBD_BKLT_CTRL			((platformskutype == PLATFORM_MTL_P_SBS_SKUs) ? \
					 KBD_BKLT_CTRL_D : \
					 KBD_BKLT_CTRL_P)

#define KBC_CAPS_LOCK			EC_GPIO_127
#define M2_SSD_EC_SMB_DATA		EC_GPIO_141
#define M2_SSD_EC_SMB_CLK		EC_GPIO_142
#define TYPEC_EC_SMBUS_ALERT_0_R	EC_GPIO_143
#define PM_BATLOW			EC_GPIO_144
#define CATERR_LED_DRV			EC_GPIO_153
#define PS2_MB_DATA			EC_GPIO_155
#define PEG_RTD3_COLD_MOD_SW_R		EC_GPIO_156
#define CS_INDICATE_LED			EC_GPIO_156
#define C10_GATE_LED			EC_GPIO_157
#define HB_NVDC_SEL_D			EC_GPIO_161
#define HB_NVDC_SEL_P			EC_GPIO_101
#define HB_NVDC_SEL			((platformskutype == PLATFORM_MTL_P_SBS_SKUs) ? \
					 HB_NVDC_SEL_D : \
					 HB_NVDC_SEL_P)

#define BATT_ID_N			EC_GPIO_162
#define PWRBTN_EC_IN_N			EC_GPIO_163
#define DG2_PRESENT			EC_GPIO_165
#define RST_MECC			EC_GPIO_165
#define STD_ADPT_CNTRL_GPIO		EC_GPIO_171
#define BC_ACOK				EC_GPIO_172
#define SX_EXIT_HOLDOFF_N		EC_GPIO_175
#define RETIMER_FORCE_PWR_BTP_EC_R	EC_GPIO_221
#define PM_DS3				EC_GPIO_226

#define RSMRST_PWRGD_MAF_P		EC_GPIO_227
#define PEG_PIM_DG2			EC_GPIO_240
#define PM_SLP_S0_EC_N			EC_GPIO_240
#define WAKE_CLK			EC_GPIO_241
#define VOL_UP				EC_GPIO_242
#define PCH_PWROK			EC_GPIO_243
#define TOP_SWAP_OVERRIDE_GPIO		EC_GPIO_244
#define TYPEC_EC_SMBUS_ALERT_1_R	EC_GPIO_245
#define VOL_DOWN			EC_GPIO_246
#define EC_PG3_EXIT_D			EC_GPIO_011
#define EC_PG3_EXIT_P			EC_GPIO_115
#define EC_PG3_EXIT			((platformskutype == PLATFORM_MTL_P_SBS_SKUs) ? \
					 EC_PG3_EXIT_D : \
					 EC_PG3_EXIT_P)
#define MIC_PRIVACY_SWITCH_D		EC_GPIO_250
#define MIC_PRIVACY_SWITCH_P		EC_GPIO_161
#define MIC_PRIVACY_SWITCH		((platformskutype == PLATFORM_MTL_P_SBS_SKUs) ? \
					 MIC_PRIVACY_SWITCH_D : \
					 MIC_PRIVACY_SWITCH_P)

#define PROCHOT				EC_GPIO_253
#define EC_PWRBTN_LED			EC_GPIO_254
#define KBC_NUM_LOCK			EC_GPIO_255

#define RSMRST_PWRGD			((boot_mode_maf == 1) ? \
					 RSMRST_PWRGD_MAF_P : \
					 RSMRST_PWRGD_G3SAF_P)

#define PM_RSMRST			((boot_mode_maf == 1) ? \
					 PM_RSMRST_MAF_P : \
					 PM_RSMRST_G3SAF_P)

#define SYS_PWROK			EC_GPIO_043
#define PM_SLP_SUS			EC_DUMMY_GPIO_HIGH

/* IO expander HW strap pins IO expander 1 */
#define SPD_PRSNT			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x03)
#define DISPLAY_ID_0			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x04)
#define DISPLAY_ID_1			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x05)
#define DISPLAY_ID_2			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x06)
#define DISPLAY_ID_3			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x07)

/* IO expander HW strap pins IO expander 2 */
#define PD_AIC_DETECT1			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x00)
#define PD_AIC_DETECT2			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x01)
/* Pin2 is named as PNP_NPNP_SKU in schematics but using this EC_EMUL_HW_STRAP */
#define EC_EMUL_HW_STRAP			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x02)
#define TIMEOUT_DISABLE			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x03)
#define SOC_VR_CNTRL			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x04)
#define EC_M_2_SSD_PLN			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x05)
#define VIRTUAL_BAT			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x06)
#define VIRTUAL_DOCK			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x07)
/* IO expander pin mapping is in hex decimal format instead of octal*/
/* Net name TP_ESPI_TESTCRD_DET */
#define THERM_STRAP			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x08)
#define KBC_SCROLL_LOCK			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x0D)

/* Net name TP_RETIMER_BYPASS_STRAP */
#define RETIMER_BYPASS			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x06)

/* Device instance names */
#define I2C_BUS_0			DT_NODELABEL(i2c_smb_0)
#define I2C_BUS_1			DT_NODELABEL(i2c_smb_1)
#if DT_NODE_HAS_STATUS(DT_INST(2, microchip_xec_i2c), okay)
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
#define BAT_LED2			DT_ALIAS(pwm_led0)
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
	((platformskutype == PLATFORM_MTL_P_SKUs) ? 90 : 90)
#define CHARGER_CURRENT_LIMIT2_PERCENTAGE \
	((platformskutype == PLATFORM_MTL_P_SKUs) ? 100 : 100)
#define CHARGER_AC_PROCHOT_PERCENTAGE \
	((platformskutype == PLATFORM_MTL_P_SKUs) ? 110 : 110)
#define CHARGER_FAST_CHARGING_CURRENT \
	((platformskutype == PLATFORM_MTL_P_SKUs) ? 0x1E14 : 0x2FF8)

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
#define CHARGER_CURRENT_LIMIT_DIVISOR 2U

/* Delay to match the output adaptor current ramp-up with the
 * system current
 */
#define SINK_FET_VOL_RAMPUP_DLY_MS 50U

/* Minimum Adapter power(Milli Watts) for proceeding with boot */
#define ADP_CRIT_POWERUP		26000

/* Configure the PD I2C addresses based on the board id */
#define TIPD_PORT_0_I2C_ADDR	((pd_i2c_addr_set == MTL_P_I2C_SET1) ? \
					 0x20U : \
					 0x22U)
#define TIPD_PORT_1_I2C_ADDR	((pd_i2c_addr_set == MTL_P_I2C_SET1) ? \
					 0x24U : \
					 0x26U)
#define TIPD_PORT_2_I2C_ADDR	((pd_i2c_addr_set == MTL_P_I2C_SET1) ? \
					 0x21U : \
					 0x23U)
#define TIPD_PORT_3_I2C_ADDR	((pd_i2c_addr_set == MTL_P_I2C_SET1) ? \
					 0x25U : \
					 0x27U)

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

#endif /* __MTL_P_MEC1501_H__ */
