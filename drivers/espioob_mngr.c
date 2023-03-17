/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/espi.h>
#include "espi_hub.h"
#include "espioob_mngr.h"
#include "memops.h"

LOG_MODULE_REGISTER(oobmngr, CONFIG_ESPIOOB_MNGR_LOG_LEVEL);

#define MAX_OOB_BUF_SIZE		75U

#define ASYNC_MSGQ_MAX_MSGS		8U
#define ASYNC_MSGQ_ALIGNMENT		4U

#define OOB_MSG_LEN_FROM_BYTE_CNT(x)	(x + OOB_IDX_BYTE_CNT + 1)

static uint8_t rx_buf[MAX_OOB_BUF_SIZE];

struct oob_msg {
	struct espi_oob_packet *tx;
	struct espi_oob_packet *rx;
	struct k_mutex txn_lock;
	struct k_sem txn_sync;
};

static struct oob_msg master_hw;
static struct oob_msg master_pmc;
static struct oob_msg master_csme;

static oob_rx_callback_handler_t csme_msg_hndlr;
static oob_rx_callback_handler_t pmc_msg_hndlr;

struct async_msb {
	uint8_t buf[MAX_OOB_BUF_SIZE];
	uint16_t len;
	uint8_t from;
	oob_rx_callback_handler_t fn;
};

K_MSGQ_DEFINE(async_msgq, sizeof(struct async_msb), ASYNC_MSGQ_MAX_MSGS,
	ASYNC_MSGQ_ALIGNMENT);


void register_oob_hndlr(uint8_t master_addr, oob_rx_callback_handler_t fn)
{
	switch (master_addr) {
	case OOB_MASTER_ADDR_CSME:
		csme_msg_hndlr = fn;
		break;
	case OOB_MASTER_ADDR_PMC:
		pmc_msg_hndlr = fn;
		break;
	default:
		LOG_ERR("No handler support for master address");
	}
}

static int verify_oob_tx_pckt(struct espi_oob_packet *tx)
{
	uint8_t len = OOB_MSG_LEN_FROM_BYTE_CNT(tx->buf[OOB_IDX_BYTE_CNT]);

	if ((len >= MAX_OOB_BUF_SIZE) || (len < OOB_IDX_HDR_SIZE)) {
		LOG_ERR("Invalid byte count %d", len);
		return -EINVAL;
	}

	if (len != tx->len) {
		LOG_ERR("Tx len %d & byte count %d mis-match", tx->len, len);
		return -EINVAL;
	}

	switch (tx->buf[OOB_IDX_DEST_SLV_ADDR]) {
	case OOB_DST_ADDR(OOB_MASTER_ADDR_HW):
	case OOB_DST_ADDR(OOB_MASTER_ADDR_CSME):
	case OOB_DST_ADDR(OOB_MASTER_ADDR_PMC):
		break;
	default:
		LOG_ERR("Invalid Dest slave addr: %x",
			tx->buf[OOB_IDX_DEST_SLV_ADDR]);
		return -EINVAL;
	}

	if (tx->buf[OOB_IDX_SRC_SLV_ADDR] !=
		OOB_SRC_ADDR(OOB_SLAVE_ADDR_EC)) {
		LOG_INF("Invalid Src slave addr. Use %x", OOB_SLAVE_ADDR_EC);
		/* No need to return as error */
	}

	return 0;
}

static int verify_oob_rx_pckt(struct espi_oob_packet *rx)
{
	uint8_t len = OOB_MSG_LEN_FROM_BYTE_CNT(rx->buf[OOB_IDX_BYTE_CNT]);

	if ((len >= MAX_OOB_BUF_SIZE) || (len < OOB_IDX_HDR_SIZE)) {
		LOG_ERR("Invalid byte count %d", len);
		return -EINVAL;
	}

	if (len != rx->len) {
		LOG_ERR("rx len & byte count mis-match %d", len);
		return -EINVAL;
	}

	if (rx->buf[OOB_IDX_DEST_SLV_ADDR] !=
		OOB_DST_ADDR(OOB_SLAVE_ADDR_EC)) {
		/*
		 * Commented out due to PMC bug - PMC sends invalid addr.
		LOG_ERR("Invalid Dest slave addr in OOB Rx");
		return -EINVAL;
		*/
	}

	switch (rx->buf[OOB_IDX_SRC_SLV_ADDR]) {
	case OOB_SRC_ADDR(OOB_MASTER_ADDR_HW):
	case OOB_SRC_ADDR(OOB_MASTER_ADDR_CSME):
	case OOB_SRC_ADDR(OOB_MASTER_ADDR_PMC):
		break;
	default:
		LOG_ERR("Invalid Src slave addr %x in OOB Rx",
			rx->buf[OOB_IDX_SRC_SLV_ADDR]);
		return -EINVAL;
	}

	return 0;
}

