/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <soc.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include "board_config.h"
#include "saf_spi_transaction.h"
LOG_MODULE_DECLARE(saf_config, CONFIG_ESPIHUB_LOG_LEVEL);

#define SPI_RESET_DELAY_US             40U
#define MHZ_TO_HZ(x)                   ((x) * 1000000U)
#define MAX_SPI_RESPONSE               10U
#define QSPI_EXIT_CONTINUOUS_TRANS_LEN 9U

/* SPI opcodes are vendor-specific.
 * These are either used directly as commands sent via SPI driver
 * or used to initialize SAF HW registers.
 * SAF bridge then will translate the eSPI FLASH requests from host into
 * SPI transactions.
 */
#ifdef CONFIG_FLASH
#elif CONFIG_SAF_SPI_WINBOND
#include "saf_spi_winbond.h"
#else
#pragma error "Unsupported SAF SPI flash device"
#endif

#if DT_NODE_HAS_STATUS(DT_ALIAS(spi0), disabled)
#pragma error "spi hw block not enabled"
#else
#define DT_SPI_INST	DT_ALIAS(spi0)
#endif

#if DT_PROP(DT_SPI_INST, lines) == 4
#define SPI_IO_LINES	SPI_LINES_QUAD
#else
#define SPI_IO_LINES	SPI_LINES_DUAL
#endif

static const struct device *spi_dev;
static struct spi_config spi_cfg;

static struct saf_spi_transaction *spi_cmd(enum saf_command_index command)
{
#ifdef CONFIG_SAF_SPI_WINBOND
	return windbond_qspi_cmd(command);
#else
#pragma error "Unsupported SAF SPI flash device"
	return NULL;
#endif
}

static int spi_send_cmd(uint8_t slave_index, struct saf_spi_transaction *cmd,
			int mode)
{
	int ret;
	uint8_t data[MAX_SPI_RESPONSE];
#ifdef CONFIG_SPI_XEC_QMSPI_LDMA
	struct spi_buf rx[2];
	struct spi_buf tx[2];
	struct spi_buf_set rx_bufs = {
		.buffers = rx,
		.count = 1,
	};
	struct spi_buf_set tx_bufs = {
		.buffers = tx,
		.count = 1,
	};
#else
	struct spi_buf rx;
	struct spi_buf tx;
	struct spi_buf_set rx_bufs = {
		.buffers = NULL,
		.count = 0,
	};

	struct spi_buf_set tx_bufs = {
		.buffers = &tx,
		.count = 1,
	};
#endif

#ifdef CONFIG_SPI_XEC_QMSPI_LDMA
	tx[0].buf = cmd->buf;
	tx[0].len = cmd->tx_len;
	tx[1].buf = NULL;
	tx[1].len = cmd->rx_len;

	rx[0].buf = NULL;
	rx[0].len = cmd->tx_len;
	rx[1].buf = data;
	rx[1].len = cmd->rx_len;
#else
	tx.buf = cmd->buf;
	tx.len = cmd->tx_len;
	rx.buf = data;
	rx.len = cmd->rx_len;
#endif
	/* Increase SPI driver compatibility, do not indicate RX buffer despite
	 * of the buffers been empty
	 */
	if (cmd->rx_len) {
#ifdef CONFIG_SPI_XEC_QMSPI_LDMA
		rx_bufs.count = 2;
		tx_bufs.count = 2;
#else
		rx_bufs.buffers = &rx;
		rx_bufs.count = 1;
#endif
	}

	spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB
			    | SPI_WORD_SET(8) | mode;
	spi_cfg.slave = slave_index;

	/* Need to send empty rx buffer in dual/ quad mode for SPI LDMA driver */
	if (IS_ENABLED(CONFIG_SPI_XEC_QMSPI_LDMA) && (mode != SPI_LINES_SINGLE)) {
		ret = spi_transceive(spi_dev, &spi_cfg, &tx_bufs, NULL);
	} else {
		ret = spi_transceive(spi_dev, &spi_cfg, &tx_bufs, &rx_bufs);
	}

	if (ret < 0) {
		LOG_ERR("SPI transceive error: %d", ret);
		return ret;
	}

	return 0;
}

static int qspi_read_status(uint8_t slave_index)
{
	int ret;
	struct saf_spi_transaction *spi_command;

	LOG_DBG("%d", slave_index);
	enum saf_command_index cmds[] = {
		RD_STS1_CMD_INDEX, RD_STS2_CMD_INDEX, EN_RST_CMD_INDEX,
		RD_STS1_CMD_INDEX, RD_STS1_CMD_INDEX
	};


	for (int i = 0; i < ARRAY_SIZE(cmds); i++) {
		spi_command = spi_cmd(cmds[i]);
		if (spi_command != NULL) {
			ret = spi_send_cmd(slave_index, spi_command,
					 SPI_LINES_SINGLE);
			if (ret) {
				return ret;
			}
		} else {
			return -EINVAL;
		}

	}

	return 0;
}

