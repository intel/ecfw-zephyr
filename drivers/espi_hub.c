/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#include <zephyr/drivers/espi.h>
#include "espi_hub.h"
#include "pwrseq_utils.h"
#include "board_config.h"
#include "espioob_mngr.h"

LOG_MODULE_REGISTER(espihub, CONFIG_ESPIHUB_LOG_LEVEL);

#ifdef ENABLE_ESPI_LTR
/* LTR requirement enable */
#define ESPI_LTR_REQ_ENABLE		1U
/* LTR latency value in unit of scale */
#define ESPI_LTR_LATENCY		2U
#endif

static const struct device *espi_dev;
static struct espihub_context hub;
static espi_warn_handler_t warn_handlers[ESPIHUB_MAX_HANDLER_INDEX];
static espi_state_handler_t state_handler;
static espi_acpi_handler_t acpi_handlers[MAX_ACPI_HANDLERS];
static espi_kbc_handler_t kbc_handler;
static espi_postcode_handler_t postcode_handler;

/* Registration from other modules */
int espihub_add_state_handler(espi_state_handler_t handler)
{
	__ASSERT(handler, "Handler shouldn't be NULL");
	if (state_handler) {
		LOG_ERR("Only 1 state handler supported");
		return -EINVAL;
	}

	state_handler = handler;
	return 0;
}

int espihub_add_warn_handler(enum espihub_handler type,
			      espi_warn_handler_t handler)
{
	__ASSERT(handler, "Handler shouldn't be NULL");
	if (warn_handlers[type]) {
		LOG_ERR("Only warn %d handler supported", type);
		return -EINVAL;
	}

	warn_handlers[type] = handler;
	return 0;
}

int espihub_add_acpi_handler(enum espihub_acpi_handler type,
			      espi_acpi_handler_t handler)
{
	__ASSERT(handler, "Handler shouldn't be NULL");
	if (acpi_handlers[type]) {
		LOG_ERR("Only 1 ACPI %d handler supported", type);
		return -EINVAL;
	}

	acpi_handlers[type] = handler;
	return 0;
}

int espihub_add_kbc_handler(espi_kbc_handler_t handler)
{
	__ASSERT(handler, "Handler shouldn't be NULL");
	if (kbc_handler) {
		LOG_ERR("Only 1 KBC handler supported");
		return -EINVAL;
	}

	kbc_handler = handler;
	return 0;
}

int espihub_add_postcode_handler(espi_postcode_handler_t handler)
{
	__ASSERT(handler, "Handler shouldn't be NULL");
	if (postcode_handler) {
		LOG_ERR("Only 1 postcode handler supported");
		return -EINVAL;
	}

	postcode_handler = handler;
	return 0;
}

/* Status */
enum boot_config_mode espihub_boot_mode(void)
{
	return hub.spi_boot_mode;
}

bool espihub_reset_status(void)
{
	return hub.espi_rst_sts;
}

bool espihub_dnx_status(void)
{
	int ret;
	uint8_t vw_level;

	ret = espihub_retrieve_vw(ESPI_VWIRE_SIGNAL_DNX_WARN, &vw_level);
	if (ret) {
		LOG_WRN("Unable to retrieve DnX warn level");
		vw_level = ESPIHUB_VW_LOW;
	} else {
		espihub_send_vw(ESPI_VWIRE_SIGNAL_DNX_ACK, vw_level);
		hub.dnx_mode = vw_level;
	}

	return (vw_level == ESPIHUB_VW_HIGH);
}

