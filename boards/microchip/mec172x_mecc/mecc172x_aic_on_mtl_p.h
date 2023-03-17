/*
 * Copyright (c) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include "mec150x_pin.h"
#include "common_mec1501.h"

#ifndef __MECC172X_ON_MTL_P_H__
#define __MECC172X_ON_MTL_P_H__

extern uint8_t platformskutype;
extern uint8_t pd_i2c_addr_set;

#define KSC_PLAT_NAME                   "MTLPMECC"

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

#define PM_BATLOW			EC_GPIO_023
#define PM_BAT_STATUS_LED1		EC_DUMMY_GPIO_HIGH
#define TYPEC_EC_SMBUS_ALERT_0_R	EC_GPIO_143

/* Only for MTL-P the G3_SAF HW strap is not in the expander_cfg
 * Cannot use HW strap but dummy GPIO to decide G3/SAF
 * Default SAF
 */
#define G3_SAF_DETECT			EC_DUMMY_GPIO_HIGH
#define TYPEC_ALERT_2			EC_DUMMY_GPIO_HIGH
#define TYPEC_ALERT_1			EC_DUMMY_GPIO_HIGH
#define KBC_SCROLL_LOCK			EC_DUMMY_GPIO_HIGH
#define KBC_NUM_LOCK			EC_DUMMY_GPIO_HIGH
#define KBC_CAPS_LOCK			EC_DUMMY_GPIO_HIGH
#define PM_SLP_S0_N			EC_DUMMY_GPIO_HIGH
#define EC_PG3_EXIT			EC_DUMMY_GPIO_LOW
#define EC_PWRBTN_LED			EC_DUMMY_GPIO_LOW
#define CPU_C10_GATE			EC_DUMMY_GPIO_HIGH
#define HOME_BUTTON			EC_DUMMY_GPIO_HIGH
#define DG2_PRESENT			EC_DUMMY_GPIO_LOW
#define PEG_RTD3_COLD_MOD_SW_R		EC_DUMMY_GPIO_LOW

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
/* Net name TP_RETIMER_BYPASS_STRAP */
#define RETIMER_BYPASS			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x06)

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
 * JJ - Major Version
 * M - Minor Version
 * N - Sub Minor Version
 */
#define TIPD_UCSI_MAJOR_VERSION 0x02
#define TIPD_UCSI_MINOR_VERSION 0x0
#define TIPD_UCSI_SUB_MINOR_VERSION 0x0


#endif /* __MECC172X_ON_MTL_P_H__ */
