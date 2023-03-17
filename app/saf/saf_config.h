/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SAF_CONFIG_H__
#define __SAF_CONFIG_H__

#include <soc.h>
#include <zephyr/drivers/espi_saf.h>

/**
 * @brief Performs eSPI controller initialization.
 *
 * Note: This operation should complete before eSPI flash channel negotiation
 * is started to avoid cases where eSPI master attempts flash operations
 * before SAF block is ready.
 *
 * It's recomended to perform this operation before RSMRST is de-asserted.
 *
 * @retval -EIO General input / output error, failed to configure device.
 * @retval -EINVAL invalid capabilities, failed to configure device.
 * @retval -ENOTSUP capability not supported by eSPI slave.
 * @retval 0 if success.
 */

int initialize_saf_bridge(void);

/**
 * @brief Sends a write request packet for slave attached flash.
 *
 * This routines provides an interface to send a request to write to the flash
 * components shared between the eSPI master and eSPI slaves.
 *
 * @param pckt Address of the representation of write flash transaction.
 *
 * @retval -ENOTSUP eSPI flash logical channel transactions not supported.
 * @retval -EBUSY eSPI flash channel is not ready or disabled by master.
 * @retval -EIO General input / output error, failed request to master.
 */
int saf_write_flash(struct espi_saf_packet *pckt);

/**
 * @brief Sends an erase request packet for slave attached flash.
 *
 * This routines provides an interface to send a request to erase a page in
 * the flash components shared between the eSPI master and eSPI slaves.
 *
 * @param pckt Address of the representation of erase flash transaction.
 *
 * @retval -ENOTSUP eSPI flash logical channel transactions not supported.
 * @retval -EBUSY eSPI flash channel is not ready or disabled by master.
 * @retval -EIO General input / output error, failed request to master.
 */
int saf_erase_flash(struct espi_saf_packet *pckt);

#endif /* __SAF_CONFIG_H__ */