static inline struct oob_msg *get_oob_master(uint8_t addr_byte)
{
	/* Decode 7bit master address from 8bit address value */
	uint8_t master_addr = OOB_7BIT_ADDR(addr_byte);

	switch (master_addr) {
	case OOB_MASTER_ADDR_HW:
		return &master_hw;
	case OOB_MASTER_ADDR_CSME:
		return &master_csme;
	case OOB_MASTER_ADDR_PMC:
		return &master_pmc;
	default:
		return NULL;
	}
}


int oob_send_sync(struct espi_oob_packet *req, struct espi_oob_packet *resp,
		  int timeout)
{
	int ret = 0;
	struct oob_msg *master;
	int wait_time = MAX(MIN(timeout, MAX_WAIT_TIME_FOR_OOB_IN_MS),
		MIN_WAIT_TIME_FOR_OOB_IN_MS);

#ifndef CONFIG_OOBMNGR_SUPPORT
	return -ENOTSUP;
#endif

	if ((req == NULL) || (resp == NULL)) {
		return -ENODATA;
	}

	ret = verify_oob_tx_pckt(req);
	if (ret) {
		LOG_ERR("OOB Tx packet verification failed %d", ret);
		return ret;
	}

	master = get_oob_master(req->buf[OOB_IDX_DEST_SLV_ADDR]);

	if (master == NULL) {
		return -EINVAL;
	}

	if (k_mutex_lock(&master->txn_lock,
			 K_MSEC(MIN_WAIT_TIME_FOR_OOB_IN_MS))) {
		LOG_ERR("OOB tx lock timeout");
		return -EBUSY;
	}

	master->tx = req;
	master->rx = resp;
	k_sem_reset(&master->txn_sync);

	ret = espihub_send_oob(master->tx);
	if (ret) {
		LOG_ERR("Error sending OOB %d", ret);
		k_mutex_unlock(&master->txn_lock);
		k_sem_give(&master->txn_sync);
		return -EIO;
	}

	LOG_DBG("OOB Tx Successful");

	/* Wait till OOB response, txn_sync semaphore released by rx handler */
	ret = k_sem_take(&master->txn_sync, K_MSEC(wait_time));

	if (ret) {
		LOG_ERR("OOB Rx sem timeout");
		ret = -ETIMEDOUT;
	} else {
		if (master->rx->len) {
			LOG_DBG("OOB Rx Successful");
		} else {
			LOG_ERR("OOB Rx received, but buffer space not enough");
			ret = -ENOBUFS;
		}
	}

	k_mutex_unlock(&master->txn_lock);
	k_sem_give(&master->txn_sync);

	return ret;
}


int oob_send_async(struct espi_oob_packet *req, oob_rx_callback_handler_t cb)
{
	int ret;
	struct async_msb msg;

#ifndef CONFIG_OOBMNGR_SUPPORT
	return -ENOTSUP;
#endif

	if (req == NULL) {
		return -ENODATA;
	}

	ret = verify_oob_tx_pckt(req);
	if (ret) {
		LOG_ERR("OOB Tx packet verification failed %d", ret);
		return ret;
	}

	msg.len = req->len;
	memcpys(msg.buf, req->buf, req->len);
	msg.fn = cb;
	msg.from = OOB_SLAVE_ADDR_EC;

	ret = k_msgq_put(&async_msgq, &msg, K_NO_WAIT);
	if (ret) {
		LOG_ERR("Async msg request enque failed %d", ret);
		return -ENOBUFS;
	}

	return 0;
}


int oob_respond_master(struct espi_oob_packet *tx)
{
	int ret;
	struct oob_msg *master;

#ifndef CONFIG_OOBMNGR_SUPPORT
	return -ENOTSUP;
#endif

	if (tx == NULL) {
		return -ENODATA;
	}

	ret = verify_oob_tx_pckt(tx);
	if (ret) {
		LOG_ERR("OOB Tx packet verification failed %d", ret);
		return ret;
	}

	master = get_oob_master(tx->buf[OOB_IDX_DEST_SLV_ADDR]);

	if (master == NULL) {
		return -EINVAL;
	}

	if (k_mutex_lock(&master->txn_lock, K_NO_WAIT)) {
		LOG_ERR("OOB tx lock timeout");
		return -EBUSY;
	}

	master->tx = tx;

	ret = espihub_send_oob(master->tx);
	if (ret) {
		LOG_ERR("Error sending OOB %d", ret);
		ret = -EIO;
	} else {
		LOG_DBG("OOB Tx Successful");
	}

	k_mutex_unlock(&master->txn_lock);
	return ret;
}