#ifdef CONFIG_SAF_ENABLE_XIP
int qspi_enable_xip(uint8_t slave_index)
{
	int ret = 0;
	struct saf_spi_transaction *spi_command;

	LOG_INF("%s", __func__);
	enum saf_command_index cmds[] = { WRITE_ENABLE_INDEX,
		WRITE_NV_REGISTER_INDEX,
		READ_NV_REGISTER_INDEX };

	for (int i = 0; i < ARRAY_SIZE(cmds); i++) {
		spi_command = spi_cmd(cmds[i]);
		if (spi_command != NULL) {
			ret = spi_send_cmd(slave_index, spi_command,
				 SPI_LINES_SINGLE);
			if (ret) {
				LOG_ERR("SPI command failed %d", ret);
				return ret;
			}
		} else {
			LOG_ERR("Invalid command");
			return -EINVAL;
		}
	}

	return 0;
}
#endif

static int qspi_reset_spi_flash_device(uint8_t slave_index)
{
	int ret;
	struct saf_spi_transaction *spi_command;

	LOG_DBG("%d", slave_index);
	enum saf_command_index cmds[] = { EN_RST_CMD_INDEX, RST_CMD_INDEX };

	for (int i = 0; i < ARRAY_SIZE(cmds); i++) {
		spi_command = spi_cmd(cmds[i]);
		if (spi_command != NULL) {
			ret = spi_send_cmd(slave_index, spi_command,
					SPI_IO_LINES);
			if (ret) {
				return ret;
			}
		} else {
			return -EINVAL;
		}
	}

	k_busy_wait(SPI_RESET_DELAY_US);

#ifdef CONFIG_SAF_ENABLE_XIP
	qspi_enable_xip(slave_index);
#endif

	/* For SPI flash devices with capacity over 16MB, need to enable
	 * 4-byte address mode
	 */
#if (CONFIG_SAF_SPI_CAPACITY == 32)
	ret = spi_send_cmd(slave_index, spi_cmd(FOUR_BYTE_CMD_INDEX),
	     SPI_LINES_SINGLE);
#endif

	return ret;
}

static int qspi_exit_continuous_mode(uint8_t slave_index)
{
	int ret;
	struct spi_buf tx;

	struct spi_buf_set rx_bufs = {
		.buffers = NULL,
		.count = 0,
	};

	struct spi_buf_set tx_bufs = {
		.buffers = &tx,
		.count = 1,
	};

	LOG_DBG("%d", slave_index);
	tx.buf = NULL;
	tx.len = QSPI_EXIT_CONTINUOUS_TRANS_LEN;

	spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB
			    | SPI_WORD_SET(8) | SPI_IO_LINES;
	spi_cfg.slave = slave_index;

	/* Need to send empty rx buffer in dual/ quad mode for SPI LDMA driver */
	if (IS_ENABLED(CONFIG_SPI_XEC_QMSPI_LDMA)) {
		ret = spi_transceive(spi_dev, &spi_cfg, &tx_bufs, NULL);
	} else {
		ret = spi_transceive(spi_dev, &spi_cfg, &tx_bufs, &rx_bufs);
	}

	if (ret < 0) {
		LOG_ERR("SPI transceive error: %d", ret);
		return ret;
	}

	return 0;
}

int saf_spi_init(uint8_t slave_index)
{
	int ret;

	LOG_DBG("%d SPI flash devices", CONFIG_SAF_SPI_DEVICES_COUNT);
	LOG_DBG("Capacity %d  MB", CONFIG_SAF_SPI_CAPACITY);
	LOG_DBG("Frequency %d", CONFIG_SAF_SPI_FREQ_MHZ);

	spi_dev = DEVICE_DT_GET(DT_ALIAS(spi0));
	if (!device_is_ready(spi_dev)) {
		LOG_ERR("SPI device not ready");
		return -ENODEV;
	};

	spi_cfg.frequency = MHZ_TO_HZ(CONFIG_SAF_SPI_FREQ_MHZ);
	spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB
			    | SPI_WORD_SET(8) | SPI_LINES_SINGLE;
	spi_cfg.slave = 0;

	ret = qspi_read_status(slave_index);
	if (ret) {
		LOG_ERR("Fail to read SPI flash device sts: %d", ret);
		return ret;
	}

	ret = qspi_exit_continuous_mode(slave_index);
	if (ret) {
		LOG_ERR("Fail to exit continuous mode: %d", ret);
		return ret;
	}

	ret = qspi_reset_spi_flash_device(slave_index);
	if (ret) {
		LOG_ERR("Fail to reset SPI flash device: %d", ret);
		return ret;
	}

	return 0;
}
