/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SAF_SPI_WINBOND_H__
#define __SAF_SPI_WINBOND_H__

/* Winbond SPI commands */
#define PAGE_PROGRAM_OPCODE            0x02U
#define READ_STATUS_1_OPCODE           0x05U
#define WRITE_ENABLE_OPCODE            0x06U
#define SECTOR_ERASE_OPCODE            0x20U
#define READ_STATUS_2_OPCODE           0x35U
#define BLOCK_ERASE_32K_OPCODE         0x52U
#define ENABLE_RESET_OPCODE            0x66U
#define ERASE_SUSPEND_OPCODE           0x75U
#define ERASE_RESUME_OPCODE            0x7AU
#define RESET_OPCODE                   0x99U
#define UNKNOWN_OPCODE                 0xA5U
#define FOUR_BYTE_ADDRESS_ENTER_OPCODE 0xB7U
#define BLOCK_ERASE_64K_OPCODE         0xD8U
#define FAST_READ_QUAD_IO_3BYTE_OPCODE 0xEBU
#define FAST_READ_QUAD_IO_4BYTE_OPCODE 0xECU
#define EXIT_QPI_OPCODE                0xFFU

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

