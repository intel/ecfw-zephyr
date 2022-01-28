/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __ACPI_H__
#define __ACPI_H__

#define ACPI_FLAG_OBF        0x01
#define ACPI_FLAG_IBF        0x02
#define ACPI_INTR_ERRI       0x04
#define ACPI_INTR_IBF        0x05
#define ACPI_FLAG_CD         0x08
#define ACPI_FLAG_LRST       0x09
#define ACPI_FLAG_ACPIBURST  0x10
#define ACPI_FLAG_SWDN       0x0A
#define ACPI_FLAG_ABRT       0x0B
#define ACPI_FLAG_SCIEVENT   0x20
#define ACPI_FLAG_SMIEVENT   0x40
#define ACPI_FLAG_IRQ1       0xF1
#define ACPI_FLAG_IRQ12      0xF2

#define EC_READ              0x80
#define EC_WRITE             0x81
#define EC_BURST             0x82
#define EC_NORM              0x83
#define EC_QUERY             0x84

/** Number of loops to wait for host in burst */
#define HOST_TIMEOUT         600ul

enum acpi_ec_interface {
	ACPI_EC_0,
	ACPI_EC_1,
};

/**
 * @brief Get ACPI flag.
 *
 * @param type the flag to be retrieved.
 */
bool acpi_get_flag(enum acpi_ec_interface num, uint8_t type);

/**
 * @brief Set ACPI flag.
 *
 * @param type the flag to be set.
 * @param hilow indicates if flag is to be set 1-HIGH or 0-LOW.
 */
void acpi_set_flag(enum acpi_ec_interface num, uint8_t type, bool hilow);

/**
 * @brief Read data sent from OS to EC.
 *
 * @retval the byte sent.
 */
uint8_t acpi_read_idr(enum acpi_ec_interface num);

/**
 * @brief Send data from EC to OS.
 *
 * @param byte the data to be sent to OS.
 */
void acpi_write_odr(enum acpi_ec_interface num, uint8_t byte);

/**
 * @brief Read status from I/F block.
 *
 * @retval data sent from OS to EC.
 */
uint8_t acpi_read_str(enum acpi_ec_interface num);

int acpi_send_byte(enum acpi_ec_interface num, uint8_t data);

#endif /* __ACPI_H__ */
