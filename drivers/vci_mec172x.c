/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include "board.h"
#include "vci.h"
#include "soc.h"

/* Set bits 0,1 and 2 to enable BGPO0, BGPO1 and BGPO2 */
#define BGPO_EN_MASK	0x7U

static uintptr_t vci_regbase = DT_REG_ADDR(DT_NODELABEL(vci0));
static uintptr_t wktmr_regbase = DT_REG_ADDR(DT_NODELABEL(weektmr0));

/* VCI.Config bit[7] does not exist on MEC172x
 * Empty implementations for now to avoid app changes
 */

void vci_enable(void)
{
}

void vci_disable(void)
{
}

enum vci_wake_source vci_wake_reason(void)
{
	struct vci_regs *VCI_REGS = (struct vci_regs *)vci_regbase;

	if (VCI_REGS->CONFIG & MCHP_VCI_CFG_IN0_HI) {
		return VCI_POWER_BUTTON;
	} else if (VCI_REGS->CONFIG & MCHP_VCI_CFG_IN1_HI) {
		return VCI_BATTERY;
	} else if (VCI_REGS->CONFIG & MCHP_VCI_VCI_OVRD_IN_HI) {
		return VCI_AC;
	}

	return VCI_UNKNOWN;
}

void bgpo_disable(void)
{
	struct wktmr_regs *regs = (struct wktmr_regs *)wktmr_regbase;

	uint32_t data = regs->BGPO_PWR;
	/* Clear mask to disable BGPO */
	data &= ~(BGPO_EN_MASK);
	regs->BGPO_PWR = data;
}

