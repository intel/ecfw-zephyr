/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __FLASH_IMG_HDR_H__
#define __FLASH_IMG_HDR_H__

struct ksc_img_hdr {
	uint16_t checksum;
	uint8_t signature[4];
	uint8_t version[4];
	uint8_t copyright[0x60 - 10];
	uint32_t img_size;
	uint8_t platform_str[8];
	uint16_t platform_id[74];
};

/**
 * @brief Gets EC FW major version.
 *
 * @retval byte representing the EC FW major version.
 */
uint8_t major_version(void);

/**
 * @brief Gets EC FW minor version.
 *
 * @retval byte representing the EC FW minor version.
 */
uint8_t minor_version(void);

/**
 * @brief Gets the patch ID.
 *
 * @retval byte representing the patch ID.
 */
uint8_t patch_id(void);

/**
 * @brief Gets EC FW QS build version.
 *
 * @retval byte representing the EC FW QS build version.
 */
uint8_t qs_build_version(void);

#endif /* __FLASH_IMG_HDR_H__ */
