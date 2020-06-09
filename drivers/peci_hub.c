/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/peci.h>
#include <logging/log.h>
#include "board_config.h"
#include "errno.h"
#include <soc.h>

/* PECI Host address */
#define PECI_HOST_ADDR          0x30u
/* PECI Host bitrate 1Mbps */
#define PECI_HOST_BITRATE       1000u

#define PECI_CONFIGINDEX_TJMAX  16u
#define PECI_CONFIGHOSTID       0u
#define PECI_CONFIGPARAM        0u

#define PECI_RETRY_CNT		3
#define PECI_RETRY_WAIT		1 /* 1 milli sec */

/* Offsets in rx buffer */
#define PECI_RX_BUF_RESP_OFFSET	0
#define PECI_RX_BUF_TJMAX_OFFSET 3

/* 16-bit interpretation of temperature,
 * sign bit = 15bit and bit14-bit6 = Temperatur data
 */
#define GET_TEMP_POS		6
#define GET_TEMP_MASK		0x7E00

#define PECI_TARGET_HOST_ID_OFFSET 0
#define PECI_RETRY_BIT		BIT(0)

/* CPU fail safe temperature value is 72C */
#define PECI_CPUTEMP_FAILSAFE    72

LOG_MODULE_REGISTER(peci_interface, CONFIG_PECI_LOG_LEVEL);
K_MUTEX_DEFINE(trans_mutex);

static struct device *peci_dev;
static bool peci_initialized;
static u8_t cpu_tjmax;

static void peci_get_max_temp(void);

static int peci_exec_transfer(struct device *dev, struct peci_msg *msg)
{
	int ret;
	u8_t rd_len = msg->rx_buffer.len;

	k_mutex_lock(&trans_mutex, K_FOREVER);
	if (!peci_initialized) {
		LOG_ERR("PECI not initialized");
		k_mutex_unlock(&trans_mutex);
		return -ENODEV;
	}

	ret = peci_transfer(peci_dev, msg);

	for (int i = 0; i < rd_len; i++) {
		LOG_DBG("%s:Rx[%d]-%02x", __func__, i,
				msg->rx_buffer.buf[i]);
	}

	LOG_DBG("%s:Rx-FCS=%x", __func__, msg->rx_buffer.buf[rd_len]);
	k_mutex_unlock(&trans_mutex);
	return ret;
}

int peci_rdpkg_config(u8_t *req_buf, u8_t *resp_buf, u8_t rd_len)
{
	int ret;
	int retries = PECI_RETRY_CNT;
	u8_t peci_resp;
	struct peci_msg packet;

	packet.tx_buffer.buf = req_buf;
	packet.tx_buffer.len = PECI_RD_PKG_WR_LEN;
	packet.rx_buffer.buf = resp_buf;
	packet.rx_buffer.len = rd_len;

	do {
		packet.addr = PECI_HOST_ADDR;
		packet.cmd_code = PECI_CMD_RD_PKG_CFG0;

		ret = peci_exec_transfer(peci_dev, &packet);


		peci_resp = resp_buf[PECI_RX_BUF_RESP_OFFSET];
		k_sleep(K_MSEC(PECI_RETRY_WAIT));
		LOG_DBG("peci_resp %x", peci_resp);
		retries--;
	} while ((peci_resp != PECI_RW_PKG_CFG_RSP_PASS) && (retries > 0));

	if (peci_resp != PECI_RW_PKG_CFG_RSP_PASS) {
		LOG_ERR("Peci RdPkgConfig failed");
		return -EIO;
	}

	LOG_DBG("Peci RdPkgConfig success");
	return 0;
}

int peci_get_tjmax(u8_t *tjmax)
{
	int ret;

	u8_t resp_buf[PECI_RD_PKG_LEN_DWORD + 1];
	u8_t req_buf[] = { PECI_CONFIGHOSTID,
				PECI_CONFIGINDEX_TJMAX,
				PECI_CONFIGPARAM & 0x00FF,
				(PECI_CONFIGPARAM & 0xFF00) >> 8,
	};

	ret = peci_rdpkg_config(req_buf, resp_buf, PECI_RD_PKG_LEN_DWORD);

	if (!ret) {
		*tjmax = resp_buf[PECI_RX_BUF_TJMAX_OFFSET];
	}

	return ret;
}

int peci_get_temp(int *temperature)
{
	s16_t raw_cpu_temp;
	int ret;
	struct peci_msg packet;

	/* If cpu tjmax is not fetched then cpu temperature cannot be
	 * calculated. In this case return fail safe temperature.
	 */
	if (cpu_tjmax == 0) {
		peci_get_max_temp();
		*temperature = PECI_CPUTEMP_FAILSAFE;
		return -EINVAL;
	}

	u8_t resp_buf[PECI_GET_TEMP_RD_LEN + 1];

	packet.tx_buffer.buf = NULL;
	packet.tx_buffer.len = PECI_GET_TEMP_WR_LEN;
	packet.rx_buffer.buf = resp_buf;
	packet.rx_buffer.len = PECI_GET_TEMP_RD_LEN;

	packet.addr = PECI_HOST_ADDR;
	packet.cmd_code = PECI_CMD_GET_TEMP0;

	ret = peci_exec_transfer(peci_dev, &packet);
	if (ret) {
		LOG_ERR("Peci GetTemp failed, ret-%d", ret);
		*temperature = PECI_CPUTEMP_FAILSAFE;
		return ret;
	}

	raw_cpu_temp = (s16_t)(resp_buf[PECI_GET_TEMP_LSB] |
			(s16_t)((resp_buf[PECI_GET_TEMP_MSB] << 8) & 0xFF00));

	if (raw_cpu_temp == PECI_GENERAL_SENSOR_ERROR) {
		LOG_ERR("%s:Invalid temp %x", __func__, raw_cpu_temp);
		*temperature = PECI_CPUTEMP_FAILSAFE;
		return -EINVAL;
	}

	 /* 16-bit interpretation of temperature,
	  * sign bit = 15bit and bit14-bit6 = Temperatur data
	  */
	raw_cpu_temp = (raw_cpu_temp >> GET_TEMP_POS) | GET_TEMP_MASK;

	/* Temperature conversion: Temperature reading is always -ve and
	 * refers offset from CPU Tjmax Value. i.e. if Tjmax is 100C and
	 * gettemp reading is -10 then cpu temperature will be 90c.
	 */
	*temperature = raw_cpu_temp + cpu_tjmax;

	return 0;
}

static void peci_get_max_temp(void)
{
	int ret;

	ret = peci_get_tjmax(&cpu_tjmax);
	if (ret) {
		LOG_ERR("Fail to obtain maximum temperature: %d", ret);
	} else {
		LOG_INF("CPU maximum temperature: %d", cpu_tjmax);
	}
}

int peci_init(void)
{
	int ret;

	peci_dev = device_get_binding(PECI_0_INST);
	if (!peci_dev) {
		LOG_ERR("PECI device not found");
		return -ENODEV;
	}

	ret = peci_config(peci_dev, PECI_HOST_BITRATE);
	if (ret) {
		LOG_ERR("Fail to configure bitrate");
		return -EIO;
	}

	ret = peci_enable(peci_dev);
	if (ret) {
		LOG_ERR("Fail to enable peci");
		return -EIO;
	}
	peci_initialized = true;
	LOG_INF("Peci init success");

	cpu_tjmax = 0;
	peci_get_max_temp();
	return 0;
}