static void host_warn_handler(uint32_t signal, uint32_t status)
{
	LOG_DBG("%s", __func__);
	switch (signal) {
	case ESPI_VWIRE_SIGNAL_PLTRST:
		LOG_INF("PLT_RST changed %d", status);
		if (warn_handlers[ESPIHUB_PLATFORM_RESET]) {
			warn_handlers[ESPIHUB_PLATFORM_RESET](status);
		} else {
			LOG_WRN("No PLT_RST handler registered");
		}
		break;
	case ESPI_VWIRE_SIGNAL_HOST_RST_WARN:
		if (warn_handlers[ESPIHUB_RESET_WARNING]) {
			warn_handlers[ESPIHUB_RESET_WARNING](status);
		} else {
			LOG_WRN("No Host rst handler registered");
		}
		LOG_INF("Send ACK HOST RST %d", status);
		espi_send_vwire(espi_dev, ESPI_VWIRE_SIGNAL_HOST_RST_ACK,
				status);
		break;
	case ESPI_VWIRE_SIGNAL_OOB_RST_WARN:
		LOG_INF("Send OOB_RST_ACK %d", status);
		espi_send_vwire(espi_dev, ESPI_VWIRE_SIGNAL_OOB_RST_ACK,
				status);
		break;
	case ESPI_VWIRE_SIGNAL_SUS_WARN:
		if (warn_handlers[ESPIHUB_SUSPEND_WARNING]) {
			warn_handlers[ESPIHUB_SUSPEND_WARNING](status);
		} else {
			LOG_WRN("No suspend handler registered");
		}
		break;
	case ESPI_VWIRE_SIGNAL_DNX_WARN:
		hub.dnx_mode = status;
		LOG_INF("Send DnX WARN %d", status);
		espi_send_vwire(espi_dev, ESPI_VWIRE_SIGNAL_DNX_ACK, status);
		if (warn_handlers[ESPIHUB_DNX_WARNING]) {
			warn_handlers[ESPIHUB_DNX_WARNING](status);
		} else {
			LOG_WRN("No dnx handler registered");
		}
		break;
	default:
		LOG_ERR("Host warning unhandled %d", signal);
		break;
	}
}

/* eSPI bus event handler */
static void espi_reset_handler(const struct device *dev,
			       struct espi_callback *cb,
			       struct espi_event event)
{
	LOG_WRN("%s", __func__);
	if (event.evt_type == ESPI_BUS_RESET) {
		hub.espi_rst_sts = event.evt_data;
		LOG_INF("eSPI BUS reset %d", event.evt_data);
		if (warn_handlers[ESPIHUB_BUS_RESET]) {
			warn_handlers[ESPIHUB_BUS_RESET](event.evt_data);
		} else {
			LOG_WRN("No bus reset handler registered");
		}
	}
}

/* eSPI logical channels enable/disable event handler */
static void espi_ch_handler(const struct device *dev, struct espi_callback *cb,
			    struct espi_event event)
{
#ifdef ENABLE_ESPI_LTR
	int ltr_status;
#endif

	LOG_DBG("%s", __func__);
	if (event.evt_type == ESPI_BUS_EVENT_CHANNEL_READY) {
		if (event.evt_details == ESPI_CHANNEL_VWIRE) {
			LOG_INF("VW channel ready: %d", event.evt_data);
			hub.host_vw_ready = event.evt_data;
		}

#ifdef ENABLE_ESPI_LTR
		if (event.evt_details == ESPI_CHANNEL_PERIPHERAL) {
			LOG_INF("PH channel is ready");
			if (event.evt_data == ESPI_PC_EVT_BUS_MASTER_ENABLE) {
				LOG_DBG("Sending ltr.... ");
				ltr_status = espihub_send_ltr();
				if (!ltr_status) {
					LOG_DBG("LTR_Send success.... ");
				} else {
					LOG_ERR("LTR_Send failed %d",
						ltr_status);
				}
			}
		}
#endif
	}
}

