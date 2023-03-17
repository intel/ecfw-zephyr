/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MEC172X_AIC_DEFS_H__
#define __MEC172X_AIC_DEFS_H__

/* Signal to gpio mapping for MEC172x card */

/* Following blue wires are required
 */

#ifdef CONFIG_MEC172X_AIC_HW_REV2
#define PROCHOT				EC_GPIO_243
#else
#define PROCHOT				EC_GPIO_002
#endif

#ifdef CONFIG_MEC172X_AIC_HW_REV2
#define RSMRST_PWRGD_G3SAF_P		EC_GPIO_011
#else
#define RSMRST_PWRGD_G3SAF_P		EC_GPIO_221
#endif

#define RSMRST_PWRGD_MAF_P		EC_GPIO_227
#define RSMRST_PWRGD			((boot_mode_maf == 1) ? \
					 RSMRST_PWRGD_MAF_P : \
					 RSMRST_PWRGD_G3SAF_P)

/* Mapping in MEC172x HW revision #1 for power button conflicts with SPI IO3
 * This in addition to SHD_IO2 and SHD_IO3 not routed to flash
 * prevents SAF Quad mode. This is fixed in MEC172x HW revision #2.
 */
#ifdef CONFIG_MEC172X_AIC_HW_REV2
#define PM_PWRBTN			EC_GPIO_245
#else
#define PM_PWRBTN			EC_GPIO_016
#endif

#define VOL_UP				EC_GPIO_036
#define STD_ADP_PRSNT			EC_GPIO_043
#define WAKE_SCI			EC_GPIO_051
#define PM_RSMRST_G3SAF_P		EC_GPIO_054
#define PM_RSMRST_MAF_P			EC_GPIO_055
#define PM_RSMRST			((boot_mode_maf == 1) ? \
					 PM_RSMRST_MAF_P : PM_RSMRST_G3SAF_P)

#define ALL_SYS_PWRGD			EC_GPIO_057

/* We poll this GPIO in MAF mode in order to sense the input signal.
 * This pin was already configured in pinmux as ALT mode 1 NOT GPIO
 */
#define ESPI_RESET_MAF			EC_GPIO_061
#define PCH_PWROK			EC_GPIO_106

#ifdef CONFIG_MEC172X_AIC_HW_REV2
#define FAN_PWR_DISABLE_N		EC_GPIO_201
#else
#define FAN_PWR_DISABLE_N		EC_GPIO_141
#endif

#define BC_ACOK				EC_GPIO_156
#define SYS_PWROK			EC_GPIO_202
/* Not used across all boards */
#define BATT_ID_N			EC_GPIO_206
#define VCCST_PWRGD			EC_GPIO_207
#define SMC_LID				EC_GPIO_226
#define KBD_BKLT_CTRL			EC_GPIO_142
#define EC_SLATEMODE_HALLOUT_SNSR_R	EC_GPIO_222
#define STD_ADPT_CNTRL_GPIO		EC_GPIO_244

/* PM_SLP_SUS is mapped to GPIO227 in MEC172x HW rev #1
 * This causes re-configuration since this is also RSMRST_PWRGD
 * in MAF mode.
 */
#ifdef CONFIG_MEC172X_AIC_HW_REV2
#define PM_SLP_SUS			EC_GPIO_067
#else
#define PM_SLP_SUS			EC_DUMMY_GPIO_HIGH
#endif
#define PWRBTN_EC_IN_N			EC_GPIO_246
#define VOL_DOWN			EC_GPIO_254

#ifdef CONFIG_MEC172X_AIC_HW_REV2
/* VCI_IN0 */
#define PM_SLP_S0_CS			EC_GPIO_154
#else
#define PM_SLP_S0_CS			EC_DUMMY_GPIO_HIGH
#endif

#ifdef CONFIG_MEC172X_AIC_HW_REV2
/* VCI_OUT2 */
#define PM_DS3				EC_GPIO_025
#else
#define PM_DS3				EC_DUMMY_GPIO_HIGH
#endif

/* Button/Switch Initial positions */
#define PWR_BTN_INIT_POS		1
#define VOL_UP_INIT_POS			1
#define VOL_DN_INIT_POS			1
#define LID_INIT_POS			1
#define HOME_INIT_POS			1
#define SLATEMODE_INIT_POS		1
#define IOEXP_INIT_POS			1
#define VIRTUAL_DOCK_INIT_POS		1
#define VIRTUAL_BAT_INIT_POS		1

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
#define KSCAN_MATRIX			DT_NODELABEL(kscan0)

#endif /* __MEC172X_AIC_DEFS_H__ */
