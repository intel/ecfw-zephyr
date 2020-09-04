/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __FLASH_IMG_HDR_H__
#define __FLASH_IMG_HDR_H__

struct ksc_img_hdr {
	u16_t checksum;
	u8_t signature[4];
	u8_t version[4];
	u8_t copyright[0x60 - 10];
	u32_t img_size;
	u8_t platform_str[8];
	u16_t platform_id[74];
};

/**
 * @brief Gets EC FW major version.
 *
 * @retval byte representing the EC FW major version.
 */
u8_t major_version(void);

/**
 * @brief Gets EC FW minor version.
 *
 * @retval byte representing the EC FW minor version.
 */
u8_t minor_version(void);

#ifdef CONFIG_ESPI_PERIPHERAL_HOST_IO_PVT
/**
 * @brief Gets EC FW build version.
 *
 * @retval byte representing the EC FW build version.
 */
u8_t build_version(void);

/**
 * @brief Gets the platform ID.
 *
 * @retval byte representing the platform ID.
 */
u8_t platform_id(void);
#endif

#endif /* __FLASH_IMG_HDR_H__ */
