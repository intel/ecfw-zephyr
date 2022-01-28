/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __KBCHOST_H__
#define __KBCHOST_H__

/* port 0x64 keyboard controller commands.
 * Note : This commands are processed by the emulated keyboard controller
 */
#define KBC_8042_READ_CMD_BYTE		0x20U
/* expect a byte on port 0x60 */
#define KBC_8042_WRITE_CMD_BYTE		0x60U
#define KBC_8042_TEST_PASSWORD		0xa4U
#define KBC_8042_DIS_MOUSE		0xa7U
#define KBC_8042_ENA_MOUSE		0xa8U
#define KBC_8042_TEST_MOUSE		0xa9U
#define KBC_8042_RESET_SELF_TEST	0xaaU
#define KBC_8042_TEST_KB_PORT		0xabU
#define KBC_8042_DIAGNOSTIC_DUMP	0xacU
#define KBC_8042_DIS_KB			0xadU
#define KBC_8042_ENA_KB			0xaeU
#define KBC_8042_READ_INPUT_PORT	0xe0U
#define KBC_8042_INS_HOTKEY_EVENT	0xc3U
#define KBC_8042_READ_OUTPUT_PORT	0xd0U
#define KBC_8042_WRITE_OUTPUT_PORT	0xd1U
#define KBC_8042_WRITE_KBD_OUTPUT_REG	0xd2U
/* expect a byte on port 0x60 */
#define KBC_8042_ECHO_MOUSE		0xd3U
/* expect a byte on port 0x60 */
#define KBC_8042_SEND_TO_MOUSE		0xd4U
/* CPU reset through ESPI VW*/
#define KBC_8042_PULSE_OUTPUT		0xfeU

/* port 0x60 keyboard commands */
#define KBC_8042_ECHO_KEYBOARD		0xeeU
#define KBC_8042_SET_LEDS		0xedU
#define KBC_8042_SET_GET_SCANCODE	0xf0U
#define KBC_8042_READ_ID		0xf2U
#define KBC_8042_SET_TYPEMATIC_RATE	0xf3U
#define KBC_8042_EN_KEYBOARD		0xf4U
#define KBC_8042_DEFAULT_DIS		0xf5U
#define KBC_8042_SET_DEFAULT		0xf6U
#define KBC_8042_RESEND			0xfeU
#define KBC_8042_RESET			0xffU

/* port 0x60 return values */
#define KBC_8042_ACK			0xfaU
#define KBC_8042_NACK			0xfeU
#define KBC_8042_BAT			0xaaU
#define KBC_8042_MOUSE_ID		0U

/* Flags for the "command byte" which is located in address 0x20 in ancient
 * KBCs. In the EC case we use a simmple variable for book-keeping.
 */
#define KBC_8042_EN_KBD_IRQ		(1 << KBC_8042_EN_KBD_IRQ_POS)
#define KBC_8042_EN_MOUSE_IRQ		(1 << KBC_8042_EN_MOUSE_IRQ_POS)
#define KBC_8042_HOST_SYS_FLAG		(1 << KBC_8042_HOST_SYS_FLAG_POS)
#define KBC_8042_KBD_DIS		(1 << KBC_8042_KBD_DIS_POS)
#define KBC_8042_MOUSE_DIS		(1 << KBC_8042_MOUSE_DIS_POS)
#define KBC_8042_TRANSLATE		(1 << KBC_8042_TRANSLATE_POS)

/* Keyboard controller command byte offsets */
#define KBC_8042_EN_KBD_IRQ_POS		0U
#define KBC_8042_EN_MOUSE_IRQ_POS	1U
#define KBC_8042_HOST_SYS_FLAG_POS	2U
#define KBC_8042_KBD_DIS_POS		4U
#define KBC_8042_MOUSE_DIS_POS		5U
#define KBC_8042_TRANSLATE_POS		6U

#define NO_PASSWORD			0xf1
#define PASSWORD			0xfa
#define TEST_PASSED			0x55

#define KBC_8042_DEFAULT_SCAN_CODE	2U

/* Extract led value */
#define SCROLL_LOCK_POS			0U
#define NUM_LOCK_POS			1U
#define CAPS_LOCK_POS			2U
#define GET_SCROLL_LOCK(x)		(((x) >> SCROLL_LOCK_POS) & 0x1)
#define GET_NUM_LOCK(x)			(((x) >> NUM_LOCK_POS) & 0x1)
#define GET_CAPS_LOCK(x)		(((x) >> CAPS_LOCK_POS) & 0x1)

#define KBC_LED_FLASH_DELAY_MS		250U

/**
 * @brief Report IBF data when registered with the eSPI driver.
 *
 * This routine receives IBF information from the emulated 8042 block. The
 * IBF information can be command or data.
 *
 * @param data This is what comes from the host through eSPI(port 60 or 64).
 * @param cmd_data The parameter received from the host is a command when
 * the value is 1, and 0 indicates data was received.
 */
void kbc_handler(uint8_t data, uint8_t cmd_data);

/**
 * @brief Disable the IBF callback.
 *
 * This routine suspends IBF callback events from the host.
 * @retval 0 if successful.
 * @retval negative on error code.
 */
int kbc_disable_interface(void);

/**
 * @brief Enable the IBF callback.
 *
 * This routine resumes IBF callback events from the host.
 * @retval 0 if successful.
 * @retval negative on error code.
 */
int kbc_enable_interface(void);

/**
 * @brief Control the keyboard related RVP leds.
 *
 * This routine is used for setting the 3 leds on an RVP.
 * @param leds Every bit represents a led state.
 * Note :
 * Bit 0 contains the value of Scroll Lock.
 * Bit 1 contains the value of Num Lock.
 * Bit 2 contains the value of Caps Lock.
 */
void kbc_set_leds(uint8_t leds);

/**
 * @brief Retrieve the latest led state from an RVP.
 *
 * This routine queries the state of the 3 leds.
 * Note :
 * Bit 0 contains the value of Scroll Lock.
 * Bit 1 contains the value of Num Lock.
 * Bit 2 contains the value of Caps Lock.
 *
 * @retval led state.
 */
int kbc_get_leds(void);

void to_from_host_thread(void *p1, void *p2, void *p3);
void to_host_kb_thread(void *p1, void *p2, void *p3);

#endif /* __KBCHOST_H__ */
