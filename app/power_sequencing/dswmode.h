/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef __DSW_MODE_H__
#define __DSW_MODE_H__

/* DSW mode disabled. */
#define DSW_DISABLED                  0x00
/* DSW enabled in S5/battery-only */
#define ENABLE_IN_S5_DC               0x01
/* DSW enabled in S5/always on */
#define ENABLE_IN_S5_ACDC             0x02
/* DSW enabled in S4-S5/battery-only */
#define ENABLE_IN_S4S5_DC             0x03
/* DSW enabled in S4-S5/always on */
#define ENABLE_IN_S4S5_ACDC           0x04
/* DSW enabled in S3-S5/DC */
#define ENABLE_IN_S3S4S5_DC           0x05
/* FF-DSW enabled in S4-S5/battery-only */
#define ENABLE_FF_IN_S4S5_DC          0x06
/* FF-DSW enabled in S3-S5/battery-only */
#define ENABLE_FF_IN_S3S4S5_DC        0x07

/* EEPROM offset to save DSW setting */
#define EEPROM_DSW_OFFSET             0x06

/**
 * @brief Indicate if DeepSx support is enabled.
 *
 * @retval true if DeepSx support is enabled, false otherwise.
 */
bool dsw_enabled(void);

/**
 * @brief Indicate which DeepSx mode is enabled.
 *
 * @retval the Deep Sx mode enabled, 0 if none is disabled.
 */
uint8_t dsw_mode(void);

/**
 * @brief Update DeepSx mode if different than current.
 *
 * @param the new Deep Sx mode.
 */
void dsw_update_mode(uint8_t mode);

/**
 * @brief Read Deep Sx mode from EEPROM.
 *
 */
void dsw_read_mode(void);

/**
 * @brief Save Deep Sx mode into EEPROM.
 *
 */
void dsw_save_mode(void);

#endif /* __DSW_MODE_H__ */
