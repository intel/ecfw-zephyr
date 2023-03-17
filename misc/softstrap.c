/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <soc.h>
#include <zephyr/logging/log.h>
#include "pwrseq_timeouts.h"
#include "softstrap.h"
LOG_MODULE_DECLARE(ecfw, CONFIG_EC_LOG_LEVEL);

/* Need to declare in single structure to guarantee proper layout in binary
 * |-------------------------------|
 * |   MEC IMAGE HEADER            |
 * |-------------------------------|
 * |   IMAGE START                 |
 * |   ==> EC FW INFO TOKEN        |
 * |                               |
 * |   ==> SOFTSTRAP STRAP TOKEN   |
 * |       SOFTSTRAP METADATA      |
 * |       SOFTSTRAP PAYLOAD       |
 * |   IMAGE END                   |
 * |-------------------------------|
 * |   MEC HEADER HASH DIGEST      |
 * |-------------------------------|
 */
__in_section(softstrap, static, var) const struct softstrap_layout strp_reg = {
	/* This section should be updated whenever more sw_strp start been used
	 * across EC FW modules.
	 * Sofstrap application should never modify the header.
	 */
	.header = {
		.token = {'S', 'T', 'R', 'P'},
		.size = sizeof(struct softstrap_region),
		.major_version = SOFTSTRAP_MAJOR_VERSION,
		.minor_version = SOFTSTRAP_MINOR_VERSION,
		.support = {
			.feature_override = 0,
			.timeout_config = 1,
			.usbc_config = 0,
			.charger_config = 0,
			.dtt_config = 0,
			.debug = 0,
			.security = 0,
		}
	},
	/* Do not modify these structures.
	 * These are generated from softstrap spec document
	 */
	.straps = {
		.features = {0},
		.timeouts = {
			.rsm_rst_pwrgd = TIMEOUT_FROM_US(RSMRST_PWRDG_TIMEOUT),
			.espi_rst = TIMEOUT_FROM_US(ESPI_RST_TIMEOUT),
			.sus_wrn = TIMEOUT_FROM_US(SUSWARN_TIMEOUT),
			.slp_s5 = TIMEOUT_FROM_US(SLPS5_TIMEOUT),
			.slp_s4 = TIMEOUT_FROM_US(SLPS4_TIMEOUT),
			.slp_m = TIMEOUT_FROM_US(SLP_M_TIMEOUT),
			.all_sys_pwrg = TIMEOUT_FROM_US(ALL_SYS_PWRGD_TIMEOUT),
			.plt_rst = TIMEOUT_FROM_US(PLT_RESET_TIMEOUT),
		},
		/* Update as required when these strap blocks are supported */
		.usbc_cfg = { {0} },
		.chrg_cfg = { {0} },
		.spi_cfg = {0},
	},
};

/* Example of strap validation */
static void strap_validate_timeouts(void)
{
	const struct softstrap_header *soft_header = &strp_reg.header;
	const struct softstrap_region *strps = &strp_reg.straps;
	const struct timeout_params *time = &strps->timeouts;

	/* Shown as example only, this should never occur since UI tool
	 * shall guarantee it
	 */
	if (time->rsm_rst_pwrgd == 0) {
		LOG_WRN("Invalid SW strap RSMRST_PWRDG timeout");
	}

	LOG_DBG("SW timeout config supported by FW: %d",
		soft_header->support.timeout_config);
	LOG_DBG("RSM_RST_PWRGD timeout %d ms",
		TIMEOUT_TO_MS(strps->timeouts.rsm_rst_pwrgd));
	LOG_DBG("espi_rst timeout %d ms",
		TIMEOUT_TO_MS(strps->timeouts.espi_rst));
	LOG_DBG("PLT_RESET_TIMEOUT timeout %d ms",
		TIMEOUT_TO_MS(strps->timeouts.plt_rst));
	LOG_DBG("PLT_RESET_TIMEOUT timeout %d us",
		TIMEOUT_TO_US(strps->timeouts.plt_rst));
}

void strap_init(void)
{
	const struct softstrap_header *soft_header = &strp_reg.header;

	/* Debug data to verify specification */
	LOG_DBG("Softstrap header size: %d", sizeof(struct softstrap_header));
	LOG_DBG("Sofstrap payload size: %d", sizeof(struct softstrap_region));
	LOG_DBG("Platform features size: %d", sizeof(struct plat_features));
	LOG_DBG("Timeout config size: %d", sizeof(struct timeout_params));
	LOG_DBG("USBC config size: %d", sizeof(struct usbc_config));
	LOG_DBG("Charger config size: %d", sizeof(struct charger_config));
	LOG_DBG("SPI config size: %d", sizeof(struct spi_config));
	LOG_DBG("DTT config size: %d", sizeof(struct dtt_config));
	LOG_DBG("Dev config size: %d", sizeof(struct dev_config));
	LOG_DBG("Security config size: %d", sizeof(struct security_config));

	LOG_INF("Softstrap version: v%d.%d", soft_header->major_version,
		soft_header->minor_version);
	LOG_DBG("Softstrap encoded size: %d", soft_header->size);
	LOG_DBG("SW feature override supported by FW: %d",
		soft_header->support.feature_override);

	strap_validate_timeouts();
}

const struct softstrap_region *sw_strps(void)
{
	return &strp_reg.straps;
}
