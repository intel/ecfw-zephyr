/*
 * Copyright (c) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <soc.h>
#include <zephyr/logging/log.h>
#include "acpi.h"
#include "postcodemgmt.h"

LOG_MODULE_DECLARE(smchost, CONFIG_SMCHOST_LOG_LEVEL);

/* eSPI host subsystem node */
#define ESPI_HOST_NODE		DT_NODELABEL(host_sub)

struct pmch_reg *acpi_intfs[] = {
	(struct pmch_reg *) DT_REG_ADDR_BY_NAME(ESPI_HOST_NODE, pm_acpi),
	(struct pmch_reg *)DT_REG_ADDR_BY_NAME(ESPI_HOST_NODE, pm_acpi2),
};

bool acpi_get_flag(enum acpi_ec_interface num, uint8_t type)
{
	struct pmch_reg *pmc_ec_regs = acpi_intfs[num];

	return pmc_ec_regs->HIPMST & type;
}

void acpi_set_flag(enum acpi_ec_interface num, uint8_t type, bool val)
{

	struct pmch_reg *pmc_ec_regs = acpi_intfs[num];

	if (val) {
		pmc_ec_regs->HIPMST |= type;
	} else {
		pmc_ec_regs->HIPMST &= ~type;
	}
}

uint8_t acpi_read_idr(enum acpi_ec_interface num)
{
	struct pmch_reg *pmc_ec_regs = acpi_intfs[num];

	return pmc_ec_regs->HIPMDI;
}

void acpi_write_odr(enum acpi_ec_interface num, uint8_t byte)
{
	struct pmch_reg *pmc_ec_regs = acpi_intfs[num];

	pmc_ec_regs->HIPMDOC = byte;
}

uint8_t acpi_read_str(enum acpi_ec_interface num)
{
	struct pmch_reg *pmc_ec_regs = acpi_intfs[num];

	return pmc_ec_regs->HIPMST;
}

void acpi_write_str(enum acpi_ec_interface num, uint8_t byte)
{
	struct pmch_reg *pmc_ec_regs = acpi_intfs[num];

	pmc_ec_regs->HIPMST = byte;
}

/* TODO: Improve retry mechanism using k_yield and and k_busy_wait */
int acpi_send_byte(enum acpi_ec_interface num, uint8_t data)
{
	for (uint16_t i = 0; i < HOST_TIMEOUT; i++) {
		if (acpi_get_flag(num, ACPI_FLAG_OBF) == 1) {
			continue;
		}

		acpi_write_odr(num, data);
		return 0;
	}

	LOG_ERR("Host %d didn't consume the data %x. OBF always 1", num, data);
	return -1;
}

void acpi_ibf_intrpt_control(enum acpi_ec_interface num, bool en)
{
	/* TODO: Find associated interrupt controls*/
}
