/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <soc.h>
#include <zephyr/drivers/espi.h>
#include <zephyr/drivers/espi_saf.h>
#include <zephyr/logging/log.h>
#include "board_config.h"
#include "saf_spi_transaction.h"
#ifdef CONFIG_SAF_SPI_WINBOND
#include "saf_spi_winbond.h"
#endif
#include "saf_config.h"

LOG_MODULE_REGISTER(saf_config, CONFIG_ESPIHUB_LOG_LEVEL);

static const struct device *saf_dev = DEVICE_DT_GET(DT_ALIAS(espitaf));

#ifdef CONFIG_FLASH
static struct espi_saf_cfg taf_cfg_flash = {
	.nflash_devices = 1U,
};
#endif

static const struct espi_saf_cfg *get_saf_cfg(void)
{
#ifdef CONFIG_FLASH
	return &taf_cfg_flash;
#elif CONFIG_SAF_SPI_WINBOND
	return windbond_saf_cfg();
#else
#pragma error "Unsupported SAF SPI flash device"
	return NULL;
#endif
}

int initialize_saf_bridge(void)
{
	int ret;
	bool saf_ready;

	if (!device_is_ready(saf_dev)) {
		LOG_ERR("ESPI SAF device not ready");
		return -ENODEV;
	}

#ifdef CONFIG_SPI
	/* SAF requires that SPI flash device is reset and in Quad mode */
	for (int i = 0; i < CONFIG_SAF_SPI_DEVICES_COUNT; i++) {
		ret = saf_spi_init(i);
		if (ret) {
			LOG_ERR("Fail to init SPI device %d: %d", i, ret);
			return ret;
		}
	}
#endif

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
