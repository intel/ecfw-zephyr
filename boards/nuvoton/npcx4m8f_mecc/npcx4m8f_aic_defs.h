/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __NPCX4M8F_AIC_DEFS_H__
#define __NPCX4M8F_AIC_DEFS_H__

#define RSMRST_PWRGD			((boot_mode_maf == 1) ? \
					 RSMRST_PWRGD_MAF_P : \
					 RSMRST_PWRGD_G3SAF_P)

#define PM_RSMRST			((boot_mode_maf == 1) ? \
					 PM_RSMRST_MAF_P : \
					 PM_RSMRST_G3SAF_P)

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
#define I2C_BUS_0			DT_ALIAS(i2c0)

#if DT_NODE_HAS_STATUS(DT_ALIAS(i2c1), okay)
#define I2C_BUS_1			DT_ALIAS(i2c1)
#endif

#if DT_NODE_HAS_STATUS(DT_ALIAS(i2c2), okay)
#define I2C_BUS_2			DT_ALIAS(i2c2)
#define I2C_PORT_NVME			I2C_2
#endif

#define PS2_KEYBOARD			DT_NODELABEL(ps2_channel0)
#define PS2_MOUSE			DT_NODELABEL(ps2_channel0)
#define ESPI_0				DT_NODELABEL(espi0)
#define ESPI_SAF_0			DT_NODELABEL(espi_saf0)
#define SPI_0				DT_NODELABEL(spi0)
#define ADC_CH_BASE			DT_NODELABEL(adc1)
#define PECI_0_INST			DT_NODELABEL(peci0)
#define WDT_0				DT_NODELABEL(twd0)
#define KSCAN_MATRIX			DT_NODELABEL(kscan0)

#endif /* __NPCX4M8F_AIC_DEFS_H__ */
