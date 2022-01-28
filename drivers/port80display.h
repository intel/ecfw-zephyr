/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief APIs for Port80 display
 */

#ifndef __PORT80_DISPLAY_H__
#define __PORT80_DISPLAY_H__

/**
 * @brief This routine initializes onboard 7-segment display array.
 *
 * @retval -EIO General input / output error, failed to configure device.
 */
int port80_display_init(void);

/**
 * @brief Turn the display on.
 *
 * @retval -EIO General input / output error, failed to configure device.
 */
int port80_display_on(void);

/**
 * @brief Turn the display off.
 *
 * @retval -EIO General input / output error, failed to configure device.
 */
int port80_display_off(void);

/**
 * @brief display a 4-digits word into onboard 7-segment display array.
 *
 * @param word the hexadecimal value to be displayed.
 */
void port80_display_word(uint16_t word);


#endif /* __PORT80_DISPLAY_H__ */