/* eSPI vwire received event handler */
static void vwire_handler(const struct device *dev, struct espi_callback *cb,
			  struct espi_event event)
{
	LOG_INF("VWire %d sts: %d", event.evt_details, event.evt_data);
	if (event.evt_type == ESPI_BUS_EVENT_VWIRE_RECEIVED) {
		switch (event.evt_details) {
		case ESPI_VWIRE_SIGNAL_PLTRST:
			host_warn_handler(event.evt_details, event.evt_data);
			break;
		case ESPI_VWIRE_SIGNAL_SLP_S3:
		case ESPI_VWIRE_SIGNAL_SLP_S4:
		case ESPI_VWIRE_SIGNAL_SLP_S5:
			LOG_INF("SLP %d changed %d", event.evt_details,
				event.evt_data);
			if (state_handler) {
				state_handler(event.evt_details,
					      event.evt_data);
			} else {
				LOG_WRN("No state handler registered");
			}
			break;
		case ESPI_VWIRE_SIGNAL_SUS_WARN:
		case ESPI_VWIRE_SIGNAL_HOST_RST_WARN:
		case ESPI_VWIRE_SIGNAL_OOB_RST_WARN:
		case ESPI_VWIRE_SIGNAL_DNX_WARN:
			host_warn_handler(event.evt_details, event.evt_data);
			break;
		default:
			LOG_WRN("Unhandled VWire %d", event.evt_details);
			break;
		}
	}
}

/* eSPI peripheral channel notifications handler */
static void periph_handler(const struct device *dev, struct espi_callback *cb,
			   struct espi_event event)
{
	uint8_t periph_type;
	uint8_t periph_index;

	periph_type = ESPI_PERIPHERAL_TYPE(event.evt_details);
	periph_index = ESPI_PERIPHERAL_INDEX(event.evt_details);

	switch (periph_type) {
	case ESPI_PERIPHERAL_DEBUG_PORT80:
		if (postcode_handler) {
			postcode_handler(periph_index, event.evt_data);
		} else {
			LOG_WRN("No postcode handler registered");
		}
		break;
	case ESPI_PERIPHERAL_HOST_IO:
		if (acpi_handlers[ESPIHUB_ACPI_PUBLIC]) {
			acpi_handlers[ESPIHUB_ACPI_PUBLIC]();
		} else {
			LOG_WRN("No ACPI handler registered");
		}
		break;
#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC
	case ESPI_PERIPHERAL_8042_KBC:
		/* The second lowest byte contains the data and the lowest
		 * byte indicates if the information received was command
		 * or data
		 */
		if (kbc_handler) {
			kbc_handler(KBC_IBF_DATA(event.evt_data),
				    KBC_CMD_DATA(event.evt_data));
		} else {
			LOG_WRN("No KBC handler registered");
		}
		break;
#endif
	default:
		LOG_INF("%s periph 0x%x [%x]", __func__,
			periph_type, event.evt_data);
		break;
	}
}

#ifdef CONFIG_ESPI_OOB_CHANNEL_RX_ASYNC
/* eSPI bus OOB event handler */
static void espi_oob_rx_handler(const struct device *dev,
			       struct espi_callback *cb,
			       struct espi_event event)
{
	if (event.evt_type == ESPI_BUS_EVENT_OOB_RECEIVED) {
		oob_rx_cb_handler();
	}
}
#endif

void detect_boot_mode(void)
{
	bool flash_sts;
	int level;

	if (hub.boot_config_detected) {
		LOG_INF("Boot mode already detected");
		return;
	}

	/* Default state */
	hub.spi_boot_mode = FLASH_BOOT_MODE_G3_SHARING;

	flash_sts = espi_get_channel_status(espi_dev, ESPI_CHANNEL_FLASH);
	LOG_WRN("Flash channel ready %d", flash_sts);
	if (flash_sts) {
		hub.spi_boot_mode = FLASH_BOOT_MODE_MAF;
	} else {
		level = gpio_read_pin(G3_SAF_DETECT);
		LOG_WRN("G3_SAF_DETECT %d", level);
		if (level < 0) {
			LOG_WRN("Fail to read HW strap");
			return;
		}

		if (level) {
			hub.spi_boot_mode = FLASH_BOOT_MODE_SAF;
		} else {
			hub.spi_boot_mode =  FLASH_BOOT_MODE_G3_SHARING;
		}
	}

	hub.boot_config_detected = true;
}

