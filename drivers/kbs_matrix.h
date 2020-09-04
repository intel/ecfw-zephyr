/*
 * Copyright (c) 2020 Intel Corportation
 */

#ifndef SCAN_MATRIX_KB_H
#define SCAN_MATRIX_KB_H

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

typedef void (*kbs_matrix_callback)(u8_t *data, u8_t len);

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
int kbs_matrix_init(kbs_matrix_callback callback, u8_t *initial_set);

/**
 * @brief Write commands to kscan keyboard.
 *
 * This routine writes the typematic rate and delay data bytes to the
 * keyboard instance.
 *
 * @param data Byte value representing either command or data.
 */
void kbs_write_typematic(u8_t data);

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

#endif /* SCAN_MATRIX_KB */

