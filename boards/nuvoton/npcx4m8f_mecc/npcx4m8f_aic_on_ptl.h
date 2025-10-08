/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include "npcx_pin.h"
#include "common_npcx.h"
#include "npcx4m8f_aic_defs.h"

#ifndef __NPCX4M8F_AIC_ON_PTL_P_H__
#define __NPCX4M8F_AIC_ON_PTL_P_H__

extern uint8_t platformskutype;
extern uint8_t pd_i2c_addr_set;

#define KSC_PLAT_NAME                   "PTLNPCX4"

#define PLATFORM_DATA(x, y)  ((x) | ((y) << 8))

#if defined(CONFIG_NVME_RECOVERY) && !defined(CONFIG_I3C)
#define I2C_PORT_NVME		I2C_2
#define NVME_I2C_SPEED		I2C_SPEED_FAST
#endif

/* In PTL UH board id is 6-bits */
#define BOARD_ID_MASK        0x003Fu
#define BOM_ID_MASK          0x01C0u
#define FAB_ID_MASK          0x0600u
#define HW_STRAP_MASK        0xF800u
#define HW_ID_MASK           (FAB_ID_MASK|BOM_ID_MASK|BOARD_ID_MASK)
#define BOARD_ID_OFFSET      0u
#define BOM_ID_OFFSET        6u
#define FAB_ID_OFFSET        9u
#define HW_STRAP_OFFSET      11u

/* Support PTL UH skus */
enum platform_skus {
	PLATFORM_PTL_UH_SKUs,
};

/* PTL UH PD I2C Address Group*/
enum i2c_addr_set {
	PTL_UH_I2C_SET1 = 0,
	PTL_UH_I2C_SET2 = 1,
};

/* Support board ids */
#define BRD_ID_PTL_UH_LP5x_T3_ERB		0x09u
#define BRD_ID_PTL_UH_LP5x_T3_RVP1		0x01u
#define BRD_ID_PTL_UH_LP5x_CAMM_DTBT_RVP2		0x02u
#define BRD_ID_PTL_UH_LP5x_T4_RVP3		0x03u
#define BRD_ID_PTL_UH_DDR5_T3_RVP4		0x04u

/* I2C addresses */
#define EEPROM_DRIVER_I2C_ADDR          0x50
#define IO_EXPANDER_0_I2C_ADDR          0x22

/* Signal to gpio mapping for NPCX4 200-pin card for PTL UH */
#define PWRBTN_EC_IN_N			EC_GPIO_000

#define SMC_LID				EC_GPIO_001

#define PM_SLP_SUS			EC_GPIO_007

#define BATT_ID_N			EC_GPIO_013
#define PM_SLP_S0_CS			EC_DUMMY_GPIO_HIGH

#define STD_ADPT_CNTRL_GPIO		EC_GPIO_023

/* Cannot support PSON since same EC pins are routed for PG3 */
#define PS_ON_OUT			EC_DUMMY_GPIO_HIGH
#define PS_ON_IN_EC_N			EC_GPIO_043

#define ESPI_RESET_MAF			EC_GPIO_054

#define PROCHOT				EC_GPIO_060
#define PCH_PWROK			EC_GPIO_061
#define STD_ADP_PRSNT			EC_GPIO_062
#define ALL_SYS_PWRGD			EC_GPIO_063
#define PM_PWRBTN			EC_GPIO_066

#define RSMRST_PWRGD_MAF_P		EC_GPIO_072
#define RSMRST_PWRGD_G3SAF_P		EC_GPIO_072
#define FAN_PWR_DISABLE_N		EC_GPIO_073
/* This is routed to EC_GPIO_074 which is 3.3V logic, while RVP is 1.8V */
#define EC_SLATEMODE_HALLOUT_SNSR_R	EC_DUMMY_GPIO_HIGH
#define WAKE_SCI			EC_GPIO_076

/* In PTL, this is already accesible via IO expander no need to redefine
 *
 * #define VIRTUAL_BAT			EC_GPIO_080
 */

#define PM_BATLOW			EC_GPIO_112

/* In PTL, this is already accesible via IO expander no need to redefine
 *
 * #define KBC_SCROLL_LOCK		EC_GPIO_117
 */
#define KBC_NUM_LOCK			EC_GPIO_120
#define KBD_BKLT_CTRL			EC_DUMMY_GPIO_HIGH
#define RECOVERY_INDICATOR_N		EC_GPIO_122
#define KBC_CAPS_LOCK			EC_GPIO_124
#define PM_DS3				EC_GPIO_041

#define SYS_PWROK			EC_GPIO_130
#define BC_ACOK				EC_GPIO_132

