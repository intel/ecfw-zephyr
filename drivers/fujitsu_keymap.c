/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kbs_keymap.h"
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

/* Below is the keymap for the fujitsu keyboard we borrowed from MCHP */

/****************************************************************************/
/*  Fujitsu keyboard model N860-7401-TOO1                                   */
/*                                                                          */
/*  Sense7  Sense6  Sense5  Sense4  Sense3  Sense2  Sense1  Sense0          */
/*+---------------------------------------------------------------+         */
/*|       | Capslk|       |   1!  |  Tab  |   F1  |   `~  |       | Scan  0 */
/*|       |  (30) |       |  (2)  |  (16) | (112) |  (1)  |       | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|       |   W   |   Q   |   F7  |       |  Esc  |   F6  |   F5  | Scan  1 */
/*|       |  (18) |  (17) | (118) |       | (110) | (117) | (116) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|       |   F8  |       |   2@  |   F4  |   F3  |       |   F2  | Scan  2 */
/*|       | (119) |       |  (3)  | (115) | (114) |       | (113) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|       |   R   |   E   |   3#  |   4$  |   C   |   F   |   V   | Scan  3 */
/*|       |  (20) |  (19) |  (4)  |  (5)  |  (48) |  (34) |  (49) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|   N   |   Y   |   6^  |   5%  |   B   |   T   |   H   |   G   | Scan  4 */
/*|  (51) |  (22) |  (7)  |  (6)  | (50)  |  (21) |  (36) |  (35) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*| SpaceB|   M   |   A   |   7&  |   J   |   D   |   S   |   U   | Scan  5 */
/*|  (61) |  (52) |  (31) |  (8)  |  (37) |  (33) |  (32) |  (23) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|   F9  |   I   |   ,<  |   8*  |       |   Z   |   X   |   K   | Scan  6 */
/*| (120) |  (24) |  (53) |  (9)  |       |  (46) |  (47) |  (38) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|       |   =+  |   ]}  |   9(  |   L   |       |  CRSL |   O   | Scan  7 */
/*|       |  (13) |  (28) |  (10) |  (39) |       |  (79) |  (25) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|   -_  |   0)  |   /?  |   [{  |   ;:  |       |       |   '"  | Scan  8 */
/*|  (12) |  (11) |  (55) |  (27) |  (40) |       |       |  (41) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|  F10  | Pause | NumLK |   P   |       |       |       |       | Scan  9 */
/*| (121) | (126) |  (90) |  (26) |       |       |       |       | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*| BkSpac|   \|  |  F11  |   .>  |       |       | W-Appl|  CRSD | Scan 10 */
/*|  (15) |  (29) | (122) |  (54) |       |       |  (71) |  (84) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*| Enter | Delete| Insert|  F12  |       |       |  CRSU |  CRSR | Scan 11 */
/*|  (43) |  (76) |  (75) | (123) |       |       |  (83) |  (89) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|       | R-WIN |       |       |       |       | L-WIN |   Fn  | Scan 12 */
/*|       |  (127 |       |       |       |       | (127) | (255) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|       |       |       | RShift| LShift|       |       |       | Scan 13 */
/*|       |       |       |  (57) |  (44) |       |       |       | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|       |       |       |       |       |       | L Alt | R Alt | Scan 14 */
/*|       |       |       |       |       |       |  (60) |  (62) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*| R Ctrl|       |       |       |       | L Ctrl|       |       | Scan 15 */
/*|  (64) |       |       |       |       |  (58) |       |       | (KEY #) */
/*+---------------------------------------------------------------+         */
/*                                                                          */
/****************************************************************************/

/* This should be aligned with Kconfig MAX row/columns */
#define MAX_MTX_KEY_COLS	16U
#define MAX_MTX_KEY_ROWS	8U

LOG_MODULE_DECLARE(kbchost, CONFIG_KBCHOST_LOG_LEVEL);

struct km_api *fujitsu_init(void);
int fujitsu_get_keynum(uint8_t col, uint8_t row);
int fujitsu_get_fn_key(uint8_t key_num, struct fn_data *data,
			       bool pressed);


struct km_api fujitu_keyboard_api = {
	.get_keynum = fujitsu_get_keynum,
	.get_fnkey = fujitsu_get_fn_key,
};

const uint8_t keymap[MAX_MTX_KEY_COLS][MAX_MTX_KEY_ROWS] = {
	{KEY_RSVD, 1, 112, 16, 2, KEY_RSVD, 30, KEY_RSVD},
	{116, 117, 110, KEY_RSVD, 118, 17, 18, KEY_RSVD},
	{113, KEY_RSVD, 114, 115, 3, KEY_RSVD, 119, KEY_RSVD},
	{49, 34, 48, 5, 4, 19, 20, KEY_RSVD},
	{35, 36, 21, 50, 6, 7, 22, 51},
	{23, 32, 33, 37, 8, 31, 52, 61},
	{38, 47, 46, KEY_RSVD, 9, 53, 24, 120},
	{25, 79, KEY_RSVD, 39, 10, 28, 13, KEY_RSVD},
	{41, KEY_RSVD, KEY_RSVD, 40, 27, 55, 11, 12},
	{KEY_RSVD, KEY_RSVD, KEY_RSVD, KEY_RSVD, 26, 90, 126, 121},
	{84, 71, KEY_RSVD, KEY_RSVD, 54, 122, 29, 15},
	{89, 83, KEY_RSVD, KEY_RSVD, 123, 75, 76, 43},
	{255, 127, KEY_RSVD, KEY_RSVD, KEY_RSVD, KEY_RSVD, 127, KEY_RSVD},
	{KEY_RSVD, KEY_RSVD, KEY_RSVD, 44, 57, KEY_RSVD, KEY_RSVD, KEY_RSVD},
	{62, 60, KEY_RSVD, KEY_RSVD, KEY_RSVD, KEY_RSVD, KEY_RSVD, KEY_RSVD},
	{KEY_RSVD, KEY_RSVD, 58, KEY_RSVD, KEY_RSVD, KEY_RSVD, KEY_RSVD, 64},
};

