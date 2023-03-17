/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MEC15XX_AIC_DEFS_H__
#define __MEC15XX_AIC_DEFS_H__

/* Signal to gpio mapping for MEC15xx card */


/* Following blue wires are required
 * PWRBTN_EC_OUT - from MECC TP65 to MECC J50.11 (GPIO250)
 * PWRBTN_EC_IN  - from MECC TP64 to MECC J50.8  (GPIO163)
 * PM_SLP_SUS    - from MECC TP55 to MECC J50.16
 * SYS_PWROK     - from MECC J51.15 (GPIO034) to RVP
 * PM_DS3        - from MECC J50.4  (GPIO165) to RVP
 * Simulated LEDS
 * SCROLL_LOCK   - J50.12 (GPIO101)
 * NUM_LOCK      - J50.10 (GPIO102)
 * CAPS_LOCK     - J50.14 (GPIO172)
 * 48MHZ TST_CLK - JP20.16
 */

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
#define KBC_SCROLL_LOCK			EC_GPIO_101
#define KBC_NUM_LOCK			EC_GPIO_102
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
#define KBC_CAPS_LOCK			EC_GPIO_172
#define BATT_ID				EC_GPIO_206
#define SMC_LID				EC_GPIO_226
#define PM_SLP_S0_N			EC_GPIO_243
#define PM_PWRBTN			EC_GPIO_250
#define VOL_DOWN			EC_GPIO_254

/* Button/Switch Initial positions */
#define PWR_BTN_INIT_POS		1
#define VOL_UP_INIT_POS			1
#define VOL_DN_INIT_POS			1
#define LID_INIT_POS			1
#define HOME_INIT_POS			1
#define VIRTUAL_DOCK_INIT_POS		1
#define VIRTUAL_BAT_INIT_POS		1

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
#define KSCAN_MATRIX			DT_NODELABEL(kscan0)

#endif /* __MEC15XX_AIC_DEFS_H__ */
