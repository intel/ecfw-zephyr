/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include "board.h"
#include "vci.h"
#include "soc.h"

/* Set bits 0,1 and 2 to enable BGPO0, BGPO1 and BGPO2 */
#define BGPO_EN_MASK	0x7U

void vci_enable(void)
{
	/* Set polarity for VCI_IN0/1 as active low */
	VCI_REGS->POLARITY &= ~MCHP_VCI_POL_ACT_HI_IN0;
	VCI_REGS->POLARITY &= ~MCHP_VCI_POL_ACT_HI_IN1;

	/* Gate VCI_IN1 and Un-Gate the VCI_IN0/VCI_OVRD_IN */
	/* TODO:Check with MCHP for VCI_OVRD_IN */
	VCI_REGS->INPUT_EN |= (MCHP_VCI_INPUT_EN_IN0 | BIT(8));
	VCI_REGS->INPUT_EN &= ~MCHP_VCI_INPUT_EN_IN1;

	/* Reset the VCI_IN# latches before enable */
	VCI_REGS->LATCH_RST |= (MCHP_VCI_LR_IN0 | MCHP_VCI_LR_IN1);
	/* Enable latches on VCI_IN0 so that assertions will be latched
	 * until next reset
	 */
	VCI_REGS->LATCH_EN |= (MCHP_VCI_LE_IN0 | MCHP_VCI_LE_IN1);

	/* Enable Input Filters on VCI_IN# pins */
	VCI_REGS->CONFIG &= ~MCHP_VCI_FW_EXT_SEL;
	VCI_REGS->CONFIG &= ~MCHP_VCI_FILTER_BYPASS;

	VCI_REGS->CONFIG |= MCHP_VCI_CFG_IN0_HI;
	VCI_REGS->CONFIG |= MCHP_VCI_CFG_IN1_HI;

	VCI_REGS->BUFFER_EN &= ~MCHP_VCI_BEN_IN0;
	VCI_REGS->BUFFER_EN &= ~BIT(8);
	/* Configure VCI_OUT for VBAT powered */
	VCI_REGS->CONFIG |= MCHP_VCI_CFG_VPWR_VBAT;
}

void vci_disable(void)
{
	/* Configure VCI_OUT for VTR powered */
	VCI_REGS->CONFIG &= ~MCHP_VCI_CFG_VPWR_VBAT;
}

enum vci_wake_source vci_wake_reason(void)
{
	if (VCI_REGS->CONFIG & MCHP_VCI_CFG_IN0_HI) {
		return VCI_POWER_BUTTON;
	} else if (VCI_REGS->CONFIG & MCHP_VCI_CFG_IN1_HI) {
		return VCI_BATTERY;
	} else if (VCI_REGS->CONFIG & MCHP_VCI_VCI_OVRD_IN_PIN) {
		return VCI_AC;
	}

	return VCI_UNKNOWN;
}

void bgpo_disable(void)
{
	uint32_t data = WKTMR_REGS->BGPO_PWR;
	/* Clear mask to disable BGPO */
	data &= ~(BGPO_EN_MASK);
	WKTMR_REGS->BGPO_PWR = data;
}
