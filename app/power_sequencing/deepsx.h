/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __DEEP_SX_H__
#define __DEEP_SX_H__

/**
 * @brief Manage Deep S3, if still in DeepS3 then exit.
 *
 * @retval true if system is in deep S3, false otherwise.
 */
bool manage_deep_s3(void);

/**
 * @brief Manage Deep S4/S5, if still in Deep S4/S5 then exit.
 *
 * @retval true if system is in deep S4/S5, false otherwise.
 */
bool manage_deep_s4s5(void);

/**
 * @brief Check if conditions to enter Deep S5 while on battery are met.
 *
 * @retval true if conditions are met.
 */
bool check_s5_battonly_condition(void);

/**
 * @brief Check if conditions to enter Deep S5 while on AC are met.
 *
 * @retval true if conditions are met.
 */
bool check_s5_alwayson_condition(void);

/**
 * @brief Check if conditions to enter Deep S4/S5 while on battery are met.
 *
 * @retval true if conditions are met.
 */
bool check_s4s5_battonly_condition(void);

/**
 * @brief Check if conditions to enter Deep S4/S5 while on AC are met.
 *
 * @retval true if conditions are met.
 */
bool check_s4s5_alwayson_condition(void);

/**
 * @brief Enter Deep Sx.
 *
 */
void deep_sx_enter(void);

/**
 * @brief Exit Deep Sx.
 *
 */
void deep_sx_exit(void);

/**
 * @brief Indicate if system is in deep sleep or not.
 *
 * @retval true if all conditions were met and final handshake completed,
 * false otherwise.
 */
bool dsx_entered(void);

#endif /* __DEEP_SX_H__ */
