/*
 * Copyright (c) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include "mec150x_pin.h"
#include "common_mec1501.h"

#ifndef __MECC172X_AIC_ON_ADL_P_H__
#define __MECC172X_AIC_ON_ADL_P_H__

extern uint8_t platformskutype;

#define KSC_PLAT_NAME                   "ADLPMECC"

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

#define CPU_C10_GATE			EC_GPIO_023
#define PM_BATLOW			EC_DUMMY_GPIO_HIGH
#define TYPEC_ALERT_2			EC_DUMMY_GPIO_HIGH
#define TYPEC_ALERT_1			EC_DUMMY_GPIO_HIGH
#define PS2_KB_DATA			EC_DUMMY_GPIO_HIGH
#define KBC_NUM_LOCK			EC_DUMMY_GPIO_HIGH
#define KBC_CAPS_LOCK			EC_DUMMY_GPIO_HIGH
#define HOME_BUTTON			EC_DUMMY_GPIO_HIGH
#define EC_PWRBTN_LED			EC_DUMMY_GPIO_LOW
#define DG2_PRESENT			EC_DUMMY_GPIO_LOW
#define EC_PG3_EXIT			EC_DUMMY_GPIO_LOW
#define PS2_MB_DATA			EC_DUMMY_GPIO_HIGH
#define PEG_RTD3_COLD_MOD_SW_R		EC_DUMMY_GPIO_LOW
/* No pin assigned for this in MECC card. Make SAF the default */
#define G3_SAF_DETECT			EC_DUMMY_GPIO_HIGH

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
#define KBC_SCROLL_LOCK			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x0D)

/* Net name TP_RETIMER_BYPASS_STRAP */
#define RETIMER_BYPASS			EC_GPIO_PORT_PIN(EC_EXP_PORT_2, 0x06)
#endif /* __MECC172X_AIC_ON_ADL_P_H__ */
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
