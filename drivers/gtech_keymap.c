/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kbs_keymap.h"
#include <zephyr/logging/log.h>

/****************************************************************************/
/*  Gtech keyboard                                                          */
/*                                                                          */
/*  Sense0  Sense1  Sense2  Sense3  Sense4  Sense5  Sense6  Sense7          */
/*+---------------------------------------------------------------+         */
/*|       |       |       |       |  N/A  |       | L Ctrl|  F5   |Scan  0  */
/*|       |       |       |       | (64)  |       | (58)  | (116) |(KEY #)  */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|  Q    |   F6  |  A    |  ESC  |  Z    |       |  ` ~  |  1 !  | Scan  1 */
/*| (17)  |  (16) | (31)  | (110) | (46)  |       |  (1)  |  (2)  | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|  W    |Capslk |  S    |       |   X   |       |  F1   |  2 @  | Scan  2 */
/*| (18)  | (30)  | (32)  |       |  (47) |       | (112) |  (3)  | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|  E    |  F3   |   D   |  F4   |   C   |       |   F2  |  3 #  | Scan  3 */
/*| (19)  | (114) |  (33) | (115) |  (48) |       | (113) |  (4)  | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|  20   |   T   |   F   |   G   |   V   |   B   |  5 %  |  4 $  | Scan  4 */
/*|  (R)  |  (21) |  (34) | (35)  | (49)  |  (50) |  (6)  |  (5)  | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|   U   |   Y   |   J   |   H   |   M   |   N   |  6 ^  |  7 &  | Scan  5 */
/*|  (23) |  (22) | (37)  |  (36) | (52)  |  (51) |  (7)  |  (8)  | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|   I   |  } ]  |   K   |   F6  | <  ,  |       |  + =  |  8 *  | Scan  6 */
/*|  (24) |  (28) |  (38) | (117) | (53)  |       |  (13) |  (9)  | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|   O   |  F7   |   L   |       | > .   |       |  F8   |  9 (  | Scan  7 */
/*|  (25) | (118) |  (39) |       | (54)  |       | (119) |  (10) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|   P   |  {  [ |  : ;  |  ' "  |  Fn   |  ? /  |  - _  |  0 )  | Scan  8 */
/*|  (26) |  (27) |  (40) |  (41) |  (255)|  (55) | (12)  |  (11) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|  - _  |       |       | LAlt  |       | R Alt |       |       | Scan  9 */
/*|  (0)  |       |       |  (60) |       |  (62) |       |       | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|  Fn   | BkSpac|Delete |  F11  | Enter | F12   |  F9   |  F10  | Scan 10 */
/*|  (255)|  (15) | (76)  | (122) | (43)  | (123) | (120) | (121) | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|       |       |       | SBar  |       | DArrw |  | \  |       | Scan 11 */
/*|       |       |       | (61)  |       | (84)  |  (29) |       | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|       |       |       |       |       | RArrw |       |       | Scan 12 */
/*|       |       |       |       |       |  (89) |       |       | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|       |Windows|       |       |       |       |       |       | Scan 13 */
/*|       | (127) |       |       |       |       |       |       | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|       |       |       | UpArrw|       | LArrw |       |       | Scan 14 */
/*|       |       |       | (83)  |       |  (79) |       |       | (KEY #) */
/*|-------+-------+-------+-------+-------+-------+-------+-------|         */
/*|       |LShift |RShift |       |       |       |       |       | Scan 15 */
/*|       | (44)  |  (57) |       |       |       |       |       | (KEY #) */
/*+---------------------------------------------------------------+         */
/*                                                                          */
/****************************************************************************/

#define KM_GTECH_PAUSE_KEY 76U

LOG_MODULE_DECLARE(kbchost, CONFIG_KBCHOST_LOG_LEVEL);

int gtech_get_fn_key(uint8_t key_num, struct fn_data *data, bool pressed);

#ifdef CONFIG_KSCAN_EC
#ifdef CONFIG_SOC_FAMILY_MEC
#define MAX_MTX_KEY_COLS CONFIG_KSCAN_XEC_COLUMN_SIZE
#define MAX_MTX_KEY_ROWS CONFIG_KSCAN_XEC_ROW_SIZE
#endif
/* 64 is not assigned. We marked as KM_RSVD in the first column */
/* 0 in the first column  _, - is also marked as KM_RSVD */

/* Here we assign 255(KM_FN_KEY) to Fn on purpose since we don't have a
 * standard keymap which can give you an scan code using 59.
 * Also Fn does not produce scan codes. In the data sheet 59 is repated twice.
 */

/* Two different keymaps are swapped on purpose for this keyboard. The keys to
 * swapped are 29 and 76. These keys are misplanced in the documentation
 */
static const uint8_t gtech_keymap[MAX_MTX_KEY_COLS][MAX_MTX_KEY_ROWS] = {
	{KM_RSVD, KM_RSVD, KM_RSVD, KM_RSVD, KM_RSVD, KM_RSVD, 58U, 116U},
	{17U, 16U, 31U, 110U, 46U, KM_RSVD, 1U, 2U},
	{18U, 30U, 32U, KM_RSVD, 47, KM_RSVD, 112U, 3U},
	{19U, 114U, 33U, 115U, 48U, KM_RSVD, 113U, 4U},
	{20U, 21U, 34U, 35U, 49U, 50U, 6U, 5U},
	{23U, 22U, 37U, 36U, 52U, 51U, 7U, 8U},
	{24U, 28U, 38U, 117U, 53U, KM_RSVD, 13U, 9U},
	{25U, 118U, 39U, KM_RSVD, 54U, KM_RSVD, 119U, 10U},
	{26U, 27U, 40U, 41U, 255U, 55U, 12U, 11U},
	{0U, KM_RSVD, KM_RSVD, 60U, KM_RSVD, 62, KM_RSVD, KM_RSVD},
	{KM_FN_KEY, 15U,  KM_GTECH_PAUSE_KEY, 122U, 43U, 123U, 120U, 121U},
	{KM_RSVD, KM_RSVD, KM_RSVD, 61U, KM_RSVD, 84U, 29U, KM_RSVD},
	{KM_RSVD, KM_RSVD, KM_RSVD, KM_RSVD, KM_RSVD, 89U, KM_RSVD, KM_RSVD},
	{KM_RSVD, 127U, KM_RSVD, KM_RSVD, KM_RSVD, KM_RSVD, KM_RSVD, KM_RSVD},
	{KM_RSVD, KM_RSVD, KM_RSVD, 83U, KM_RSVD, 79U, KM_RSVD, KM_RSVD},
	{KM_RSVD, 44U, 57U, KM_RSVD, KM_RSVD, KM_RSVD, KM_RSVD, KM_RSVD},
};

int gtech_get_keynum(uint8_t col, uint8_t row);

struct km_api gtech_keyboard_api = {
	.get_keynum = gtech_get_keynum,
	.get_fnkey = gtech_get_fn_key,
};

int gtech_get_keynum(uint8_t col, uint8_t row)
{
	if (col > MAX_MTX_KEY_COLS &&
	    row > MAX_MTX_KEY_ROWS) {
		return -EINVAL;
	}

	return gtech_keymap[col][row];
}
#else
/* We still want to compile the function that handles FN top row keys since
 * we want to test it via PS/2 keyboard
 */
struct km_api gtech_keyboard_api = {
	.get_keynum = NULL,
	.get_fnkey = gtech_get_fn_key,
};
#endif

int gtech_get_fn_key(uint8_t key_num, struct fn_data *data,
		     bool pressed)
{
	switch (key_num) {
	/* Multimedia scan code set 2: Mute */
	case KM_F1_KEY:
		data->type = FN_SCAN_CODE;
		data->sc.typematic = false;
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
		data->sc.typematic = false;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x21U;
			data->sc.len = 2U;
			data->sc.typematic = true;
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
		data->sc.typematic = false;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x32U;
			data->sc.len = 2U;
			data->sc.typematic = true;
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
		data->sc.typematic = false;
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
		data->sc.typematic = false;
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
		data->sc.typematic = false;
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
	case KM_F7_KEY:
		/* Do nothing. As long as the length is 0 we don't care
		 * to indicate whether the type is a scan code or an sci
		 */
		data->type = FN_SCAN_CODE;
		data->sc.typematic = false;
		data->sc.code[0] = SC_UNMAPPED;
		data->sc.len = 1U;
		break;
	case KM_F8_KEY:
		/* Do nothing. As long as the length is 0 we don't care
		 * to indicate whether the type is a scan code or an sci
		 */
		data->type = FN_SCAN_CODE;
		data->sc.typematic = false;
		data->sc.code[0] = SC_UNMAPPED;
		data->sc.len = 1U;
		break;
	/* SCI: Brightness down */
	case KM_F9_KEY:
		/*.updating the scan codes as per the vendor data sheet
		 * https://www.vetra.com/scancodes.html
		 */
		data->type = SCI_CODE;
		if (pressed) {
			data->sci_code = 0x43U;
		} else {
			data->sci_code  = 0U;
		}
		break;
	/* SCI: Brightness up */
	case KM_F10_KEY:
		data->type = SCI_CODE;
		if (pressed) {
			data->sci_code = 0x44U;
		} else {
			data->sci_code = 0U;
		}
		break;
	/* SCI: Airplane mode */
	case KM_F11_KEY:
		data->type = SCI_CODE;
		if (pressed) {
			data->sci_code = 0x45U;
		} else {
			data->sci_code = 0U;
		}
		break;
	/* Scan code set 2: Scroll lock */
	case KM_F12_KEY:
		data->type = FN_SCAN_CODE;
		data->sc.typematic = false;
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
			data->sc.code[1] = 0x6CU;
			data->sc.len = 2U;
			data->sc.typematic = true;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x6CU;
			data->sc.len = 3U;
			data->sc.typematic = false;
		}
		break;
	/* Scan code set 2: End via right arrow */
	case KM_RGT_ARROW_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x69U;
			data->sc.len = 2U;
			data->sc.typematic = true;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x69U;
			data->sc.len = 3U;
			data->sc.typematic = false;
		}
		break;
	/* Scan code set 2: Page up via up arrow */
	case KM_UP_ARROW_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x7DU;
			data->sc.len = 2U;
			data->sc.typematic = true;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x7D;
			data->sc.len = 3U;
			data->sc.typematic = false;
		}
		break;
	/* Scan code set 2: Page down via down arrow */
	case KM_DN_ARROW_KEY:
		data->type = FN_SCAN_CODE;
		if (pressed) {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0x7AU;
			data->sc.len = 2U;
			data->sc.typematic = true;
		} else {
			data->sc.code[0] = 0xE0U;
			data->sc.code[1] = 0xF0U;
			data->sc.code[2] = 0x7AU;
			data->sc.len = 3U;
			data->sc.typematic = false;
		}
		break;
	/* Scan code set 2: Pause key */
	case KM_GTECH_PAUSE_KEY:
		data->type = FN_SCAN_CODE;
		data->sc.typematic = false;
		/* Pause scan code 2 */
		if (pressed) {
			data->sc.code[0] = 0xE1U;
			data->sc.code[1] = 0x14U;
			data->sc.code[2] = 0x77U;
			data->sc.code[3] = 0xE1U;
			data->sc.code[4] = 0xF0U;
			data->sc.code[5] = 0x14U;
			data->sc.code[6] = 0xF0U;
			data->sc.code[7] = 0x77U;
			data->sc.len = 8U;
		} else {
			data->sc.code[0] = SC_UNMAPPED;
			data->sc.len = 1U;
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

struct km_api *gtech_init(void)
{
	return &gtech_keyboard_api;
}

