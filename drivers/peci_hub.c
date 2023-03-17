/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/peci.h>
#include <zephyr/logging/log.h>
#include "board_config.h"
#include "errno.h"
#include <soc.h>
#include "espioob_mngr.h"
#include "memops.h"
#include "smchost.h"
#include "peci_hub.h"

#define PECI_GPU_ADDR		0x32u
#define PECI_CPU_ADDR		0x30u
#define PECI_HOST_BITRATE_KBPS  1000u

#define PECI_CONFIGINDEX_PL4OFFSET	72u
#define PECI_CONFIGINDEX_TJMAX  16u
#define PECI_CONFIGHOSTID       0u
#define PECI_CONFIGPARAM        0u
#define PECI_CFG_WRPKG_AWFCS    8u
#define PECI_CFG_WRMSR_AWFCS    12u
#define PECI_CFG_WRPCI_AWFCS    9u

#define PECI_WRPKG_AWFCS_LEN    12u
#define PECI_WRMSR_AWFCS_LEN    16u
#define PECI_WRPCI_AWFCS_LEN    13u

#define PECI_FCS_LEN		2

#define PECI_RETRY_CNT		3
#define PECI_RETRY_WAIT		1 /* 1 milli sec */

/* Offsets in rx buffer */
#define PECI_RX_BUF_RESP_OFFSET	0
#define PECI_RX_BUF_TJMAX_OFFSET 3

/* Offsets in tx buffer */
#define PECI_TX_BUF_HOSTIDRETRY_OFFSET 0
#define PECI_TX_BUF_INDEX		 1U
#define PECI_TX_BUF_PARAM_LSB		 2U
#define PECI_TX_BUF_PARAM_MSB		 3U
#define PECI_TX_BUF_DATA0		 4U
#define PECI_TX_BUF_DATA1		 5U
#define PECI_TX_BUF_DATA2		 6U
#define PECI_TX_BUF_DATA3		 7U

#define GET_TEMP_INTEGER_POS	6u

#define PECI_TARGET_HOST_ID_OFFSET 0
#define PECI_RETRY_EN		BIT(0)

/* CPU fail safe temperature value is 72C */
#define PECI_CPUGPU_TEMP_FAILSAFE	72

/* Various PECI offsets in the buffer sent by the host. These can
 * be considered the header of the ensuing PECI transaction
 */
#define CLIENT_ADDRESS_OFFSET	0U
#define TX_BUF_LEN_OFFSET	1U
#define RX_BUF_LEN_OFFSET	2U
#define COMMAND_CODE_OFFSET	3U
#define TX_BUF_START_OFFSET	4U

#define PECI_DATA_BUF_LEN_MAX	30U /* 30 Bytes data length */
#define PECI_AWFCS_DATA_LEN		12U

#define PCH_OOB_PECI_SLV_ADDR	0x20U
#define EC_OOB_SLV_ADDR		0x0EU
#define PECI_OOB_CMD_CODE	0x01U
#define OOB_PACKET_HEADER_SIZE	4U
#define OOB_PECI_REQ_HDR_SIZE	4U
#define OOB_PECI_RESP_SIZE	1U

LOG_MODULE_REGISTER(peci_interface, CONFIG_PECIHUB_LOG_LEVEL);
K_MUTEX_DEFINE(trans_mutex);

struct espi_oob_header {
} __packed;

struct espi_oob_peci_req_msg {
	uint8_t oob_dest_addr;
	uint8_t oob_cmd_code;
	uint8_t oob_byte_cnt;
	uint8_t oob_src_addr;
	uint8_t peci_addr;
	uint8_t peci_wr_len;
	uint8_t peci_rd_len;
	uint8_t peci_cmd_code;
	uint8_t data[PECI_DATA_BUF_LEN_MAX];
} __packed;

struct espi_oob_peci_resp_msg {
	uint8_t oob_dest_addr;
	uint8_t oob_cmd_code;
	uint8_t oob_byte_cnt;
	uint8_t oob_src_addr;
	uint8_t peci_resp_code;
	uint8_t data[PECI_DATA_BUF_LEN_MAX];
} __packed;