int fujitsu_get_keynum(uint8_t col, uint8_t row)
{
	if (col > MAX_MTX_KEY_COLS &&
	    row > MAX_MTX_KEY_ROWS) {
		return -EINVAL;
	}

	return keymap[col][row];
}

int fujitsu_get_fn_key(uint8_t key_num, struct fn_data *data,
			      bool pressed)
{
	switch (key_num) {
	/* Multimedia scan code set 2: Mute */
	case KM_F1_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x23U;
			data->sc.len = 2U;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x23U;
			data->sc.len = 3U;
		}
		break;
	/* Multimedia scan code set 2: Volume down */
	case KM_F2_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x21U;
			data->sc.len = 2U;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x21U;
			data->sc.len = 3U;
		}
		break;
	/* Multimedia scan code set 2: Volume up */
	case KM_F3_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x32U;
			data->sc.len = 2U;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x32U;
			data->sc.len = 3U;
		}
		break;
	/* Multimedia scan code set 2: Play pause */
	case KM_F4_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x34U;
			data->sc.len = 2U;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x34U;
			data->sc.len = 3U;
		}
		break;
	/* Scan code set 2: Insert key */
	case KM_F5_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x70U;
			data->sc.len = 2U;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x70U;
			data->sc.len = 3U;
		}
		break;
	/* Scan code set 2: Print screen */
	case KM_F6_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x12U;
			data->sc.code[2] = 0xE0U;
			data->sc.code[3] = 0x7CU;
			data->sc.len = 4U;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x7CU;
			data->sc.code[3] = 0xE0U;
			data->sc.code[4] = 0xF0U;
			data->sc.code[5] = 0x12U;
			data->sc.len = 6U;
		}
		break;
	/* Toogle display */
	case KM_F7_KEY:
		/* Do nothing. As long as the length is 0 we don't care
		 * to indicate whether the type is a scan code or an sci
		 */
		data->type = FN_SCAN_CODE;
		data->sc.len = 0U;
		break;
	/* Scan code set 2: Numlock */
	case KM_F8_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0x77U;
			data->sc.len = 1U;
		} else {
			data->sc.code[0] = 0xF0U;
			data->sc.code[1] = 0x77U;
			data->sc.len = 2U;
		}
		break;
	/* SCI: Brightness down */
	case KM_F9_KEY:
		data->type = SCI_CODE;
		if (pressed) {
			data->sci_code = 0x43U;
			data->sc.len = 1U;
		} else {
			/* Clients must do nothng with braek code */
			data->sc.len = 0U;
		}
		break;
	/* SCI: Brightness up */
	case KM_F10_KEY:
		data->type = SCI_CODE;
		if (pressed) {
			data->sci_code = 0x44U;
			data->sc.len = 1U;
		} else {
			/* Clients must do nothng with braek code */
			data->sc.len = 0U;
		}
		data->type = SCI_CODE;
		break;
	/* SCI: Mail */
	case KM_F11_KEY:
		data->type = SCI_CODE;
		if (pressed) {
			data->sci_code = 0x45U;
			data->sc.len = 1U;
		} else {
			/* Clients must do nothng with braek code */
			data->sc.len = 0U;
		}
		break;
	/* Scan code set 2: Scroll lock */
	case KM_F12_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0x7EU;
			data->sc.len = 1U;
		} else {
			data->sc.code[0] = 0xF0U;
			data->sc.code[1] = 0x7EU;
			data->sc.len = 2U;
		}
		break;
	/* Scan code set 2: Home via left arrow */
	case KM_LFT_ARROW_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xC6U;
			data->sc.len = 2U;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x6CU;
			data->sc.len = 3U;
			}
		break;
	/* Scan code set 2: End via right arrow */
	case KM_RGT_ARROW_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x69U;
			data->sc.len = 2U;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x69U;
			data->sc.len = 3U;
			}
		break;
	/* Scan code set 2: Page up via up arrow */
	case KM_UP_ARROW_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x7DU;
			data->sc.len = 2U;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x7D;
			data->sc.len = 3U;
			}
		break;
	/* Scan code set 2: Page down via down arrow */
	case KM_DN_ARROW_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x7AU;
			data->sc.len = 2U;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x7AU;
			data->sc.len = 3U;
			}
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

struct km_api *fujitsu_init(void)
{
	return &fujitu_keyboard_api;
}
