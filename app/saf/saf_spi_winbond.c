/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <soc.h>
#include <drivers/spi.h>
#include <drivers/espi.h>
#include <drivers/espi_saf.h>
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
	.flashsz = W25Q_CAPACITY_BYTES,
	.opa = MCHP_SAF_OPCODE_REG_VAL(WRITE_ENABLE_OPCODE,
				       ERASE_SUSPEND_OPCODE,
				       ERASE_RESUME_OPCODE,
				       READ_STATUS_1_OPCODE),
	.opb = MCHP_SAF_OPCODE_REG_VAL(SECTOR_ERASE_OPCODE,
				       BLOCK_ERASE_32K_OPCODE,
				       BLOCK_ERASE_64K_OPCODE,
				       PAGE_PROGRAM_OPCODE),
#if (CONFIG_SAF_SPI_CAPACITY == 32)
	.opc = MCHP_SAF_OPCODE_REG_VAL(FAST_READ_QUAD_IO_4BYTE_OPCODE,
				       EXIT_QPI_OPCODE,
				       UNKNOWN_OPCODE,
				       READ_STATUS_2_OPCODE),
#elif (CONFIG_SAF_SPI_CAPACITY == 16)
	.opc = MCHP_SAF_OPCODE_REG_VAL(FAST_READ_QUAD_IO_3BYTE_OPCODE,
				       EXIT_QPI_OPCODE,
				       UNKNOWN_OPCODE,
				       READ_STATUS_2_OPCODE),
#else
#pragma error "Unsupported SPI capacity"
#endif
	.cont_prefix = 0U,
	.cs_cfg_descr_ids = MCHP_CS0_CFG_DESCR_IDX_REG_VAL,
#if (CONFIG_SAF_SPI_CAPACITY == 32)
	.poll2_mask = MCHP_W25Q256_POLL2_MASK,
	.flags = MCHP_FLASH_FLAG_ADDR32,
	.descr = {
		MCHP_W25Q256_CM_RD_D0,
		MCHP_W25Q256_CM_RD_D1,
		MCHP_W25Q256_CM_RD_D2,
		MCHP_W25Q256_ENTER_CM_D0,
		MCHP_W25Q256_ENTER_CM_D1,
		MCHP_W25Q256_ENTER_CM_D2
	}
#elif (CONFIG_SAF_SPI_CAPACITY == 16)
	.poll2_mask = MCHP_W25Q128_POLL2_MASK,
	.flags = 0,
	.descr = {
		MCHP_W25Q128_CM_RD_D0,
		MCHP_W25Q128_CM_RD_D1,
		MCHP_W25Q128_CM_RD_D2,
		MCHP_W25Q128_ENTER_CM_D0,
		MCHP_W25Q128_ENTER_CM_D1,
		MCHP_W25Q128_ENTER_CM_D2
	}
#else
#pragma "Unsupported SPI capacity"
#endif
};

static const struct espi_saf_cfg saf_cfg = {
	.nflash_devices = CONFIG_SAF_SPI_DEVICES_COUNT,
	.hwcfg = {
		.qmspi_freq_hz = CONFIG_SAF_SPI_FREQ_MHZ,
		.qmspi_cs_timing = MCHP_SAF_QMSPI_CS_TIMING,
		.qmspi_cpha = MCHP_SAF_HW_CFG_FLAG_CPHA,
		.flags = 0,
		.generic_descr = {
			MCHP_SAF_EXIT_CM_DESCR12,
			MCHP_SAF_EXIT_CM_DESCR13,
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