#define TYPEC_EC_SMBUS_ALERT_0_R	EC_GPIO_150
#define VOL_DOWN			EC_GPIO_151
#ifdef CONFIG_PCH_DEBUG_VIA_VOLUME_UP_SIMULATION
#define EC_PCH_DEBUG			EC_GPIO_152
#define VOL_UP				EC_DUMMY_GPIO_HIGH
#else
#define EC_PCH_DEBUG			EC_DUMMY_GPIO_HIGH
#define VOL_UP				EC_GPIO_152
#endif

#define EC_PG3_EXIT			EC_GPIO_153
#define PM_RSMRST_MAF_P			EC_GPIO_105
#define PM_RSMRST_G3SAF_P		EC_GPIO_105

#define EC_PWRBTN_LED			EC_DUMMY_GPIO_HIGH
#define CPU_C10_GATE			EC_DUMMY_GPIO_HIGH
#define HOME_BUTTON			EC_DUMMY_GPIO_HIGH
#define DG2_PRESENT			EC_DUMMY_GPIO_LOW
#define PEG_RTD3_COLD_MOD_SW_R		EC_DUMMY_GPIO_LOW

#define PM_BAT_STATUS_LED1		EC_DUMMY_GPIO_HIGH
#define PECI_OVER_ESPI			EC_DUMMY_GPIO_HIGH

/* IO expander HW strap pins IO expander 1 */
#define SPD_PRSNT			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x03)
#define DISPLAY_ID_0			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x04)
#define DISPLAY_ID_1			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x05)
#define DISPLAY_ID_2			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x06)
#define DISPLAY_ID_3			EC_GPIO_PORT_PIN(EC_EXP_PORT_1, 0x07)

#define MOD_TCSS1_DETECT		EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x00)
#define MOD_TCSS2_DETECT		EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x01)
/* Pin2 is named as PNP_NPNP_SKU in schematics but using this G3_SAF_DETECT
 * PTL has G3_SAF_DETECT but is not accessible from MECC card, using SW1W1.2 instead
 */
#define G3_SAF_DETECT			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x02)
#define TIMEOUT_DISABLE			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x03)
#define SOC_VR_CNTRL			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x04)
#define EC_M_2_SSD_PLN			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x05)
#define VIRTUAL_BAT			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x06)
#define VIRTUAL_DOCK			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x07)

/* IO expander pin mapping is in hex decimal format instead of octal*/

/* Pin4 is named as TP_ESPI_TESTCRD_DET in schematics but using this NPCX_CAF_EMULATION */
#define NPCX_CAF_EMULATION		EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x08)
#define THERM_STRAP			EC_DUMMY_GPIO_HIGH
#define PD_AIC_DETECT_SLOT_ID		EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x0A)
#define KBC_SCROLL_LOCK			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x0D)
#define C10_GATE_LED			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x0E)
#define CS_INDICATE_LED			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x0F)

#define CHARGER_CURRENT_LIMIT1_PERCENTAGE 90
#define CHARGER_CURRENT_LIMIT2_PERCENTAGE 100
#define CHARGER_AC_PROCHOT_PERCENTAGE 110
#define CHARGER_FAST_CHARGING_CURRENT 0x2FF8

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

/* Change the BOOT mode name to APP1 for TOMCAT PD Controller*/
#define TIPD_BOOT_COMPLETE_MODE	"APP1"

/* Configure the PD I2C addresses based on the board id */
#define TIPD_PORT_0_I2C_ADDR	((pd_i2c_addr_set == PTL_UH_I2C_SET1) ? \
					 0x24U : \
					 0x20U)
#define TIPD_PORT_1_I2C_ADDR	((pd_i2c_addr_set == PTL_UH_I2C_SET1) ? \
					 0x20U : \
					 0x21U)
#define TIPD_PORT_2_I2C_ADDR	((pd_i2c_addr_set == PTL_UH_I2C_SET1) ? \
					 0x21U : \
					 0x22U)
#define TIPD_PORT_3_I2C_ADDR	((pd_i2c_addr_set == PTL_UH_I2C_SET1) ? \
					 0x22U : \
					 0x26U)

/* Configure the UCSI Connector number based on the board id */
#define UCSI_CONNECTOR_NUM_PORT_0	((pd_i2c_addr_set == PTL_UH_I2C_SET1) ? \
					0x02U : \
					0x01U)
#define UCSI_CONNECTOR_NUM_PORT_1	((pd_i2c_addr_set == PTL_UH_I2C_SET1) ? \
					0x01U : \
					0x01U)
#define UCSI_CONNECTOR_NUM_PORT_2	((pd_i2c_addr_set == PTL_UH_I2C_SET1) ? \
					0x01U : \
					0x01U)
#define UCSI_CONNECTOR_NUM_PORT_3	((pd_i2c_addr_set == PTL_UH_I2C_SET1) ? \
					0x01U : \
					0x02U)

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
#endif /* __NPCX4M8F_AIC_DEFS_H__ */
