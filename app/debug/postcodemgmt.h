/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief APIs for POST code management
 */

#ifndef __POSTCODE_MGMT_H__
#define __POSTCODE_MGMT_H__

#define POSTCODE_PORT80    0
#define POSTCODE_PORT81    1

/**
 * @brief BIOS debug port debug management.
 *
 * This routine performs management BIOS Port80 debug.
 *
 * @param p1 pointer to additional task-specific data.
 * @param p2 pointer to additional task-specific data.
 * @param p2 pointer to additional task-specific data.
 */
void postcode_thread(void *p1, void *p2, void *p3);

/**
 * @brief Report error code reported in BIOS debug port.
 *
 * @param errcode the board error code identifier.
 */
void update_error(uint8_t errcode);

#endif /* __POSTCODE_MGMT_H__ */