/* Intended to be handled as in ISR - No Lengthy routines */
static void oob_rx_handler(struct espi_oob_packet *rx)
{
	int ret;
	struct oob_msg *master;
	struct async_msb msg;

	/* Validate OOB message */
	ret = verify_oob_rx_pckt(rx);
	if (ret) {
		LOG_ERR("Invalid Rx packet");
		return;
	}

	/* Find the OOB Rx master */
	master = get_oob_master(rx->buf[OOB_IDX_SRC_SLV_ADDR]);

	if (master == NULL) {
		LOG_ERR("Msg from Unknown master - Discard");

		/* Clear Rx buffer for next downstream oob */
		memsets(rx->buf, 0, rx->len);
		rx->len = sizeof(rx_buf);
		return;
	}

	/*
	 * Route the OOB Rx to appropriate Rx buffer.
	 * If OOB rx received when semaphore not acquired by EC i.e. sem_count
	 * is a non-zero value, then the downstream OOB message is treated as
	 * master initiated OOB request.
	 * If OOB rx received when semaphore is already acquired by EC for the
	 * same master i.e. sem_count = 0, then the downstream OOB message is
	 * tied as a response to the EC initiated OOB request message. Semaphore
	 * must be released then for the waiting task to catch the response.
	 */
	if (k_sem_count_get(&master->txn_sync)) {
		/*
		 * This is where CSME incoming messages can be handled
		 *
		 * Also OOB responses received post timeout land up here.
		 * No action needed, they can be discarded. Warning log is
		 * enough. When that happens, MIN_WAIT_TIME should be tweaked.
		 */

		if (rx->len < sizeof(msg.buf)) {
			memcpys(msg.buf, rx->buf, rx->len);
			msg.fn = NULL;
			msg.len = rx->len;
			msg.from = OOB_7BIT_ADDR(rx->buf[OOB_IDX_SRC_SLV_ADDR]);

			if (k_msgq_put(&async_msgq, &msg, K_NO_WAIT)) {
				LOG_ERR("Rx msg enque failed %d", ret);
			}
		} else {
			LOG_ERR("Rx Message size too big.");
		}

	} else {

		if (master->rx->len >= rx->len) {
			memcpys(master->rx->buf, rx->buf, rx->len);
			master->rx->len = rx->len;
		} else {
			master->rx->len = 0;
			LOG_WRN("Rx Buf space too small");
		}

		k_sem_give(&master->txn_sync);
	}


	/* Clear Rx buffer for next downstream oob */
	memsets(rx->buf, 0, rx->len);
	rx->len = sizeof(rx_buf);
}

static void oobmngr_init(void)
{
	k_mutex_init(&master_hw.txn_lock);
	k_mutex_init(&master_pmc.txn_lock);
	k_mutex_init(&master_csme.txn_lock);

	k_sem_init(&master_hw.txn_sync, 1, 1);
	k_sem_init(&master_pmc.txn_sync, 1, 1);
	k_sem_init(&master_csme.txn_sync, 1, 1);
}


void oobmngr_thread(void *p1, void *p2, void *p3)
{
	int ret;
	struct async_msb msg;

	oobmngr_init();

	while (1) {
		k_msgq_get(&async_msgq, &msg, K_FOREVER);

		if (msg.from == OOB_SLAVE_ADDR_EC) {
			/* OOB message from EC to master */
			struct espi_oob_packet req = {
				.buf = msg.buf, .len = msg.len};
			struct espi_oob_packet resp = {
				.buf = msg.buf, .len = sizeof(msg.buf)};

			ret = oob_send_sync(&req, &resp,
					    OOB_MSG_SYNC_WAIT_TIME_DFLT);

			LOG_DBG("Async msg processed, status: %d", ret);

			if (msg.fn != NULL) {
				msg.fn(&resp, ret);
			}
		} else {
			/* Master initiated OOB message */
			switch (msg.from) {
			case OOB_MASTER_ADDR_CSME:
				msg.fn = csme_msg_hndlr;
				break;
			case OOB_MASTER_ADDR_PMC:
				msg.fn = pmc_msg_hndlr;
				break;
			default:
				LOG_ERR("Unsupported 0%x", msg.from);
				break;
			}

			if (msg.fn != NULL) {
				struct espi_oob_packet mstr_msg = {
					.buf = msg.buf, .len = msg.len};

				msg.fn(&mstr_msg, 0);
			}
		}
	}
}

void oob_rx_cb_handler(void)
{
#ifndef CONFIG_OOBMNGR_SUPPORT
	return;
#endif
	struct espi_oob_packet rx = {
		.buf = rx_buf,
		.len = sizeof(rx_buf)
	};

	if (espihub_retrieve_oob(&rx) == 0) {
		oob_rx_handler(&rx);
	}
}

