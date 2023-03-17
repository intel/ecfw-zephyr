/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <soc.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/espi.h>
#include <zephyr/drivers/espi_saf.h>
#include "saf_spi_transaction.h"
#include "saf_spi_winbond.h"

#define W25Q_CAPACITY_BYTES (CONFIG_SAF_SPI_CAPACITY * 1024U * 1024U)

static struct saf_spi_transaction winbond_qspi_cmds[] = {
	[RD_STS1_CMD_INDEX] = {
		.buf = { READ_STATUS_1_OPCODE },
		.tx_len = 1,
		.rx_len = 1,
		.mode = SPI_LINES_SINGLE,
	},
	[RD_STS2_CMD_INDEX] = {
		.buf = { READ_STATUS_2_OPCODE },
		.tx_len = 1,
		.rx_len = 2,
		.mode = SPI_LINES_SINGLE,
	},
	[EN_RST_CMD_INDEX] = {
		.buf = { ENABLE_RESET_OPCODE },
		.tx_len = 1,
		.rx_len = 0,
	},
	[RST_CMD_INDEX] = {
		.buf = { RESET_OPCODE },
		.tx_len = 1,
		.rx_len = 0,
	},
	[FOUR_BYTE_CMD_INDEX] = {
		.buf = { FOUR_BYTE_ADDRESS_ENTER_OPCODE },
		.tx_len = 1,
		.rx_len = 0,
	},
	[WRITE_ENABLE_INDEX] = {
		.buf = { WRITE_ENABLE_OPCODE,  },
		.tx_len = 1,
		.rx_len = 0,
	},
	/* XIP register address 0x000006, new value 0xFE */
	[WRITE_NV_REGISTER_INDEX] = {
		.buf = { WRITE_VOLATILE_CFG_OPCODE, 0x00, 0x00, 0x06, 0xFE },
		.tx_len = 5,
		.rx_len = 0,
	},
	/* Buffer data smaller than tx_len, so driver transmit dummy clocks */
	[READ_NV_REGISTER_INDEX] = {
		.buf = { READ_VOLATILE_CFG_OPCODE, 0x00, 0x00, 0x06 },
		.tx_len = 5,
		.rx_len = 1,
	},
};


/* SAF bridge Windbond configuration
 *
 * SAF bridge translates eSPI flash requests into SPI bus transactions
 * hence need to know the vendor-specific commands for each of the following
 * transactions.
 *
 * Refer to espi_saf driver subystem for additional details.
 *
 */
static const struct espi_saf_flash_cfg flash_w25qxxx = {
#ifdef CONFIG_SOC_SERIES_MEC172X
	.version = MCHP_SAF_VER_2,
#endif
	.flashsz = W25Q_CAPACITY_BYTES,
	.opa = MCHP_SAF_OPCODE_REG_VAL(WRITE_ENABLE_OPCODE,
				       ERASE_SUSPEND_OPCODE,
				       ERASE_RESUME_OPCODE,
				       READ_STATUS_1_OPCODE),
	.opb = MCHP_SAF_OPCODE_REG_VAL(SECTOR_ERASE_OPCODE,
				       BLOCK_ERASE_32K_OPCODE,
				       BLOCK_ERASE_64K_OPCODE,
				       PAGE_PROGRAM_OPCODE),
	.opc = MCHP_SAF_OPCODE_REG_VAL(FAST_READ_IO_OPCODE,
				       EXIT_QPI_OPCODE,
				       CONTINUOUS_MODE_OPCODE,
				       READ_STATUS_2_OPCODE),
#ifdef CONFIG_SOC_SERIES_MEC172X
	.opd = MCHP_SAF_OPCODE_REG_VAL(POWER_DOWN,
				       RELEASE_POWER_DOWN,
				       SAF_RPMC_STATUS_DATA_OPCODE,
				       0U),
#endif
	.cont_prefix = 0U,
	.cs_cfg_descr_ids = MCHP_CS0_CFG_DESCR_IDX_REG_VAL,
	.poll2_mask = SAF_POLL_MASK,
	.flags = SAF_FLAGS,
	.descr = {
		SAF_DESCR_CM_RD_D0,
		SAF_DESCR_CM_RD_D1,
		SAF_DESCR_CM_RD_D2,
		SAF_DESCR_ENTER_CM_D0,
		SAF_DESCR_ENTER_CM_D1,
		SAF_DESCR_ENTER_CM_D2
	}
};

static const struct espi_saf_cfg saf_cfg = {
	.nflash_devices = CONFIG_SAF_SPI_DEVICES_COUNT,
	.hwcfg = {
#ifdef CONFIG_SOC_SERIES_MEC172X
		.version = MCHP_SAF_VER_2,
#else
		.qmspi_cs_timing = MCHP_SAF_QMSPI_CS_TIMING,
		.qmspi_cpha = MCHP_SAF_HW_CFG_FLAG_CPHA,
#endif
		.flags = 0,
		.generic_descr = {
			SAF_DESCR_EXIT_CM12,
			SAF_DESCR_EXIT_CM13,
			MCHP_SAF_POLL_DESCR14,
			MCHP_SAF_POLL_DESCR15
		},
		.tag_map = {
			MCHP_SAF_TAG_MAP0_DFLT,
			MCHP_SAF_TAG_MAP1_DFLT,
			MCHP_SAF_TAG_MAP2_DFLT
			},
	},
	.flash_cfgs = (struct espi_saf_flash_cfg *)&flash_w25qxxx
};

const struct espi_saf_cfg *windbond_saf_cfg(void)
{
	return &saf_cfg;
}

struct saf_spi_transaction *windbond_qspi_cmd(enum saf_command_index command)
{
	__ASSERT(command >= ARRAY_SIZE(winbond_qspi_cmds),
		 "Invalid index");
	return &winbond_qspi_cmds[command];
}