int espihub_init(void)
{
	int ret;

	/* Indicate to eSPI master simplest configuration: Single line,
	 * 20MHz frequency and only logical channel 0 and 1 are supported
	 */
	struct espi_cfg cfg = {
#ifdef CONFIG_POWER_SEQUENCE_MINIMUM_ESPI_CAPS
		.io_caps = ESPI_IO_MODE_SINGLE_LINE,
		.channel_caps = ESPI_CHANNEL_VWIRE | ESPI_CHANNEL_PERIPHERAL |
				ESPI_CHANNEL_OOB | ESPI_CHANNEL_FLASH,
		.max_freq = MIN_ESPI_FREQ,
#else
		.io_caps = ESPI_IO_MODE_SINGLE_LINE | ESPI_IO_MODE_QUAD_LINES |
				ESPI_IO_MODE_DUAL_LINES,
		.channel_caps = ESPI_CHANNEL_VWIRE | ESPI_CHANNEL_PERIPHERAL |
				ESPI_CHANNEL_OOB | ESPI_CHANNEL_FLASH,
		.max_freq = MAX_ESPI_FREQ,
#endif
	};

	espi_dev = DEVICE_DT_GET(ESPI_0);
	if (!device_is_ready(espi_dev)) {
		LOG_ERR("ESPI device not ready");
		return -ENODEV;
	}

	LOG_DBG("About to configure eSPI device %s", __func__);
	ret = espi_config(espi_dev, &cfg);
	if (ret) {
		LOG_ERR("eSPI slave configured failed");
		return ret;
	}

	espi_init_callback(&hub.espi_bus_cb, espi_reset_handler,
			   ESPI_BUS_RESET);
	espi_init_callback(&hub.vw_rdy_cb, espi_ch_handler,
			   ESPI_BUS_EVENT_CHANNEL_READY);
	espi_init_callback(&hub.vw_cb, vwire_handler,
			   ESPI_BUS_EVENT_VWIRE_RECEIVED);
	espi_init_callback(&hub.p80_cb, periph_handler,
			   ESPI_BUS_PERIPHERAL_NOTIFICATION);

	espi_add_callback(espi_dev, &hub.espi_bus_cb);
	espi_add_callback(espi_dev, &hub.vw_rdy_cb);
	espi_add_callback(espi_dev, &hub.vw_cb);
	espi_add_callback(espi_dev, &hub.p80_cb);

#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC

	espi_init_callback(&hub.kbc_cb, periph_handler,
				ESPI_BUS_PERIPHERAL_NOTIFICATION);
	espi_add_callback(espi_dev, &hub.kbc_cb);

#endif

#ifdef CONFIG_ESPI_OOB_CHANNEL_RX_ASYNC
	espi_init_callback(&hub.oob_cb, espi_oob_rx_handler,
			   ESPI_BUS_EVENT_OOB_RECEIVED);
	espi_add_callback(espi_dev, &hub.oob_cb);
#endif

	hub.host_vw_ready = espi_get_channel_status(espi_dev,
						    ESPI_CHANNEL_VWIRE);
	hub.espi_rst_sts = gpio_read_pin(ESPI_RESET_MAF);

	LOG_DBG("%s hub.host_vw_ready: %d", __func__, hub.host_vw_ready);
	return ret;
}

static int handle_vw_ack(enum espi_vwire_signal signal, uint8_t value)
{
	int ret;

	LOG_DBG("%s", __func__);
	switch (signal) {
	case ESPI_VWIRE_SIGNAL_SUS_WARN:
		ret = espi_send_vwire(espi_dev, ESPI_VWIRE_SIGNAL_SUS_ACK,
				      value);
		break;
	default:
		LOG_ERR("Ack for %d not supported", signal);
		return -EINVAL;
	}

	return ret;
}


int espihub_wait_for_vwire(enum espi_vwire_signal signal, uint16_t timeout,
		   uint8_t exp_level, bool ack_required)
{
	int ret;
	uint8_t level;
	int loop_cnt = timeout;

	do {
		ret = espi_receive_vwire(espi_dev, signal, &level);
		if (ret) {
			LOG_ERR("Failed to read %x %d", signal, ret);
			return -EIO;
		}

		if (exp_level == level) {
			break;
		}

		k_usleep(100);
		loop_cnt--;
	} while (loop_cnt > 0 || (timeout == WAIT_TIMEOUT_FOREVER) ||
		 ec_timeout_status());

	if (loop_cnt == 0) {
		LOG_DBG("VWIRE %d is %x", signal, level);
		return -ETIMEDOUT;
	}

	if (ack_required) {
		handle_vw_ack(signal, level);
	}

	return 0;
}