static const struct device *peci_dev;
static bool peci_initialized;
static uint8_t cpu_tjmax;
static uint8_t gpu_tjmax;

/* Initialising  to POE as default mode */
uint8_t peci_access_mode = PECI_OVER_ESPI_MODE;

/* As per the PECI specification, originator need to calculate AWFCS
 * (Assured Write Frame Check Sequence) code using below checksum table.
 */
const uint8_t peci_crc_table[] = {
	0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, /* Offset 0-7 */
	0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d, /* Offset 8-15 */
	0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, /* Offset 16-23 */
	0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, /* Offset 24-31 */
	0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, /* Offset 32-39 */
	0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD, /* Offset 40-47 */
	0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, /* Offset 48-55 */
	0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD, /* Offset 56-63 */
	0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, /* Offset 64-71 */
	0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, /* Offset 72-79 */
	0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, /* Offset 80-87 */
	0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A, /* Offset 88-95 */
	0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, /* Offset 96-103 */
	0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A, /* Offset 104-111 */
	0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, /* Offset 112-119 */
	0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, /* Offset 120-127 */
	0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, /* Offset 128-135 */
	0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4, /* Offset 136-143 */
	0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, /* Offset 144-151 */
	0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4, /* Offset 152-159 */
	0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, /* Offset 160-167 */
	0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, /* Offset 168-175 */
	0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, /* Offset 176-183 */
	0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34, /* Offset 184-191 */
	0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, /* Offset 192-199 */
	0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63, /* Offset 200-207 */
	0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, /* Offset 208-215 */
	0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13, /* Offset 216-223 */
	0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, /* Offset 224-231 */
	0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83, /* Offset 232-239 */
	0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, /* Offset 240-247 */
	0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3  /* Offset 248-255 */
};

static inline uint8_t get_peci_address(enum peci_devices dev)
{
	switch (dev) {
	case GPU:
		return PECI_GPU_ADDR;

	case CPU:
		return PECI_CPU_ADDR;

	default:
		LOG_ERR("Unknown PECI device: %d", dev);
		return -1;
	}
}

#ifdef CONFIG_DEPRECATED_HW_STRAP_BASED_PECI_MODE_SEL
static void detect_peci_over_espi_mode(void)
{
	uint8_t en = LEGACY_PECI_MODE;
	/* SW strap always takes precedence */
#ifdef CONFIG_PECI_OVER_ESPI_ENABLE
	en = PECI_OVER_ESPI_MODE;
#else
#ifdef PECI_OVER_ESPI
	if (gpio_read_pin(PECI_OVER_ESPI) == 0) {
		en = PECI_OVER_ESPI_MODE;
	}
#endif /* PECI_OVER_ESPI */
#endif /*CONFIG_PECI_OVER_ESPI_ENABLE */
	peci_access_mode = en;
}
#else
void peci_access_mode_config(uint8_t mode)
{
	/* Not updating the peci_access_mode when we receive wrong
	 * Value from BIOS , if we update wrong value it might break the
	 * ongoing peci transactions and might become security issue
	 */
	if ((mode == LEGACY_PECI_MODE) || (mode == PECI_OVER_ESPI_MODE)) {
		peci_access_mode = mode;
	} else {
		LOG_WRN("%s: Wrong peci access mode received: %x",
			__func__, mode);
	}

	LOG_DBG("%s: mode = %x, peci access mode = %x",
		__func__, mode, peci_access_mode);
}
#endif /* CONFIG_DEPRECATED_HW_STRAP_BASED_PECI_MODE_SEL */

