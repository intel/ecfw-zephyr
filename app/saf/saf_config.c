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
#include <zephyr/logging/log.h>
#include "board_config.h"
#include "saf_spi_transaction.h"
#include "saf_config.h"

LOG_MODULE_REGISTER(saf_config, CONFIG_ESPIHUB_LOG_LEVEL);

#define SPI_RESET_DELAY_US             40U
#define MHZ_TO_HZ(x)                   ((x) * 1000000U)
#define MAX_SPI_RESPONSE               10U

/* SPI opcodes are vendor-specific.
 * These are either used directly as commands sent via SPI driver
 * or used to initialize SAF HW registers.
 * SAF bridge then will translate the eSPI FLASH requests from host into
 * SPI transactions.
 */
#ifdef CONFIG_SAF_SPI_WINBOND
#include "saf_spi_winbond.h"
#else
#pragma error "Unsupported SAF SPI flash device"
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(spi0), disabled)
#pragma error "spi hw block not enabled"
#else
#define DT_SPI_INST	DT_NODELABEL(spi0)
#endif

#if DT_PROP(DT_SPI_INST, lines) == 4
#define SPI_IO_LINES	SPI_LINES_QUAD
#else
#define SPI_IO_LINES	SPI_LINES_DUAL
#endif

static const struct espi_saf_cfg *get_saf_cfg(void)
{
#ifdef CONFIG_SAF_SPI_WINBOND
	return windbond_saf_cfg();
#else
#pragma error "Unsupported SAF SPI flash device"
	return NULL;
#endif
}

static struct saf_spi_transaction *spi_cmd(enum saf_command_index command)
{
#ifdef CONFIG_SAF_SPI_WINBOND
	return windbond_qspi_cmd(command);
#else
#pragma error "Unsupported SAF SPI flash device"
	return NULL;
#endif
}

static const struct device *saf_dev;
static const struct device *spi_dev;
static struct spi_config spi_cfg;

static int spi_send_cmd(uint8_t slave_index, struct saf_spi_transaction *cmd,
			int mode)
{
	int ret;
	uint8_t data[MAX_SPI_RESPONSE];
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

	tx.buf = cmd->buf;
	tx.len = cmd->tx_len;
	rx.buf = data;
	rx.len = cmd->rx_len;

	/* Increase SPI driver compatibility, do not indicate RX buffer despite
	 * of the buffers been empty
	 */
	if (cmd->rx_len) {
		rx_bufs.buffers = &rx;
		rx_bufs.count = 1;
	}

	spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB
			    | SPI_WORD_SET(8) | mode;
	spi_cfg.slave = slave_index;

	ret = spi_transceive(spi_dev, &spi_cfg, &tx_bufs, &rx_bufs);
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

	LOG_DBG("%s ", __func__);

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

	LOG_DBG("%s", __func__);

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

	LOG_DBG("%s", __func__);
	tx.buf = NULL;
	tx.len = 9;

	spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB
			    | SPI_WORD_SET(8) | SPI_IO_LINES;
	spi_cfg.slave = slave_index;

	ret = spi_transceive(spi_dev, &spi_cfg, &tx_bufs, &rx_bufs);
	if (ret < 0) {
		LOG_ERR("SPI transceive error: %d", ret);
		return ret;
	}

	return 0;
}

int spi_flash_init(uint8_t slave_index)
{
	int ret;

	LOG_DBG("%s", __func__);

	spi_dev = DEVICE_DT_GET(SPI_0);
	if (!device_is_ready(spi_dev)) {
		LOG_ERR("SPI device not ready");
		return -ENODEV;
	};

	spi_cfg.frequency = MHZ_TO_HZ(CONFIG_SAF_SPI_FREQ_MHZ);
	spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB
			    | SPI_WORD_SET(8) | SPI_LINES_SINGLE;
	spi_cfg.slave = 0;
	spi_cfg.cs = NULL;

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

int initialize_saf_bridge(void)
{
	int ret;
	bool saf_ready;

	saf_dev = DEVICE_DT_GET(ESPI_SAF_0);
	if (!device_is_ready(saf_dev)) {
		LOG_ERR("ESPI SAF device not ready");
		return -ENODEV;
	}

	LOG_DBG("%d SPI flash devices", CONFIG_SAF_SPI_DEVICES_COUNT);
	LOG_DBG("Capacity %d  MB", CONFIG_SAF_SPI_CAPACITY);
	LOG_DBG("Frequency %d", CONFIG_SAF_SPI_FREQ_MHZ);

	/* SAF requires that SPI flash device is reset and in Quad mode */
	for (int i = 0; i < CONFIG_SAF_SPI_DEVICES_COUNT; i++) {
		ret = spi_flash_init(i);
		if (ret) {
			LOG_ERR("Fail to init SPI device %d: %d", i, ret);
			return ret;
		}
	}

	ret = espi_saf_config(saf_dev, get_saf_cfg());
	if (ret) {
		LOG_ERR("SAF configuration failed: %d", ret);
		return ret;
	}

	espi_saf_activate(saf_dev);

	LOG_DBG("Check if SAF channel is getting disabled");
	saf_ready = espi_saf_get_channel_status(saf_dev);
	if (!saf_ready) {
		LOG_ERR("SAF channel not ready");
		return -EIO;
	}

	LOG_DBG("SAF channel ready");

	return 0;
}

int saf_write_flash(struct espi_saf_packet *pckt)
{
	return espi_saf_flash_write(saf_dev, pckt);
}

int saf_erase_flash(struct espi_saf_packet *pckt)
{
	return espi_saf_flash_erase(saf_dev, pckt);
}
