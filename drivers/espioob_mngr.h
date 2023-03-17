/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief eSPI OOB Manager APIs.
 *
 */

#ifndef __ESPIOOB_MNGR_H_
#define __ESPIOOB_MNGR_H_

#include <zephyr/drivers/espi.h>


enum oob_pckt_idx {
	OOB_IDX_DEST_SLV_ADDR,
	OOB_IDX_CMD_CODE,
	OOB_IDX_BYTE_CNT,
	OOB_IDX_SRC_SLV_ADDR,

	OOB_IDX_HDR_SIZE
};

struct oob_msg_str {
	uint8_t dest_addr;
	uint8_t cmd_code;
	uint8_t byte_cnt;
	uint8_t src_addr;
	uint8_t payload[];
};

/* All the masters & slave addresses are defined 7 bits, these are bits[7:1]*/
#define OOB_MASTER_ADDR_HW		0x01U
#define OOB_MASTER_ADDR_CSME		0x08U
#define OOB_MASTER_ADDR_PMC		0x10U
#define OOB_MASTER_ADDR_IE		0x18U
#define OOB_SLAVE_ADDR_EC		0x07U

/* To form the 8 bit address for OOB source / destination slave address*/
#define OOB_DST_ADDR(addr_7bit)		(addr_7bit << 1)
#define OOB_SRC_ADDR(addr_7bit)		((addr_7bit << 1) | BIT(0))

/* To get 7 bit address from address byte field */
#define OOB_7BIT_ADDR(addr_byte)	(addr_byte >> 1)

/* OOB command codes for master: HW */
#define OOB_CMD_CODE_HW_TEMP		0x01U
#define OOB_CMD_CODE_HW_RTC		0x02U

/* OOB command codes for master: PMC */
#define OOB_CMD_CODE_PMC_PECI		0x01U
#define OOB_CMD_CODE_PMC_DBG_CNSNT	0x07U
#define OOB_CMD_CODE_PMC_PWR_MGMT_EVT	0x09U

/* OOB command codes for master: CSME */
#define OOB_CMD_CODE_CSME_SMBUS		0x0FU

/* OOB byte count for OOB messages */
#define OOB_BYTE_CNT_HW_REQ_MSG		0x01U
#define OOB_BYTE_CNT_PMC_PWR_MGMT_EVT	0x02U


/*
 * Macros for wait time
 *
 * Min & Max wait time for OOB transaction times are not by spec but for
 * fail-safe measures. Min wait time assures no second OOB transaction is
 * initiated to the master for the defined duration if response from the same
 * master is pending or not received for prior transaction. Max wait time
 * assures no single thread waits for OOB transaction completion for more
 * than defined time. These numbers may need to be tweaked for optimization.
 */
#define MAX_WAIT_TIME_FOR_OOB_IN_MS	5000U
#define MIN_WAIT_TIME_FOR_OOB_IN_MS	200U

#define OOB_MSG_SYNC_WAIT_TIME_DFLT	1000U

/**
 * @brief Routine that handles eSPI OOB transactions.
 *
 * This routine uniquely manages all in-bound & out-bound OOB transactions with
 * eSPI master. This serializes OOB messages & provide APIs for other modules.
 *
 * @param p1 pointer to additional task-specific data.
 * @param p2 pointer to additional task-specific data.
 * @param p2 pointer to additional task-specific data.
 *
 */
void oobmngr_thread(void *p1, void *p2, void *p3);

/**
 * @brief Synchronous OOB txn request.
 *
 * This allows a thread to send eSPI OOB request, and get back the response
 * from master within caller's thread context.
 *
 * @param req eSPI OOB request packet.
 * @param resp eSPI OOB response packet.
 * @param timeout max wait time in miliseconds for receiving OOB response.
 * @return 0 if successful, otherwise below error code.
 *
 * @return -ENOTSUP Function call not supported.
 * @return -EINVAL Invalid packet due to one of the following reasons:
 *		   - invalid source or slave address.
 *		   - invalid byte count field.
 *		   - invalid len if less than espi header size (4) or more than
 *		     max OOB packet buf size (75)
 * @return -ENODATA when request or response buffers are null.
 * @return -EBUSY when eSPI OOB channel is busy with an ongoing transaction for
 *		  the same master address.
 * @return -EIO General input / output error, failed to send over the bus.
 * @return -ETIMEDOUT response not received within timeout.
 * @return -ENOBUFS response buffer size is less than desired size.
 *
 * @note Can only be run from thread.
 */
int oob_send_sync(struct espi_oob_packet *req, struct espi_oob_packet *resp,
	int timeout);

/**
 * @brief Function pointer definition for handling asynchronous OOB response.
 *
 * @param rx eSPI OOB response packet.
 * @param err 0 if successful, otherwise handler function be notified with one
 *	      of the error codes for @fn oob_send_sync.
 */
typedef void (*oob_rx_callback_handler_t) (struct espi_oob_packet *rx,
		int err);

/**
 * @brief Asynchronous OOB txn request, which can be sent from ISR.
 *
 * This function allows caller to send OOB reqest from ISR.
 * This function enqueues OOB request to the message queue, and responds to the
 * caller in callback routine.
 *
 * @param req eSPI OOB request packet.
 * @param cb callback routine to be called when response for the requested OOB
 * message received from master.
 *
 * @return 0 if successful, otherwise below error code.
 *
 * @return -ENOTSUP Function call not supported.
 * @return -EINVAL Invalid packet due to one of the following reasons:
 *		   - invalid source or slave address.
 *		   - invalid byte count field.
 *		   - invalid len if less than espi header size (4) or more than
 *		     max OOB packet buf size (75).
 * @return -ENODATA when request buffer is null.
 * @return -ENOBUFS when too many OOB requests are queued and no space available
 *		    to queue another one.
 */
int oob_send_async(struct espi_oob_packet *req, oob_rx_callback_handler_t cb);

/**
 * @brief Send OOB message as response to the master initiated request.
 *
 * This allows a caller to send eSPI OOB message to the master. Caller can be
 * within a thread or an ISR.
 * This API to be used for cases where only upstream OOB need to be send, and
 * there would be no response back.
 *
 * @param tx eSPI OOB packet to be sent.
 *
 * @return 0 if successful, otherwise below error code.
 *
 * @return -ENOTSUP Function call not supported.
 * @return -EINVAL Invalid packet due to one of the following reasons:
 *		   - invalid source or slave address.
 *		   - invalid byte count field.
 *		   - invalid len if less than espi header size (4) or more than
 *		     max OOB packet buf size (75)
 * @return -ENODATA when tx buffer is null.
 * @return -EBUSY when eSPI OOB channel is busy with an ongoing transaction for
 *		  the same master address.
 * @return -EIO General input / output error, failed to send over the bus.
 */
int oob_respond_master(struct espi_oob_packet *tx);

/**
 * @brief Callback handler for downstream OOB messages.
 *
 * @note This function is executed within ISR, and only intended for eSPI_hub to
 * assign to the driver callback, when eSPI driver supports callback for OOB rx.
 * If driver does not support OOB rx callback, this routine must be run from
 * a thread to ensure the OOB rx retrieved.
 */
void oob_rx_cb_handler(void);

/**
 * @brief Register for master initiated OOB message handler.
 *
 * @param master_addr OOB master address supported : CSME, PMC.
 * @param fn callback routine to be called when master initiated OOB message
 * received from master.
 */
void register_oob_hndlr(uint8_t master_addr, oob_rx_callback_handler_t fn);

#endif /* __ESPIOOB_MNGR_H_ */
