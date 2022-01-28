/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief APIs for PS/2 keyboard and mouse.
 */


#ifndef __KEYBOARD_API_H__
#define __KEYBOARD_API_H__

typedef void (*ps2_callback)(uint8_t data);

/**
 * @brief Initialize PS/2 instance representing the keyboard.
 *
 * This routine receives a callback to notify keyboard events.
 *
 * @param ps2_callback Pointer to a function implemented in the caller code.
 *
 * @retval 0 if successful.
 * @retval negative on error code.
 */
int ps2_keyboard_init(ps2_callback callback, uint8_t *initial_set);

/**
 * @brief Write commands or data to PS/2 Keyboard.
 *
 * This routine writes data bytes to the keyboard instance.
 *
 * @param data Byte value representing either command or data.
 */
void ps2_keyboard_write(uint8_t data);

/**
 * @brief Disable the PS/2 instance representing the keyboard.
 *
 * This routine disables callbacks from the keyboard. Its use is
 * important during Windows shutdown and keyboard initialization sequence.
 */
void ps2_keyboard_disable(void);

/**
 * @brief Enable the PS/2 instance representing the keyboard.
 *
 * This routine enables callbacks from the keyboard. Its use is
 * important during Windows initialization sequence.
 */
void ps2_keyboard_enable(void);

/**
 * @brief Initialize PS/2 instance representing the mouse.
 *
 * This routine receives a callback to notify mouse events.
 *
 * @param ps2_callback Pointer to a function implemented in the caller code.
 *
 * @retval 0 if successful.
 * @retval negative on error code.
 */
int ps2_mouse_init(ps2_callback callback);

/**
 * @brief Write commands or data to PS/2 mouse.
 *
 * This routine writes data bytes to the mouse.
 *
 * @param ps2_callback Pointer to a function implemented in the caller code.
 */
void ps2_mouse_write(uint8_t data);

/**
 * @brief Disable the PS/2 instance representing the mouse.
 *
 * This routine disables callbacks from the mouse. Its use is
 * important during Windows shutdown and mouse initialization sequence.
 */
void ps2_mouse_disable(void);

/**
 * @brief Enable the PS/2 instance representing the mouse.
 *
 * This routine enables callbacks from the mouse. Its use is
 * important during Windows initialization sequence.
 */
void ps2_mouse_enable(void);

#endif /* __KEYBOARD_API_H__ */

