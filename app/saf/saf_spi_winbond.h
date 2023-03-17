/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SAF_SPI_WINBOND_H__
#define __SAF_SPI_WINBOND_H__

#include "spi_winbond_opcodes.h"

/* Adjust descriptors based on SPI capacity to simplify SAF structure */
#if DT_NODE_HAS_STATUS(DT_NODELABEL(spi0), disabled)
#pragma error "spi hw block not enabled"
#else
#define DT_SPI_INST	DT_NODELABEL(spi0)
#if DT_PROP(DT_SPI_INST, lines) == 4
#define FAST_READ_IO_OPCODE            FAST_READ_QUAD_IO_OPCODE
#else
#define FAST_READ_IO_OPCODE            FAST_READ_DUAL_IO_OPCODE
#endif /* DT_PROP(DT_SPI_INST, lines) == 4 */
#endif /* DT_NODE_HAS_STATUS(DT_NODELABEL(spi0), disabled) */

/*
 * Dual mode adjustments
 * CM_D0  [Command op code ]		1 byte
 * CM_D1  [4-byte Address ] [PEM=A5]	5 bytes
 * CM_D2  [1-byte dummy read]		1 byte
 *
 * RD_D0  [4-byte Address ] [PEM=A5]	5 bytes
 * RD_D1  No dummy clocks
 * RD_D2  [1-byte dummy read]		0
 *
 * For each of the Continuous Mode Read chains, the SAF Bridge dynamically
 * overrides the length field of the descriptor associated with the input
 * of the flash data. This Descriptor is identified by another SAF
 * Bridge Descriptors register field (CONT_SIZE).
 */
#define DUAL_SAF_EXIT_CM_DESCR12 ((MCHP_SAF_EXIT_CM_DESCR12 & \
				  ~MCHP_QMSPI_C_IFM_4X) | MCHP_QMSPI_C_IFM_2X)

#define DUAL_SAF_EXIT_CM_DESCR13 ((MCHP_SAF_EXIT_CM_DESCR13 & \
				  ~MCHP_QMSPI_C_IFM_4X) | MCHP_QMSPI_C_IFM_2X)


#define DUAL_W25Q256_CM_RD_D0 ((MCHP_W25Q256_CM_RD_D0 & \
				~MCHP_QMSPI_C_IFM_4X) | MCHP_QMSPI_C_IFM_2X)

#define DUAL_W25Q256_CM_RD_D2 ((MCHP_W25Q256_CM_RD_D2 & \
				~MCHP_QMSPI_C_IFM_4X) | MCHP_QMSPI_C_IFM_2X)

#define DUAL_W25Q256_ENTER_CM_D1 ((MCHP_W25Q256_ENTER_CM_D1 & \
				~MCHP_QMSPI_C_IFM_4X) | MCHP_QMSPI_C_IFM_2X)

#define DUAL_W25Q256_ENTER_CM_D2 ((MCHP_W25Q128_ENTER_CM_D2 & \
				  ~MCHP_QMSPI_C_IFM_4X) | MCHP_QMSPI_C_IFM_2X)

#define SAF_RPMC_STATUS_DATA_OPCODE	0U

/* Capacity adjustments */
#if (CONFIG_SAF_SPI_CAPACITY == 32)
#define SAF_POLL_MASK		(MCHP_W25Q256_POLL2_MASK)

#if DT_PROP(DT_SPI_INST, lines) == 4
#define SAF_FLAGS		(MCHP_FLASH_FLAG_ADDR32)
#define SAF_DESCR_CM_RD_D0	MCHP_W25Q256_CM_RD_D0
#define SAF_DESCR_CM_RD_D1	MCHP_W25Q256_CM_RD_D1
#define SAF_DESCR_CM_RD_D2	MCHP_W25Q256_CM_RD_D2
#define SAF_DESCR_ENTER_CM_D0	MCHP_W25Q256_ENTER_CM_D0
#define SAF_DESCR_ENTER_CM_D1	MCHP_W25Q256_ENTER_CM_D1
#define SAF_DESCR_ENTER_CM_D2	MCHP_W25Q256_ENTER_CM_D2
#define SAF_DESCR_EXIT_CM12	MCHP_SAF_EXIT_CM_DESCR12
#define SAF_DESCR_EXIT_CM13	MCHP_SAF_EXIT_CM_DESCR13
#elif DT_PROP(DT_SPI_INST, lines) == 2
#define SAF_FLAGS		(MCHP_FLASH_FLAG_ADDR32 | \
				MCHP_FLASH_CONT_READ_NO_DUMMY_CLK)
#define SAF_DESCR_CM_RD_D0	DUAL_W25Q256_CM_RD_D0
#define SAF_DESCR_CM_RD_D1	MCHP_W25Q256_CM_RD_D1
#define SAF_DESCR_CM_RD_D2	DUAL_W25Q256_CM_RD_D2
#define SAF_DESCR_ENTER_CM_D0	MCHP_W25Q256_ENTER_CM_D0
#define SAF_DESCR_ENTER_CM_D1	DUAL_W25Q256_ENTER_CM_D1
#define SAF_DESCR_ENTER_CM_D2	DUAL_W25Q256_ENTER_CM_D2
#define SAF_DESCR_EXIT_CM12	DUAL_SAF_EXIT_CM_DESCR12
#define SAF_DESCR_EXIT_CM13	DUAL_SAF_EXIT_CM_DESCR13
#else
#pragma "Unsupported SPI IO mode"
#endif /* DT_PROP(DT_SPI_INST, lines) == 4 */

#elif (CONFIG_SAF_SPI_CAPACITY == 16)
#define SAF_POLL_MASK		(MCHP_W25Q128_POLL2_MASK)
#define SAF_FLAGS		0
#define SAF_DESCR_CM_RD_D0	MCHP_W25Q128_CM_RD_D0
#define SAF_DESCR_CM_RD_D1	MCHP_W25Q128_CM_RD_D1
#define SAF_DESCR_CM_RD_D2	MCHP_W25Q128_CM_RD_D2
#define SAF_DESCR_ENTER_CM_D0	MCHP_W25Q128_ENTER_CM_D0
#define SAF_DESCR_ENTER_CM_D1	MCHP_W25Q128_ENTER_CM_D1
#define SAF_DESCR_ENTER_CM_D2	MCHP_W25Q128_ENTER_CM_D2
#define SAF_DESCR_EXIT_CM12	MCHP_SAF_EXIT_CM_DESCR12
#define SAF_DESCR_EXIT_CM13	MCHP_SAF_EXIT_CM_DESCR13
#else
#pragma "Unsupported SPI capacity"
#endif /* CONFIG_SAF_SPI_CAPACITY */

/**
 * @brief Return SAF configuration structure for Winbond SPI devices.
 *
 * @return espi saf configuration. Refer to eSPI SAF driver header.
 */
const struct espi_saf_cfg *windbond_saf_cfg(void);

/**
 * @brief Return SPI transaction details for Winbond SPI devices.
 *
 * @param the desired logical command.
 *
 * @return spi_transaction item. Refer to spi_transaction.h.
 */
struct saf_spi_transaction *windbond_qspi_cmd(enum saf_command_index command);

#endif /* __SAF_SPI_WINBOND_H__ */

