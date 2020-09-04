/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

/**
 * @brief System power states as defined in ACPI spec chapter 2-2.
 */
enum system_power_state {
	/* Mechanical off. System is completely off and consumes no power */
	SYSTEM_G3_STATE,
	/* Working state. System is fully active */
	SYSTEM_S0_STATE,
	/* Sleep. System consumes a small amount of power, nothing is executing
	 * Latency to go back to working state is minimal without reboot system
	 */
	SYSTEM_S3_STATE,
	/* Non-volatile sleep. This special state allows system context to be
	 * saved and restored (relatively slowly) when power is lost
	 */
	SYSTEM_S4_STATE,
	/* Soft off. System consumes a minimal amount of power.
	 * Requires a large latency in order to return to working state, usually
	 * requires a complete re-initialization
	 * The system’s context will not be preserved by the hardware
	 */
	SYSTEM_S5_STATE,
};

enum boot_config_mode {
	/* SPI sharing. FW is obtained directly from SPI flash but EC HW
	 * may relinquishe its access to SPI flash after obtaining its FW.
	 * Other entities in the system obtain their FW from SPI flash.
	 */
	FLASH_BOOT_MODE_G3_SHARING = 0,
	/* Master-attached flash. Intel SoC is connected to SPI flash,
	 * EC HW is not connected to SPI flash, EC FW is obtained through
	 * eSPI bus.
	 */
	FLASH_BOOT_MODE_MAF,
	/* Slave-attached flash. EC HW is connected to SPI flash,
	 * EC has exclusive access to SPI flash.
	 * Other components obtain their FW via eSPI.
	 */
	FLASH_BOOT_MODE_SAF,
	/* EC HW has a dedicated SPI flash. */
	FLASH_BOOT_MODE_OWN,
};

#endif /* __SYSTEM_H__ */
