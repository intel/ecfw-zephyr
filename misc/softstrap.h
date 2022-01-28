/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SOFTSTRAP_H__
#define __SOFTSTRAP_H__

/* Sofstrap specification version implemented */
#define SOFTSTRAP_MAJOR_VERSION    1
#define SOFTSTRAP_MINOR_VERSION    0

/* Sofstrap timeout value are specified in 500 milliseconds units.
 * i.e. 1000 ms corresponds to value 0x02
 * Minimum timeout programmable is 500 ms [0x1]
 * Maximum timeout programmable is 12.75s [0xf]
 *
 * Power sequencing module uses timeout specified in 100us periods hence
 * need to translate between both encoded values.
 */
#define PWR_100US_PERIODS_IN_1MS  10
#define TIMEOUT_FROM_MS(x)        (x/500)
#define TIMEOUT_FROM_US(x)        TIMEOUT_FROM_MS(x / PWR_100US_PERIODS_IN_1MS)

/* Returns value in ms to display if required*/
#define TIMEOUT_TO_MS(x)          (x*500)
#define TIMEOUT_TO_US(x)          TIMEOUT_TO_MS(x * PWR_100US_PERIODS_IN_1MS)

/**
 * @brief The software strap region is divided in multiple blocks.
 * This blocks indicate if feature are supported by FW or not.
 */
struct block_control {
	uint8_t feature_override:1;
	uint8_t timeout_config:1;
	uint8_t usbc_config:1;
	uint8_t charger_config:1;
	uint8_t dtt_config:1;
	uint8_t debug:1;
	uint8_t security:1;
	uint8_t rsvd:1;
} __attribute__((__packed__));

/**
 * @brief Metadata for sofstrap region.
 * Allow to detect compatibility with application and field modifications.
 */
struct softstrap_header {
	uint8_t token[4];
	uint8_t size;
	uint8_t major_version;
	uint8_t minor_version;
	struct block_control support;
};

enum feature_override {
	NO_OVERRIDE,
	RESERVED,
	OVERRIDE_ENABLED,
	OVERRIDE_DISABLED,
};

/**
 * @brief Contain enable/disable options for each one of the main features.
 */
struct plat_features {
	uint8_t battery_override:2;
	uint8_t pmic_override:2;
	uint8_t kbd_override:2;
	uint8_t vt_batt_override:2;
	uint8_t vt_dock_override:2;
	uint8_t lid_switch_override:2;
	uint8_t sys_pwr_override:2;
	uint8_t load_switch_override:2;
	uint8_t imvp_control:2;
	uint8_t lpc_override:2;
	uint8_t psg3_override:2;
	uint8_t usbc_pd_override:2;
	uint8_t timeout_override:2;
	uint8_t ps2_mouse_override:2;
	uint8_t ps2_kbd_override:2;
	uint8_t fan_override:2;
	uint8_t retimer_override:2;
	uint8_t debug_override:2;
	uint8_t usbc_boot_override:2;
	uint8_t pwr_src_override:2;
	uint8_t bios_logs_override:2;
	uint8_t rsmrst_override:2;
	uint8_t peci_override:2;
	uint8_t rsvd1[2];
} __attribute__((__packed__));

/**
 * @brief Contains timeout values encoded in 500 ms units.
 */
struct timeout_params {
	uint8_t rsm_rst_pwrgd:4;
	uint8_t espi_rst:4;
	uint8_t sus_wrn:4;
	uint8_t slp_s5:4;
	uint8_t slp_s4:4;
	uint8_t slp_m:4;
	uint8_t all_sys_pwrg:4;
	uint8_t plt_rst:4;
} __attribute__((__packed__));

struct usbc_config {
	uint32_t reserved[2];
} __attribute__((__packed__));

struct charger_config {
	uint32_t reserved[5];
} __attribute__((__packed__));

struct spi_config {
	uint32_t reserved;
} __attribute__((__packed__));

struct dtt_config {
	uint32_t reserved;
} __attribute__((__packed__));

struct dev_config {
	uint32_t reserved;
} __attribute__((__packed__));

struct security_config {
	uint32_t reserved;
} __attribute__((__packed__));

/**
 * @brief EC FW sofstraps values.
 */
struct softstrap_region {
	struct plat_features   features;
	struct timeout_params  timeouts;
	struct usbc_config     usbc_cfg;
	struct charger_config  chrg_cfg;
	struct spi_config      spi_cfg;
	struct dtt_config      dtt_cfg;
	struct dev_config      dev_cfg;
	struct security_config sec_cfg;
} __attribute__((__packed__));

/**
 * @brief EC FW sofstraps layout.
 */
struct softstrap_layout {
	struct softstrap_header  header;
	struct softstrap_region  straps;
} __attribute__((__packed__));


/**
 * @brief Peform any checks required in strap region data.
 * Display metadata and main values.
 */
void strap_init(void);

/**
 * @brief Allows access to softstrap values.
 *
 * @return a non-modifiable reference to sofstrap layout.
 */
const struct softstrap_region *sw_strps(void);

#endif /* __SOFTSTRAP_H__ */
