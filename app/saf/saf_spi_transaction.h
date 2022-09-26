/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SAF_SPI_TRANSACTION_H__
#define __SAF_SPI_TRANSACTION_H__

#define SAF_SPI_TRANS_MAX_PAYLOAD_SIZE  6U

/*
 * The SAFS Bridge assumes SPI flash device in certain state.
 * Below are the operations that need to be defined to enter such state
 */

/**
 * @brief SPI logical command needed to map to vendor-specific opcodes
 */
enum saf_command_index {
	RD_STS1_CMD_INDEX,
	RD_STS2_CMD_INDEX,
	EN_RST_CMD_INDEX,
	RST_CMD_INDEX,
	/* Devices of 32MB/256Mbit must also support a command to enter 4-byte
	 * address mode
	 */
	FOUR_BYTE_CMD_INDEX,
	WRITE_ENABLE_INDEX,
	WRITE_NV_REGISTER_INDEX,
	READ_NV_REGISTER_INDEX,
};

/**
 * @brief Describes a SPI transaction.
 */
struct saf_spi_transaction {
	uint8_t buf[SAF_SPI_TRANS_MAX_PAYLOAD_SIZE];
	uint8_t tx_len;
	uint8_t rx_len;
	uint8_t mode;
};

#endif /* __SAF_SPI_TRANSACTION_H__ */
