/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include "pmc.h"
#include "espi_hub.h"
#include "espioob_mngr.h"
#include <zephyr/logging/log.h>
#include "memops.h"
LOG_MODULE_DECLARE(pwrmgmt, CONFIG_PWRMGT_LOG_LEVEL);

#define PMC_RESET_PAYLOAD_SIZE 1U

int pmc_reset_soc(enum pmc_request req_type, bool sync)
{
	uint8_t buf[sizeof(struct oob_msg_str) + PMC_RESET_PAYLOAD_SIZE];
	struct espi_oob_packet req_pckt = {0};
	struct oob_msg_str oob_msg = {0};
	int ret = 0;

	LOG_DBG("%s", __func__);

	oob_msg.dest_addr = OOB_DST_ADDR(OOB_MASTER_ADDR_PMC);
	oob_msg.cmd_code = OOB_CMD_CODE_PMC_PWR_MGMT_EVT;
	oob_msg.byte_cnt = 2;
	oob_msg.src_addr = OOB_SRC_ADDR(OOB_SLAVE_ADDR_EC);

	/* Packetize OOB request */
	memcpys(buf, (uint8_t *)&oob_msg, sizeof(struct oob_msg_str));
	buf[sizeof(struct oob_msg_str)] = req_type;
	req_pckt.buf = buf;
	req_pckt.len = sizeof(buf);

	if (sync) {
		/* Do send the OOB PMC request immediately */
		ret = oob_send_sync(&req_pckt, NULL, OOB_TX_HAS_RX, MIN_WAIT_TIME_FOR_OOB_IN_MS);
	} else {
		/* Queue request */
		ret = oob_send_async(&req_pckt, NULL, OOB_TX_HAS_RX);
	}

	if (ret) {
		LOG_ERR("PMC request failed %d\n", ret);
		return -EIO;
	}

	LOG_INF("SoC reset executed");
	return 0;
}
