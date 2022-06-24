/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SAF_SPI_WINBOND_H__
#define __SAF_SPI_WINBOND_H__

#include "spi_winbond_opcodes.h"

/* Adjust descriptors based on SPI capacity to simplify SAF structure */
#if (CONFIG_SAF_SPI_CAPACITY == 32)
#define SAF_POLL_MASK		(MCHP_W25Q256_POLL2_MASK)
#define SAF_FLAGS		(MCHP_FLASH_FLAG_ADDR32)
#define SAF_DESCR_CM_RD_D0	MCHP_W25Q256_CM_RD_D0
#define SAF_DESCR_CM_RD_D1	MCHP_W25Q256_CM_RD_D1
#define SAF_DESCR_CM_RD_D2	MCHP_W25Q256_CM_RD_D2
#define SAF_DESCR_ENTER_CM_D0	MCHP_W25Q256_ENTER_CM_D0
#define SAF_DESCR_ENTER_CM_D1	MCHP_W25Q256_ENTER_CM_D1
#define SAF_DESCR_ENTER_CM_D2	MCHP_W25Q256_ENTER_CM_D2
#define SAF_DESCR_EXIT_CM12	MCHP_SAF_EXIT_CM_DESCR12
#define SAF_DESCR_EXIT_CM13	MCHP_SAF_EXIT_CM_DESCR13
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