static bool is_peci_over_espi_en(void)
{
	bool peci_over_espi_en;
	/* SW strap always takes precedence */
#ifdef CONFIG_PECI_OVER_ESPI_ENABLE
	peci_over_espi_en = true;
#else
	if (peci_access_mode == PECI_OVER_ESPI_MODE) {
		peci_over_espi_en = true;
	} else {
		peci_over_espi_en = false;
	}
#endif /* CONFIG_PECI_OVER_ESPI_ENABLE */

	LOG_DBG("%s: peci_over_espi_en=%x",
		__func__, peci_over_espi_en);

	return peci_over_espi_en;
}
static int peci_wire_transfer(const struct device *dev, struct peci_msg *msg)
{
	/* Skip peci access when CPU in C10 state */
	if (gpio_read_pin(CPU_C10_GATE) == LOW) {
		LOG_DBG("Skip peci in c10");
		return -EHOSTDOWN;
	}

	return peci_transfer(peci_dev, msg);
}

uint8_t peci_calc_awfcs(uint8_t *peci_buffer, uint8_t awfcs_len)
{
	uint8_t peci_awfcs = 0x0;
	uint8_t count = 0x0;

	/* Parameter verification */
	if (peci_buffer == NULL) {
		LOG_ERR("%s(): No memory available for peci buf", __func__);
		return -ENOMEM;
	}

	/* Compute AWFCS code */
	for (count = 0; count < awfcs_len; count++) {
		peci_awfcs = peci_awfcs ^ peci_buffer[count];
		peci_awfcs = peci_crc_table[peci_awfcs];
	}

	/* Invert upper bit to prevent mathematical failure that would
	 * cause the Write FCS to always be 0x0.
	 */
	peci_awfcs ^= BIT(7);

	return peci_awfcs;
}

