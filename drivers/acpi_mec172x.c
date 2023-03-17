/*
 * Copyright (c) 2021 Intel Corporation
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

#define OS2EC_LSBBYTE_MASK    0xFF
#define ACPI_EC_STRUCT_SIZE	0x400

static uintptr_t regbase[] = {
	DT_REG_ADDR(DT_NODELABEL(acpi_ec0)),
	DT_REG_ADDR(DT_NODELABEL(acpi_ec1)),
	DT_REG_ADDR(DT_NODELABEL(acpi_ec2)),
	DT_REG_ADDR(DT_NODELABEL(acpi_ec3)),
};

bool acpi_get_flag(enum acpi_ec_interface num, uint8_t type)
{
	struct acpi_ec_regs *regs = (struct acpi_ec_regs *)regbase[num];

	return regs->EC_STS & type;
}

void acpi_set_flag(enum acpi_ec_interface num, uint8_t type, bool val)
{
	struct acpi_ec_regs *regs = (struct acpi_ec_regs *) regbase[num];

	if (val) {
		regs->EC_STS |= type;
	} else {
		regs->EC_STS &= ~type;
	}
}

uint8_t acpi_read_idr(enum acpi_ec_interface num)
{
	struct acpi_ec_regs *regs = (struct acpi_ec_regs *) regbase[num];

	return (regs->OS2EC_DATA & OS2EC_LSBBYTE_MASK);
}

void acpi_write_odr(enum acpi_ec_interface num, uint8_t byte)
{
	struct acpi_ec_regs *regs = (struct acpi_ec_regs *)regbase[num];

	regs->EC2OS_DATA = byte;
}

uint8_t acpi_read_str(enum acpi_ec_interface num)
{
	struct acpi_ec_regs *regs = (struct acpi_ec_regs *)regbase[num];

	return regs->EC_STS;
}

void acpi_write_str(enum acpi_ec_interface num, uint8_t byte)
{
	struct acpi_ec_regs *regs =  (struct acpi_ec_regs *)regbase[num];

	regs->EC_STS = byte;
}

int acpi_send_byte(enum acpi_ec_interface num, uint8_t data)
{
	for (uint16_t i = 0; i < HOST_TIMEOUT; i++) {

		if (acpi_get_flag(num, ACPI_FLAG_OBF) == 1) {
			continue;
		}

		acpi_write_odr(num, data);
		return 0;
	}
	return -1;
}