int espihub_wait_for_espi_reset(uint8_t exp_sts, uint16_t timeout)
{
	int loop_cnt = timeout;

	do {
		if (exp_sts == hub.espi_rst_sts) {
			break;
		}
		k_usleep(100);
		loop_cnt--;
	} while (loop_cnt > 0 || (timeout == WAIT_TIMEOUT_FOREVER) ||
		 ec_timeout_status());

	if (loop_cnt == 0) {
		return -ETIMEDOUT;
	}

	return 0;
}

int wait_for_pin_monitor_vwire(uint32_t port_pin, uint32_t exp_sts,
			       uint16_t timeout,
			       enum espi_vwire_signal signal,
			       uint8_t abort_sts)
{
	int loop_cnt = timeout;
	uint8_t vw_level;
	int pin_sts;

	do {
		pin_sts = gpio_read_pin(port_pin);
		if (pin_sts < 0) {
			LOG_ERR("Fail to read %s pin", __func__);
		}

		/* While waiting for pin, monitor virtual wire */
		espi_receive_vwire(espi_dev, signal, &vw_level);
		if (vw_level == abort_sts) {
			LOG_WRN("eSPI host aborted transition");
			return -EINVAL;
		}

		k_usleep(100);
		loop_cnt--;
	} while (pin_sts && loop_cnt > 0);

	if (loop_cnt == 0) {
		LOG_ERR("%d never occurred", exp_sts);
		return -ETIMEDOUT;
	}

	return 0;
}

int espihub_retrieve_vw(enum espi_vwire_signal signal,
			uint8_t *level)
{
	if (!hub.host_vw_ready) {
		LOG_ERR("eSPI host not ready to receive VWs");
	}

	return espi_receive_vwire(espi_dev, signal, level);
}

int espihub_send_vw(enum espi_vwire_signal signal, uint8_t level)
{
	/* Per SoC spec need to ensure that eSPI host is ready */
	if (!hub.host_vw_ready) {
		LOG_ERR("eSPI host not ready to receive VWs");
		return -EINVAL;
	}

	return espi_send_vwire(espi_dev, signal, level);
}

int espihub_retrieve_oob(struct espi_oob_packet *resp_pckt)
{
	return espi_receive_oob(espi_dev, resp_pckt);
}

int espihub_send_oob(struct espi_oob_packet *req_pckt)
{
	return espi_send_oob(espi_dev, req_pckt);
}

int espihub_kbc_write(enum lpc_peripheral_opcode cmd, uint32_t data)
{
	uint32_t ldata = data;

	return espi_write_lpc_request(espi_dev, cmd, &ldata);
}

int espihub_kbc_read(enum lpc_peripheral_opcode cmd, uint32_t *data)
{
	return espi_read_lpc_request(espi_dev, cmd, data);
}

#ifdef ENABLE_ESPI_LTR
int espihub_send_ltr(void)
{
	struct ltr_cfg_pkt req;

	req.ltr_req = ESPI_LTR_REQ_ENABLE;
	req.ltr_scale = ESPI_LTR_SCALE_1MSEC;
	req.latency = ESPI_LTR_LATENCY;

	return espi_send_ltr(espi_dev, &req);
}
#endif

int espihub_write_flash(struct espi_flash_packet *pckt)
{
	if (hub.dnx_mode) {
		return -EBUSY;
	}

	return espi_write_flash(espi_dev, pckt);
}

int espihub_read_flash(struct espi_flash_packet *pckt)
{
	if (hub.dnx_mode) {
		return -EBUSY;
	}

	return espi_read_flash(espi_dev, pckt);
}

int espihub_erase_flash(struct espi_flash_packet *pckt)
{
	if (hub.dnx_mode) {
		return -EBUSY;
	}

	return espi_flash_erase(espi_dev, pckt);
}