static int espioob_peci_transfer(struct peci_msg *msg)
{
	struct espi_oob_peci_req_msg oob_req;
	struct espi_oob_peci_resp_msg oob_resp;
	struct espi_oob_packet req_pckt;
	struct espi_oob_packet resp_pckt;
	uint8_t oob_byte_cnt =  OOB_PECI_REQ_HDR_SIZE + msg->tx_buffer.len;
	int ret;

	LOG_DBG("%s:Msg TxLen-%d, RxLen-%d", __func__,
			msg->tx_buffer.len, msg->rx_buffer.len);
	oob_req.oob_dest_addr = PCH_OOB_PECI_SLV_ADDR;
	oob_req.oob_cmd_code = PECI_OOB_CMD_CODE;
	oob_req.oob_byte_cnt = oob_byte_cnt;
	oob_req.oob_src_addr = EC_OOB_SLV_ADDR;
	oob_req.peci_addr = msg->addr;
	oob_req.peci_wr_len = msg->tx_buffer.len;
	oob_req.peci_rd_len = msg->rx_buffer.len;
	oob_req.peci_cmd_code = msg->cmd_code;

	/* Tx length includes peci code. So copy len-1 byte as data */
	if (msg->tx_buffer.len > 1) {
		memcpys(oob_req.data, msg->tx_buffer.buf,
					msg->tx_buffer.len - 1);
	}

	req_pckt.buf = (uint8_t *)&oob_req;
	req_pckt.len = OOB_PACKET_HEADER_SIZE + oob_byte_cnt - 1;
	resp_pckt.buf = (uint8_t *)&oob_resp;
	resp_pckt.len = sizeof(oob_resp);

	ret = oob_send_sync(&req_pckt, &resp_pckt, OOB_MSG_SYNC_WAIT_TIME_DFLT);
	if (ret) {
		LOG_ERR("PECI OOB Txn failed %d", ret);
		return ret;
	}

	/* Response length include peci command code and response code.
	 * So exclude 2 byte for data.
	 */
	if (oob_resp.oob_byte_cnt > 2) {
		ret = memcpys(msg->rx_buffer.buf, oob_resp.data,
					oob_resp.oob_byte_cnt - 2);
		if (ret) {
			LOG_ERR("Failed while copying response buffer");
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Tranfers the peci packet and get the response.
 *
 * This function is intended for peci commands which
 * doenst support command retry.
 *
 * @param *msg peci packet message.
 * @retval 0 on success and failure code on error.
 */
static int peci_exec_transfer(struct peci_msg *msg)
{
	int ret;
	uint8_t rd_len = msg->rx_buffer.len;

	k_mutex_lock(&trans_mutex, K_FOREVER);
	if (!peci_initialized && !is_peci_over_espi_en()) {
		LOG_ERR("PECI not initialized");
		k_mutex_unlock(&trans_mutex);
		return -ENODEV;
	}

	/* PECI over eSPI is supported only for CPU. For
	 * others (like GPU), only legacy PECI is supported
	 */
	if (is_peci_over_espi_en() && (msg->addr == PECI_CPU_ADDR)) {
		ret = espioob_peci_transfer(msg);
	} else {
		ret = peci_wire_transfer(peci_dev, msg);
	}

	for (int i = 0; i < rd_len; i++) {
		LOG_DBG("%s:Rx[%d]-%02x", __func__, i,
				msg->rx_buffer.buf[i]);
	}

	LOG_DBG("Peci command = %x success", msg->cmd_code);
	k_mutex_unlock(&trans_mutex);
	return ret;
}

/**
 * @brief Tranfers the peci packet with retry and get the response.
 *
 * This function is intended for peci commands
 * supports command retry.
 *
 * @param *msg peci packet message.
 * @retval 0 on success and failure code on error.
 */
static int peci_exec_transfer_retry(struct peci_msg *msg)
{
	int ret;
	int retries = PECI_RETRY_CNT;
	uint8_t peci_resp = 0;

	k_mutex_lock(&trans_mutex, K_FOREVER);
	if (!peci_initialized && !is_peci_over_espi_en()) {
		LOG_ERR("PECI not initialized");
		k_mutex_unlock(&trans_mutex);
		return -ENODEV;
	}

	do {
		retries--;

		/* PECI over eSPI is supported only for CPU. For
		 * others (like GPU), only legacy PECI is supported
		 */
		if (is_peci_over_espi_en() && (msg->addr == PECI_CPU_ADDR)) {
			ret = espioob_peci_transfer(msg);
		} else {
			ret = peci_wire_transfer(peci_dev, msg);
		}
		if (ret) {
			continue;
		}

		peci_resp = msg->rx_buffer.buf[PECI_RX_BUF_RESP_OFFSET];
		LOG_DBG("peci_resp %x", peci_resp);

		if (peci_resp == PECI_CC_RSP_SUCCESS) {
			/* Command execution successful */
			break;
		}

		/* Command failed! Verify response code */
		switch (peci_resp) {
		case PECI_CC_RSP_TIMEOUT:
		case PECI_CC_OUT_OF_RESOURCES_TIMEOUT:
			/* Retry cmd since processor unable to generate response
			 * ontime or unable to allocate resources required to
			 * service the cmd.
			 */
			k_msleep(PECI_RETRY_WAIT);
			msg->tx_buffer.buf[PECI_TX_BUF_HOSTIDRETRY_OFFSET] |=
						PECI_RETRY_EN;
			break;
		case PECI_CC_RESOURCES_LOWPWR_TIMEOUT:
			/* TODO: Resources required to service cmd are in low
			 * power mode. Enable "wake on peci" mode to pop-up
			 * processor to C2 state to service the cmd.
			 */
			break;
		case PECI_CC_ILLEGAL_REQUEST:
			/* Invalid or illegal Request */
			break;
		default:
			LOG_WRN("Invalid peci response %x", peci_resp);
			break;
		}
	} while (retries > 0);

	k_mutex_unlock(&trans_mutex);

	if (peci_resp != PECI_CC_RSP_SUCCESS) {
		LOG_ERR("Peci command %x failed", msg->cmd_code);
		return -EIO;
	}

	LOG_DBG("Peci command=%x success", msg->cmd_code);
	return 0;
}

int peci_cmd_execute(uint8_t *req_buf, uint8_t *resp_buf,
		     uint8_t max_req_buf_size)
{
	int ret;
	struct peci_msg packet;

	packet.tx_buffer.buf = &req_buf[TX_BUF_START_OFFSET];
	packet.tx_buffer.len = req_buf[TX_BUF_LEN_OFFSET];
	packet.rx_buffer.buf = resp_buf;
	packet.rx_buffer.len = req_buf[RX_BUF_LEN_OFFSET];

	packet.addr = req_buf[CLIENT_ADDRESS_OFFSET];
	packet.cmd_code = req_buf[COMMAND_CODE_OFFSET];

	/* Calc AWFCS for cmds resulting in update to processor regs */
	switch (packet.cmd_code) {
	case PECI_CMD_WR_PKG_CFG0:
		packet.tx_buffer.buf[PECI_CFG_WRPKG_AWFCS] =
			peci_calc_awfcs(req_buf, PECI_WRPKG_AWFCS_LEN);
		break;
	case PECI_CMD_WR_IAMSR0:
		packet.tx_buffer.buf[PECI_CFG_WRMSR_AWFCS] =
			peci_calc_awfcs(req_buf, PECI_WRMSR_AWFCS_LEN);
		break;
	case PECI_CMD_WR_PCI_CFG0:
		packet.tx_buffer.buf[PECI_CFG_WRPCI_AWFCS] =
			peci_calc_awfcs(req_buf, PECI_WRPCI_AWFCS_LEN);
		break;
	default:
		/* No AWFCS field */
		break;
	}

	/* Retry the peci cmds on failure for cmds supports retry */
	switch (packet.cmd_code) {
	case PECI_CMD_PING:
	case PECI_CMD_GET_DIB:
	case PECI_CMD_GET_TEMP0:
		ret = peci_exec_transfer(&packet);
		break;
	case PECI_CMD_RD_PKG_CFG0:
	case PECI_CMD_WR_PKG_CFG0:
	case PECI_CMD_RD_IAMSR0:
	case PECI_CMD_WR_IAMSR0:
	case PECI_CMD_RD_PCI_CFG0:
	case PECI_CMD_WR_PCI_CFG0:
		ret = peci_exec_transfer_retry(&packet);
		break;
	default:
		LOG_WRN("Invalid peci command %x", packet.cmd_code);
		return -EINVAL;
	}

	if (ret) {
		LOG_ERR("Peci command %d failed", packet.cmd_code);
		/* Update error code on command failure */
		packet.tx_buffer.buf[packet.tx_buffer.len] =
				packet.rx_buffer.buf[PECI_RX_BUF_RESP_OFFSET];
		return ret;
	}

	/* The data received in peci resp_buf would get copied over
	 * to the peci req_buffer in the bytes after the "Write bytes".
	 * We need to ensure there's enough space in req_buffer to
	 * hold this data.
	 */
	if ((packet.tx_buffer.len + packet.rx_buffer.len) >
			max_req_buf_size) {
		LOG_ERR("tx_buffer overflow");
		return -1;
	}

	/* Populate back the req_buffer from the resp_buf */
	for (int i = 0; i < packet.rx_buffer.len; i++) {
		packet.tx_buffer.buf[packet.tx_buffer.len + i] =
					packet.rx_buffer.buf[i];
	}

	/* Except for some, most other commands expect a completion code */
	switch (packet.cmd_code) {

	case PECI_CMD_PING:
	case PECI_CMD_GET_TEMP0:
	case PECI_CMD_GET_TEMP1:
	case PECI_CMD_GET_DIB:
		break;

	default:
		if (packet.rx_buffer.buf[0] != PECI_CC_RSP_SUCCESS) {
			LOG_ERR("Peci command-%x failed", packet.cmd_code);
			return -EIO;
		}
		break;
	}

	return 0;
}

int peci_update_pl4_offset(uint32_t pl4_value)
{
	int ret;
	uint8_t req_buf[PECI_DATA_BUF_LEN_MAX];
	uint8_t resp_buf[PECI_WR_PKG_RD_LEN];

	LOG_DBG("Update PL4 offset with %d W", pl4_value);

	req_buf[CLIENT_ADDRESS_OFFSET] = PECI_CPU_ADDR;
	req_buf[TX_BUF_LEN_OFFSET] = PECI_WR_PKG_LEN_DWORD;
	req_buf[RX_BUF_LEN_OFFSET] = PECI_WR_PKG_RD_LEN;
	req_buf[COMMAND_CODE_OFFSET] = PECI_CMD_WR_PKG_CFG0;
	req_buf[TX_BUF_START_OFFSET + PECI_TX_BUF_HOSTIDRETRY_OFFSET] = 0x0;
	req_buf[TX_BUF_START_OFFSET + PECI_TX_BUF_PARAM_LSB] = 0x0;
	req_buf[TX_BUF_START_OFFSET + PECI_TX_BUF_PARAM_MSB] = 0x0;
	req_buf[TX_BUF_START_OFFSET + PECI_TX_BUF_INDEX] =
		PECI_CONFIGINDEX_PL4OFFSET;
	req_buf[TX_BUF_START_OFFSET + PECI_TX_BUF_DATA0] =
		(pl4_value & 0xFF);
	req_buf[TX_BUF_START_OFFSET + PECI_TX_BUF_DATA1] =
		((pl4_value >> 8) & 0xFF);
	req_buf[TX_BUF_START_OFFSET + PECI_TX_BUF_DATA2] =
		((pl4_value >> 16) & 0xFF);
	req_buf[TX_BUF_START_OFFSET + PECI_TX_BUF_DATA3] =
		((pl4_value >> 24) & 0xFF);

	ret = peci_wrpkg_config(req_buf, resp_buf, PECI_WR_PKG_LEN_DWORD);
	if (ret) {
		LOG_ERR("PL4 update failed (0x%x)", ret);
		return -EIO;
	}

	return 0;
}

int peci_rdpkg_config(enum peci_devices dev, uint8_t *req_buf,
		      uint8_t *resp_buf, uint8_t rd_len)
{
	int ret;
	struct peci_msg packet;
	uint8_t address = get_peci_address(dev);

	packet.tx_buffer.buf = req_buf;
	packet.tx_buffer.len = PECI_RD_PKG_WR_LEN;
	packet.rx_buffer.buf = resp_buf;
	packet.rx_buffer.len = rd_len;

	packet.addr = address;
	packet.cmd_code = PECI_CMD_RD_PKG_CFG0;

	ret = peci_exec_transfer_retry(&packet);
	if (ret) {
		LOG_ERR("Peci RdPkgConfig failed");
	}

	return ret;
}

int peci_wrpkg_config(uint8_t *req_buf, uint8_t *resp_buf, uint8_t wr_len)
{
	int ret;
	struct peci_msg packet;

	packet.addr = req_buf[CLIENT_ADDRESS_OFFSET];
	packet.cmd_code = req_buf[COMMAND_CODE_OFFSET];
	packet.tx_buffer.buf = &req_buf[TX_BUF_START_OFFSET];
	packet.tx_buffer.len = req_buf[TX_BUF_LEN_OFFSET];
	packet.rx_buffer.buf = resp_buf;
	packet.rx_buffer.len = req_buf[RX_BUF_LEN_OFFSET];
	packet.tx_buffer.buf[PECI_CFG_WRPKG_AWFCS] =
				peci_calc_awfcs(req_buf, PECI_WRPKG_AWFCS_LEN);

	ret = peci_exec_transfer_retry(&packet);
	if (ret) {
		LOG_ERR("Peci WrPkgConfig failed (0x%x)", ret);
	}

	return ret;
}

int peci_rd_ia_msr(enum peci_devices dev, uint8_t *req_buf,
		   uint8_t *resp_buf, uint8_t rd_len)
{
	int ret;
	struct peci_msg packet;
	uint8_t address = get_peci_address(dev);

	packet.tx_buffer.buf = req_buf;
	packet.tx_buffer.len = PECI_RD_IAMSR_WR_LEN;
	packet.rx_buffer.buf = resp_buf;
	packet.rx_buffer.len = rd_len;

	packet.addr = address;
	packet.cmd_code = PECI_CMD_RD_IAMSR0;

	ret = peci_exec_transfer_retry(&packet);
	if (ret) {
		LOG_ERR("Peci RdIAMSR failed");
	}

	return ret;
}

int peci_wr_ia_msr(enum peci_devices dev, uint8_t *req_buf,
		   uint8_t *resp_buf, uint8_t wr_len)
{
	int ret;
	struct peci_msg packet;
	uint8_t address = get_peci_address(dev);

	packet.tx_buffer.buf = req_buf;
	packet.tx_buffer.len = wr_len;
	packet.rx_buffer.buf = resp_buf;
	packet.rx_buffer.len = PECI_WR_IAMSR_RD_LEN;
	packet.tx_buffer.buf[PECI_CFG_WRMSR_AWFCS] =
				peci_calc_awfcs(req_buf, PECI_WRMSR_AWFCS_LEN);

	packet.addr = address;
	packet.cmd_code = PECI_CMD_WR_IAMSR0;

	ret = peci_exec_transfer_retry(&packet);
	if (ret) {
		LOG_ERR("Peci WrIAMSR failed");
	}

	return ret;
}

int peci_get_dib(enum peci_devices dev, uint8_t *dev_info, uint8_t *rev_num)
{
	int ret;
	uint8_t req_buf[PECI_GET_DIB_WR_LEN + PECI_FCS_LEN];
	uint8_t resp_buf[PECI_GET_DIB_RD_LEN + PECI_FCS_LEN];
	struct peci_msg packet;
	uint8_t address = get_peci_address(dev);

	packet.tx_buffer.buf = req_buf;
	packet.tx_buffer.len = PECI_GET_DIB_WR_LEN;
	packet.rx_buffer.buf = resp_buf;
	packet.rx_buffer.len = PECI_GET_DIB_RD_LEN;

	packet.addr = address;
	packet.cmd_code = PECI_CMD_GET_DIB;

	ret = peci_exec_transfer(&packet);
	if (ret) {
		LOG_ERR("Peci GetDIB failed");
	} else {
		*dev_info = resp_buf[PECI_GET_DIB_DEVINFO];
		*rev_num = resp_buf[PECI_GET_DIB_RD_LEN];
	}

	return ret;
}


int peci_get_tjmax(enum peci_devices dev, uint8_t *tjmax)
{
	int ret;

	uint8_t resp_buf[PECI_RD_PKG_LEN_DWORD + PECI_FCS_LEN];
	uint8_t req_buf[] = { PECI_CONFIGHOSTID,
				PECI_CONFIGINDEX_TJMAX,
				PECI_CONFIGPARAM & 0x00FF,
				(PECI_CONFIGPARAM & 0xFF00) >> 8,
	};

	ret = peci_rdpkg_config(dev, req_buf, resp_buf, PECI_RD_PKG_LEN_DWORD);

	if (!ret) {
		*tjmax = resp_buf[PECI_RX_BUF_TJMAX_OFFSET];
	}

	LOG_INF("TjMax=%d", *tjmax);
	return ret;
}

int peci_get_temp(enum peci_devices dev, int *temperature)
{
	uint16_t raw_cpu_temp;
	uint16_t peci_resp;
	int ret;
	struct peci_msg packet;
	uint8_t tjmax;
	uint8_t *tjmax_ptr;
	uint8_t address = get_peci_address(dev);

	switch (dev) {
	case CPU:
		tjmax_ptr = &cpu_tjmax;
		break;

	case GPU:
		tjmax_ptr = &gpu_tjmax;
		break;

	default:
		LOG_ERR("Unknown PECI device: %d", dev);
		return -EINVAL;
	}

	/* If cpu/gpu tjmax is not fetched then cpu/gpu temperature cannot
	 * be calculated. In this case return fail safe temperature.
	 */
	if (*tjmax_ptr == 0) {
		ret = peci_get_tjmax(dev, tjmax_ptr);
		if (ret) {
			LOG_ERR("Fail to get CPU/GPU TjMax: %d", ret);
			*temperature = PECI_CPUGPU_TEMP_FAILSAFE;
			return -EINVAL;
		}
	}
	tjmax = *tjmax_ptr;

	uint8_t resp_buf[PECI_GET_TEMP_RD_LEN + PECI_FCS_LEN];

	packet.tx_buffer.buf = NULL;
	packet.tx_buffer.len = PECI_GET_TEMP_WR_LEN;
	packet.rx_buffer.buf = resp_buf;
	packet.rx_buffer.len = PECI_GET_TEMP_RD_LEN;

	packet.addr = address;
	packet.cmd_code = PECI_CMD_GET_TEMP0;

	ret = peci_exec_transfer(&packet);
	if (ret) {
		LOG_ERR("Peci GetTemp failed, ret-%d", ret);
		*temperature = PECI_CPUGPU_TEMP_FAILSAFE;
		return ret;
	}

	peci_resp = (uint16_t)(resp_buf[PECI_GET_TEMP_LSB] |
		    (uint16_t)((resp_buf[PECI_GET_TEMP_MSB] << 8) & 0xFF00));

	if (peci_resp == PECI_GENERAL_SENSOR_ERROR) {
		LOG_ERR("%s:PECI_GENERAL_SENSOR_ERROR", __func__);
		*temperature = PECI_CPUGPU_TEMP_FAILSAFE;
		return -EINVAL;
	}

	/* TODO: During BIOS reset, EC receiving incorrect PECI data response
	 * '0' eventhough temp didn't reach TjMax leading to EC initiated
	 * shutdown. Temporary WA provided to return fail safe temperature
	 * when EC receives PECI data response '0'.
	 */
	if (peci_resp == 0) {
		LOG_ERR("%s:Incorrect PECI data response received", __func__);
		*temperature = PECI_CPUGPU_TEMP_FAILSAFE;
		return -EINVAL;
	}

	/* The temperature is returned as a negative value representing the
	 * number of degrees centigrade below the maximum processor junction
	 * temperature (Tjmax). It is formatted in a 16-bit, 2's complement
	 * value representing a number of 1/64 degrees centigrade. This format
	 * allows temperatures in a range of +/-512 C to be reported to
	 * approximately a 0.016 C resolution.
	 *
	 * The fractional value is represented by 6 LSBs (0.016 * 64 is ~1
	 * degree). We can ignore the fractional value here (i.e. 6 LSBs)
	 * to get a resolution of 1 degree C.
	 */

	/* To get the raw temperature, we take the 2's complement of the value
	 * returned by PECI and right shift by 6 (to get the resolution in 1
	 * degree C). Since this value is relative to TjMax, it is subtracted
	 * from TjMax to get the absolute temperature value.
	 */
	raw_cpu_temp = ~peci_resp + 1;
	raw_cpu_temp >>= GET_TEMP_INTEGER_POS;
	*temperature = tjmax - raw_cpu_temp;

	return 0;
}

int peci_init(void)
{
	int ret;

#ifdef CONFIG_DEPRECATED_HW_STRAP_BASED_PECI_MODE_SEL
	/* Read the HW Strap to determine the peci mode */
	detect_peci_over_espi_mode();
#endif /* CONFIG_DEPRECATED_HW_STRAP_BASED_PECI_MODE_SEL */

	peci_dev = DEVICE_DT_GET(PECI_0_INST);
	if (!device_is_ready(peci_dev)) {
		LOG_ERR("PECI device not ready");
		return -ENODEV;
	}

	ret = peci_config(peci_dev, PECI_HOST_BITRATE_KBPS);
	if (ret) {
		LOG_ERR("Fail to configure bitrate");
		return -EIO;
	}

	ret = peci_enable(peci_dev);
	if (ret) {
		LOG_ERR("Fail to enable peci");
		return -EIO;
	}
	LOG_INF("Peci init success");

	peci_initialized = true;
	cpu_tjmax = 0;
	gpu_tjmax = 0;
	return 0;
}

