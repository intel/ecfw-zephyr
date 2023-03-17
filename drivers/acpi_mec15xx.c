/*
 * Copyright (c) 2019 Intel Corporation
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

bool acpi_get_flag(enum acpi_ec_interface num, uint8_t type)
{
	ACPI_EC_Type *acpi_ec_regs =
	       (ACPI_EC_Type *)(ACPI_EC_0_BASE + num * ACPI_EC_STRUCT_SIZE);

	return acpi_ec_regs->EC_STS & type;
}

void acpi_set_flag(enum acpi_ec_interface num, uint8_t type, bool val)
{
	ACPI_EC_Type *acpi_ec_regs =
	       (ACPI_EC_Type *)(ACPI_EC_0_BASE + num * ACPI_EC_STRUCT_SIZE);

	if (val) {
		acpi_ec_regs->EC_STS |= type;
	} else {
		acpi_ec_regs->EC_STS &= ~type;
	}
}

uint8_t acpi_read_idr(enum acpi_ec_interface num)
{
	ACPI_EC_Type *acpi_ec_regs =
	       (ACPI_EC_Type *)(ACPI_EC_0_BASE + num * ACPI_EC_STRUCT_SIZE);

	return (acpi_ec_regs->OS2EC_DATA & OS2EC_LSBBYTE_MASK);
}

void acpi_write_odr(enum acpi_ec_interface num, uint8_t byte)
{
	ACPI_EC_Type *acpi_ec_regs =
	       (ACPI_EC_Type *)(ACPI_EC_0_BASE + num * ACPI_EC_STRUCT_SIZE);

	acpi_ec_regs->EC2OS_DATA = byte;
}

uint8_t acpi_read_str(enum acpi_ec_interface num)
{
	ACPI_EC_Type *acpi_ec_regs =
	       (ACPI_EC_Type *)(ACPI_EC_0_BASE + num * ACPI_EC_STRUCT_SIZE);

	return acpi_ec_regs->EC_STS;
}

void acpi_write_str(enum acpi_ec_interface num, uint8_t byte)
{
	ACPI_EC_Type *acpi_ec_regs =
	       (ACPI_EC_Type *)(ACPI_EC_0_BASE + num * ACPI_EC_STRUCT_SIZE);

	acpi_ec_regs->EC_STS = byte;
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

