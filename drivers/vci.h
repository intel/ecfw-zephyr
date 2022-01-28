/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief VBAT-powered control interface APIs.
 *
 */

#ifndef __VCI_H__
#define __VCI_H__

/* VCI wake reason requires following pin connections to EC HW
 * VCI_IN0     - Power button input to EC
 * VCI_IN1     - Battery ID
 * VCI_OVRD_IN - Buck boost charger output
 *
 * VCI_IN0     - Power button output towards PCH
 */
enum vci_wake_source {
	VCI_UNKNOWN,
	VCI_POWER_BUTTON,
	VCI_BATTERY,
	VCI_AC,
	VCI_RTC,
};

/**
 * @brief Enable VBAT-powered HW.
 *
 * This does arm VCI block shown in Figure 41-2 for microchip hw, which allow
 * to wake the platform even if FW is not running.
 *
 * This enables only the wake sources supported.
 */
void vci_enable(void);

/**
 * @brief Disable VBAT-powered HW.
 */
void vci_disable(void);

/**
 * @brief Get VBAT-powered control interface wake source.
 *
 * @retval the source that trigger the wake.
 */
enum vci_wake_source vci_wake_reason(void);

/**
 * @brief Disable battery-powered gpio so they can be used as regular gpios.
 *
 */
void bgpo_disable(void);

#endif /* __VCI_H__ */
