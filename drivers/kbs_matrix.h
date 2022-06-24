/*
 * Copyright (c) 2020 Intel Corportation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCAN_MATRIX_KB_H
#define SCAN_MATRIX_KB_H

#ifdef CONFIG_EARLY_KEY_SEQUENCE_DETECTION
#include "kbs_boot_keyseq.h"
#endif

#define KBS_NUMLOCK_DOWN	(1U << KBS_NUMLOCK_DOWN_POS)
#define KBS_SCLOCK_DOWN		(1U << KBS_SCLOCK_DOWN_POS)
#define KBS_FN_DOWN		(1U << KBS_FN_DOWN_POS)
#define KBS_CTRL_DOWN		(1U << KBS_CTRL_DOWN_POS)
#define KBS_ALT_DOWN		(1U << KBS_ALT_DOWN_POS)
#define KBS_SHIFT_DOWN		(1U << KBS_SHIFT_DOWN_POS)
#define KBS_WIN_DOWN		(1U << KBS_WIN_DOWN_POS)

#define KBS_NUMLOCK_DOWN_POS	0U
#define KBS_SCLOCK_DOWN_POS	1U
#define KBS_FN_DOWN_POS		2U
#define KBS_CTRL_DOWN_POS	3U
#define KBS_ALT_DOWN_POS	4U
#define KBS_SHIFT_DOWN_POS	5U
#define KBS_WIN_DOWN_POS	6U

typedef void (*kbs_matrix_callback)(uint8_t *data, uint8_t len);

/**
 * @brief Initialize kscan keyboard instance representing the keyboard.
 *
 * This routine receives a callback to notify keyboard events.
 *
 * @param kscan_callback Pointer to a function implemented in the caller code.
 *
 * @retval 0 if successful.
 * @retval negative on error code.
 */
int kbs_matrix_init(kbs_matrix_callback callback, uint8_t *initial_set);

/**
 * @brief Write commands to kscan keyboard.
 *
 * This routine writes the typematic rate and delay data bytes to the
 * keyboard instance.
 *
 * @param data Byte value representing either command or data.
 */
void kbs_write_typematic(uint8_t data);

/**
 * @brief Enable keyboard events from kscan driver.
 *
 * This routine allows to resume keyboard events.
 */
void kbs_keyboard_enable(void);

/**
 * @brief Disable keyboard events from kscan driver.
 *
 * This routine allows to stop receiving keyboard events.
 */
void kbs_keyboard_disable(void);

/**
 * @brief Set default settings for the scan matrix.
 *
 * This routine allows to reconfigure any default settings
 * for a matrix keyboard.
 */
void kbs_keyboard_set_default(void);

/**
 * @brief Detecting Hotkey press event.
 * This routine allows to detect the hotkey press event from kscan driver
 * @retval 1 on success.
 **/
bool kbs_is_hotkey_detected(void);

#ifdef CONFIG_EARLY_KEY_SEQUENCE_DETECTION
/**
 * @brief Check if any of supported hot key sequences was held at boot.
 *
 * @param keyseq_map the index in the key sequence map.
 * @retval true if predefined key sequence was pressed during boot,
 *              false otherwise.
 */
bool kbs_keyseq_boot_detect(enum kbs_keyseq_type index);

/**
 * @brief Define the runtime hot key sequence.
 *
 * @param modifiers a mask indicating the keyboard modifiers.
 * @param key_num the last key used in the key sequence.
 * @param callback used to notify observer.
 *
 * @retval 0 on success.
 * @retval -EINVAL if the runtime key sequence is already defined.
 */
int kbs_keyseq_define(uint8_t modifiers, uint8_t key,
		     kbs_key_seq_detected callback);

/**
 * @brief Register of notification from specific key sequence.
 *
 * @param keyseq_map the index in the key sequence map.
 * @param callback used to notify observer.
 *
 * @retval 0 on success.
 * @retval -EINVAL if a handler already registered for the key sequence.
 */
int kbs_keyseq_register(enum kbs_keyseq_type index,
			kbs_key_seq_detected callback);

#endif /* #ifdef CONFIG_EARLY_KEY_SEQUENCE_DETECTION */

#endif /* SCAN_MATRIX_KB */

