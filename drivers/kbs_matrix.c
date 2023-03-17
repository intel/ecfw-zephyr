/*
 * Copyright (c) 2020 Intel Corportation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/kscan.h>
#include "kbs_keymap.h"
#include "kbs_matrix.h"
#include "board_config.h"
#include "keyboard_utility.h"
#include "sci.h"
#include "scicodes.h"
#include "smc.h"
#ifdef CONFIG_EARLY_KEY_SEQUENCE_DETECTION
#include "kbs_boot_keyseq.h"
#endif
#include <zephyr/logging/log.h>
#include <memops.h>
LOG_MODULE_DECLARE(kbchost, CONFIG_KBCHOST_LOG_LEVEL);

static const struct device *kscan_dev;
static struct k_timer typematic_timer;
static kbs_matrix_callback kbs_callback;
static void typematic_callback(struct k_timer *timer);
static void kscan_callback(const struct device *dev, uint32_t row,
			   uint32_t col, bool pressed);

/* This is received and forwarded by kbchost */
static const uint8_t *scan_code_set;
static struct km_api *keymap_api;

/* Track current state of key modifiers/combinations */
uint32_t kscan_flags;

/* Translated buffer with key data*/
static struct scan_code make_tpmatic_code;

/* Default typematic constants.
 * This corresponds to 92 ms of repeat rate and 500 ms of delay.
 * Default typematic settigs are loadded when the host sends
 * 0xF6 set default
 * 0xF5 disable
 * 0XF4 enable
 */
static const uint8_t dflt_typematic_delay_rate = 0x01U | 0xBU;

/* This is the typematic rate index
 * Bits 4 - 0
 */
static uint8_t typematic_period_idx;
/* Delay index before repeating a key
 */
static uint8_t typematic_delay_idx;

/* Detect Hotkey press event*/
bool hotkey_detected;

/* Typematic rate and delay values correspond to the data passed after
 * the F3 command (See the 8042 spec online). The typematic rate is calulated
 * using the following formula:
 *
 * rate = (8+A) * (2**B) * .00417, units in secs
 * where A is obtained from bits [2, 0] in decimal
 * and B comes from bits [4, 3] in decimal

 * delay = (C+1) * 250 ms
 * where C comes from bits [6, 5]
 * In this implementation, the typematic rate and delay are interpreted ms.
 */
#define TYPEMATIC_RATE_MASK	0x1FU
#define TYPEMATIC_DELAY_MASK	0x60U
#define TYPEMATIC_DELAY_POS	5U

static const uint16_t typematic_period[] = {
	33U,  37U,  42U,  46U,  50U,  54U,  58U,  63U,
	67U,  75U,  83U,  92U, 100U, 109U, 116U, 125U,
	133U, 149U, 167U, 182U, 200U, 217U, 232U, 250U,
	270U, 303U, 333U, 370U, 400U, 435U, 470U, 500U
};

static const uint16_t typematic_delay[] = { 250U, 500U, 750U, 1000U };

#define MAX_SC2_TABLE_SIZE 130U
/* This table represents the scancode set 2.
 * The flags help to identify whether a key can be sent
 * during a make, break and typemaic scenario
 * NOTE: Do not use this table for ROW 124 (Print Screen) and 126 Pause/Break
 *
 */
const struct scan_code scan_code2[MAX_SC2_TABLE_SIZE] = {
/*	Scan code		Len	IBM key map */
	{{0x00U},		0U},	/* Padding key num */
	{{0x0EU},		1U},	/* 1  */
	{{0x16U},		1U},	/* 2  */
	{{0x1EU},		1U},	/* 3  */
	{{0x26U},		1U},	/* 4  */
	{{0x25U},		1U},	/* 5  */
	{{0x2EU},		1U},	/* 6  */
	{{0x36U},		1U},	/* 7  */
	{{0x3DU},		1U},	/* 8  */
	{{0x3EU},		1U},	/* 9  */
	{{0x46U},		1U},	/* 10 */
	{{0x45U},		1U},	/* 11 */
	{{0x4EU},		1U},	/* 12 */
	{{0x55U},		1U},	/* 13 */
	{{0x0U},		1U},	/* 14 */
	{{0x66U},		1U},	/* 15 */
	{{0x0DU},		1U},	/* 16 */
	{{0x15U},		1U},	/* 17 */
	{{0x1DU},		1U},	/* 18 */
	{{0x24U},		1U},	/* 19 */
	{{0x2DU},		1U},	/* 20 */
	{{0x2CU},		1U},	/* 21 */
	{{0x35U},		1U},	/* 22 */
	{{0x3CU},		1U},	/* 23 */
	{{0x43U},		1U},	/* 24 */
	{{0x44U},		1U},	/* 25 */
	{{0x4DU},		1U},	/* 26 */
	{{0x54U},		1U},	/* 27 */
	{{0x5BU},		1U},	/* 28 */
	{{0x5DU},		1U},	/* 29 */
	{{0x58U},		1U},	/* 30 */
	{{0x1CU},		1U},	/* 31 */
	{{0x1BU},		1U},	/* 32 */
	{{0x23U},		1U},	/* 33 */
	{{0x2BU},		1U},	/* 34 */
	{{0x34U},		1U},	/* 35 */
	{{0x33U},		1U},	/* 36 */
	{{0x3BU},		1U},	/* 37 */
	{{0x42U},		1U},	/* 38 */
	{{0x4BU},		1U},	/* 39 */
	{{0x4CU},		1U},	/* 40 */
	{{0x52U},		1U},	/* 41 */
	{{0x5DU},		1U},	/* 42 */
	{{0x5AU},		1U},	/* 43 */
	{{0x12U},		1U},	/* 44 */
	{{0x61U},		1U},	/* 45 */
	{{0x1AU},		1U},	/* 46 */
	{{0x22U},		1U},	/* 47 */
	{{0x21U},		1U},	/* 48 */
	{{0x2AU},		1U},	/* 49 */
	{{0x32U},		1U},	/* 50 */
	{{0x31U},		1U},	/* 51 */
	{{0x3AU},		1U},	/* 52 */
	{{0x41U},		1U},	/* 53 */
	{{0x49U},		1U},	/* 54 */
	{{0x4AU},		1U},	/* 55 */
	{{0U},			0U},	/* 56 */
	{{0x59U},		1U},	/* 57 */
	{{0x14U},		1U},	/* 58 */
	{{0x0U},		0U},	/* 59 */
	{{0x11U},		1U},	/* 60 */
	{{0x29U},		1U},	/* 61 */
	{{0xE0U, 0x11U},	2U},	/* 62 */
	{{0U},			0U},	/* 63 */
	{{0xE0U, 0x14U},	2U},	/* 64 */
	{{0U},			0U},	/* 65 */
	{{0U},			0U},	/* 66 */
	{{0U},			0U},	/* 67 */
	{{0U},			0U},	/* 68 */
	{{0U},			0U},	/* 69 */
	{{0U},			0U},	/* 70 */
	{{0U},			0U},	/* 71 */
	{{0U},			0U},	/* 72 */
	{{0U},			0U},	/* 73 */
	{{0U},			0U},	/* 74 */
	{{0xE0U, 0x70U},	2U},	/* 75 */
	{{0xE0U, 0x71U},	2U},	/* 76 */
	{{0U},			0U},	/* 77 */
	{{0U},			0U},	/* 78 */
	{{0xE0U, 0x6BU},	2U},	/* 79 */
	{{0xE0U, 0x6CU},	2U},	/* 80 */
	{{0xE0U, 0x69U},	2U},	/* 81 */
	{{0U},			0U},	/* 82 */
	{{0xE0U, 0x75U},	2U},	/* 83 */
	{{0xE0U, 0x72U},	2U},	/* 84 */
	{{0xE0U, 0x7DU},	2U},	/* 85 */
	{{0xE0U, 0x7AU},	2U},	/* 86 */
	{{0U},			0U},	/* 87 */
	{{0U},			0U},	/* 88 */
	{{0xE0U, 0x74U},	2U},	/* 89 */
	{{0x77U},		1U},	/* 90 */
	{{0x6CU},		1U},	/* 91 */
	{{0x6BU},		1U},	/* 92 */
	{{0x69U},		1U},	/* 93 */
	{{0U},			0U},	/* 94 */
	{{0xE0U, 0x4AU},	2U},	/* 95 */
	{{0x75U},		1U},	/* 96 */
	{{0x73U},		1U},	/* 97 */
	{{0x72U},		1U},	/* 98 */
	{{0x70U},		1U},	/* 99 */
	{{0x7CU},		1U},	/* 100*/
	{{0x7DU},		1U},	/* 101*/
	{{0x74U},		1U},	/* 102*/
	{{0x7AU},		1U},	/* 103*/
	{{0x71U},		1U},	/* 104*/
	{{0x7BU},		1U},	/* 105*/
	{{0x79U},		1U},	/* 106*/
	{{0U},			0U},	/* 107*/
	{{0xE0U, 0x5AU},	2U},	/* 108*/
	{{0U},			0U},	/* 109*/
	{{0x76U},		1U},	/* 110*/
	{{0U},			0U},	/* 111*/
	{{0x05U},		1U},	/* 112*/
	{{0x06U},		1U},	/* 113*/
	{{0x04U},		1U},	/* 114*/
	{{0x0CU},		1U},	/* 115*/
	{{0x03U},		1U},	/* 116*/
	{{0x0BU},		1U},	/* 117*/
	{{0x83U},		1U},	/* 118*/
	{{0x0AU},		1U},	/* 119*/
	{{0x01U},		1U},	/* 120*/
	{{0x09U},		1U},	/* 121*/
	{{0x78U},		1U},	/* 122*/
	{{0x07U},		1U},	/* 123*/
	/* Print screen pressed.
	 * Note: This entry is marked as unavailable on purpose
	 * Make : 0xE0, 0x12, 0xE0, 0x7C
	 * Break sequence 0xE0, 0xF0, 0x7C, 0xE0, 0xF0, 0x12.
	 * Use the complex can code table defined below for print screen
	 * function below
	 */
	{{0U},			0U},    /* 124*/
	{{0x7E},		1U},	/* 125*/
	/* Pause/Break  pressed
	 * Note: This entry is marked as unavailable on purpose
	 * Use the the function get pause/break.
	 */
	{{0U},			0U},	/* 126*/
	{{0xE0, 0x1F},		2U},	/* 127*/
	{{0x29},		1U},	/* 128*/
	{{0xE0, 0x2F},		2U},	/* 129*/
};

#ifdef CONFIG_EARLY_KEY_SEQUENCE_DETECTION
static struct kbs_keyseq keyseq_det[KEYSEQ_MAX_SEQ_COUNT];
#endif

static inline bool numlock_on(void)
{
	return ((kscan_flags >> KBS_NUMLOCK_DOWN_POS) & 0x1) == 0x1;
}

static inline bool sclock_on(void)
{
	return ((kscan_flags >> KBS_SCLOCK_DOWN_POS) & 0x1) == 0x1;
}

static inline bool fn_pressed(void)
{
	return ((kscan_flags >> KBS_FN_DOWN_POS) & 0x1) == 0x1;
}

static inline bool ctrl_pressed(void)
{
	return ((kscan_flags >> KBS_CTRL_DOWN_POS) & 0x1) == 0x1;
}

static inline bool alt_pressed(void)
{
	return ((kscan_flags >> KBS_ALT_DOWN_POS) & 0x1) == 0x1;
}

static inline bool shift_pressed(void)
{
	return ((kscan_flags >> KBS_SHIFT_DOWN_POS) & 0x1) == 0x1;
}

/* If there is a dedicated numlock button, then this is going to help
 * us determine the state of the button
 */
static inline void toggle_nlock_key(bool pressed)
{
	if (pressed) {
		kscan_flags ^= KBS_NUMLOCK_DOWN;
	}
}

static inline void toggle_sclock_key(bool pressed)
{
	if (pressed) {
		kscan_flags ^= KBS_SCLOCK_DOWN;
	}
}

static inline void set_fn_key(bool pressed)
{
	if (pressed) {
		kscan_flags |= KBS_FN_DOWN;
	} else {
		kscan_flags &= ~KBS_FN_DOWN;
	}
}

static inline void set_ctrl_key(bool pressed)
{
	if (pressed) {
		kscan_flags |= KBS_CTRL_DOWN;
	} else {
		kscan_flags &= ~KBS_CTRL_DOWN;
	}
}

static inline void set_alt_key(bool pressed)
{
	if (pressed) {
		kscan_flags |= KBS_ALT_DOWN;
	} else {
		kscan_flags &= ~KBS_ALT_DOWN;
	}
}

static inline void set_shift_key(bool pressed)
{
	if (pressed) {
		kscan_flags |= KBS_SHIFT_DOWN;
	} else {
		kscan_flags &= ~KBS_SHIFT_DOWN;
	}
}

static inline bool fn_with_valid_keynum(uint8_t key_num)
{
	return fn_pressed()
		&& key_num != KM_LCNTRL_KEY
		&& key_num != KM_RCNTRL_KEY
		&& key_num != KM_LSHIFT_KEY
		&& key_num != KM_RSHIFT_KEY
		&& key_num != KM_LALT_KEY
		&& key_num != KM_RALT_KEY
		&& key_num != KM_LWIN_KEY;
}

int kbs_matrix_init(kbs_matrix_callback callback, uint8_t *initial_set)
{
	if (!callback) {
		LOG_ERR("Bad callback");
		return -EINVAL;
	}

	kscan_dev = DEVICE_DT_GET(KSCAN_MATRIX);
	if (!device_is_ready(kscan_dev)) {
		LOG_ERR("kscan device not ready");
		return -ENODEV;
	}

	kbs_write_typematic(dflt_typematic_delay_rate);
	kscan_config(kscan_dev, kscan_callback);

	k_timer_init(&typematic_timer, typematic_callback, NULL);

	kbs_callback = callback;
	scan_code_set = (const uint8_t *)initial_set;

	/* Get a keyboard layout instance */
	keymap_api = keymap_init_interface();
	if (!keymap_api) {
		LOG_ERR("Custom keyboard layout init failed");
		return -ENODEV;
	}

#ifdef CONFIG_EARLY_KEY_SEQUENCE_DETECTION
	/* CTRL + ALT + SHIFT */
	keyseq_det[KEYSEQ_TIMEOUT].trigger_key = 0;
	keyseq_det[KEYSEQ_TIMEOUT].modifiers = KBS_CTRL_DOWN | KBS_SHIFT_DOWN |
					       KBS_ALT_DOWN;
	/* ALT + SHIFT + user-defined key */
	keyseq_det[KEYSEQ_CUSTOM0].trigger_key = CONFIG_EARLY_KEYSEQ_CUSTOM0;
	keyseq_det[KEYSEQ_CUSTOM0].modifiers = KBS_SHIFT_DOWN | KBS_ALT_DOWN;
	keyseq_det[KEYSEQ_CUSTOM1].trigger_key = CONFIG_EARLY_KEYSEQ_CUSTOM1;
	keyseq_det[KEYSEQ_CUSTOM1].modifiers = KBS_SHIFT_DOWN | KBS_ALT_DOWN;
#endif

	return 0;
}

void kbs_write_typematic(uint8_t data)
{
	/* Cancel typematic timer before attempting to change settings */
	k_timer_stop(&typematic_timer);

	typematic_period_idx = data  & TYPEMATIC_RATE_MASK;
	typematic_delay_idx =
		(data >> TYPEMATIC_DELAY_POS) & TYPEMATIC_DELAY_MASK;
}

void kbs_keyboard_enable(void)
{
	kscan_enable_callback(kscan_dev);
	kbs_write_typematic(dflt_typematic_delay_rate);
}

void kbs_keyboard_disable(void)
{
	k_timer_stop(&typematic_timer);
	kscan_disable_callback(kscan_dev);
}

void kbs_keyboard_set_default(void)
{
	kbs_keyboard_disable();
	kbs_keyboard_enable();
}

bool kbs_is_hotkey_detected(void)
{
	return hotkey_detected;
}

/* This function excludes Fn because it does not produce any scan code */
static void update_modifier_keys(uint8_t key_num, bool pressed)
{
	switch (key_num) {
	case KM_LCNTRL_KEY:
	case KM_RCNTRL_KEY:
		set_ctrl_key(pressed);
		break;
	case KM_LALT_KEY:
	case KM_RALT_KEY:
		set_alt_key(pressed);
		break;
	case KM_LSHIFT_KEY:
	case KM_RSHIFT_KEY:
		set_shift_key(pressed);
		break;
	case KM_NUMLOCK_KEY:
		toggle_nlock_key(pressed);
		break;
	case KM_SCLOCK_KEY:
		toggle_sclock_key(pressed);
		break;
	}
}

static void get_print_screen_scode(struct scan_code *sc2, bool pressed,
				   bool ctrl_pressed, bool alt_pressed,
				   bool shift_pressed)
{
	/* Regular print screen functionality */
	if (!shift_pressed && !ctrl_pressed && !alt_pressed) {
		if (pressed) {
			sc2->code[0] = 0xe0U;
			sc2->code[1] = 0x12U;
			sc2->code[2] = 0xe0U;
			sc2->code[3] = 0x7cU;
			sc2->len = 4U;
			sc2->typematic = true;
		} else {
			sc2->code[0] = 0xe0U;
			sc2->code[1] = 0xf0U;
			sc2->code[2] = 0x7cU;
			sc2->code[3] = 0xe0U;
			sc2->code[4] = 0xf0U;
			sc2->code[5] = 0x12U;
			sc2->len = 6U;
			sc2->typematic = false;
		}
	/* emulation of sys-req */
	} else if (alt_pressed) {
		if (pressed) {
			sc2->code[0] = 0x84U;
			sc2->len = 1U;
			sc2->typematic = true;
		} else {
			sc2->code[0] = 0xf0U;
			sc2->code[1] = 0x84U;
			sc2->len = 2U;
			sc2->typematic = false;
		}
	} else { /* ctrl or shift down */
		if (pressed) {
			sc2->code[0] = 0xe0U;
			sc2->code[1] = 0x7cU;
			sc2->len = 2U;
			sc2->typematic = true;
		} else {
			sc2->code[0] = 0xf0;
			sc2->code[1] = 0xe0;
			sc2->code[2] = 0x7c;
			sc2->len = 3U;
			sc2->typematic = false;
		}
	}
}

/* Pause/break have neither typematic nor break code */
static void get_pause_scode(struct scan_code *sc2,
			    bool ctrl_pressed)
{
	/* Break scan code 2 */
	if (ctrl_pressed) {
		sc2->code[0] = 0xe0U;
		sc2->code[1] = 0x7eU;
		sc2->code[2] = 0xe0U;
		sc2->code[3] = 0xf0U;
		sc2->code[4] = 0x7eU;
		sc2->len = 5U;
	} else {
		/* Pause scan code 2 */
		sc2->code[0] = 0xe1U;
		sc2->code[1] = 0x14U;
		sc2->code[2] = 0x77U;
		sc2->code[3] = 0xe1U;
		sc2->code[4] = 0xf0U;
		sc2->code[5] = 0x14U;
		sc2->code[6] = 0xf0U;
		sc2->code[7] = 0x77U;
		sc2->len = 8U;
	}

	sc2->typematic = false;
}

/* This function is only exercised in keyboards with numlock button */
void get_numpad_scode(uint8_t key_num, struct scan_code *sc2,
		      bool pressed)
{
	uint8_t scan_code = 0x0U;
	/* This switch represents the keys in for keyboards without
	 * real numeric keys. But I guess the scame scan codes
	 * can be used with keyboards containing numeric pads
	 */
	switch (key_num) {
	case KM_KEY_7:
		scan_code = 0x6CU;
		break;
	case KM_KEY_8:
		scan_code = 0x75U;
		break;
	case KM_KEY_9:
		scan_code = 0x7DU;
		break;
	case KM_KEY_0:
		scan_code = 0x4AU;
		break;
	case KM_KEY_U_4:
		scan_code = 0x6BU;
		break;
	case KM_KEY_5_I:
		scan_code = 0x73U;
		break;
	case KM_KEY_6_O:
		scan_code = 0x74U;
		break;
	case KM_KEY_P_MUL:
		scan_code = 0x7CU;
		break;
	case KM_KEY_1_J:
		scan_code = 0x69U;
		break;
	case KM_KEY_2_K:
		scan_code = 0x72U;
		break;
	case KM_KEY_3_L:
		scan_code = 0x7AU;
		break;
	case KM_KEY_MINUS_SEMI:
		scan_code = 0x4EU;
		break;
	case KM_KEY_0_M:
		scan_code = 0x70U;
		break;
	case KM_KEY_DOT:
		scan_code = 0x71U;
		break;
	case KM_KEY_PLUS_SLASH:
		scan_code = 0x79U;
		break;
	}

	/* We already know the numeric keys are a single byte */
	if (scan_code != 0U) {
		if (pressed) {
			sc2->code[0] = scan_code;
			sc2->len++;
			sc2->typematic = true;

		} else {
			sc2->code[0] = 0xF0U;
			sc2->code[1] = scan_code;
			sc2->len = 2U;
			sc2->typematic = false;
		}
	} else {
		/* User just pressed numlock */
		sc2->len = 0U;
	}
}

/* Funnel function that will retrieve several kind of scan codes.
 * keynum represents the key station
 * sc2 is an out parameter to be filled here
 * typematic indicates if a specific keymap has typematic behavior.
 * This is an out parameter
 * pressed is either make or break
 */
static void get_scan_code(uint8_t key_num, struct scan_code *sc2, bool pressed)
{

	update_modifier_keys(key_num, pressed);

	bool ctrl_down = ctrl_pressed();
	bool alt_down = alt_pressed();
	bool shift_down = shift_pressed();

	/* This is exclusively used for keyboards without numpad.
	 * We can add a property to the keyboard implementatoin indicating
	 * whether it has numpad.
	 */

	if (numlock_on()) {
		get_numpad_scode(key_num, sc2, pressed);
		/* If the length is zero, then allow the execution
		 * to continue. This is because numlock may engaged,
		 * but the user is pressing other keys.
		 */
		if (sc2->len != 0U) {
			return;
		}
	}

	if (key_num == KM_PRINT_SCREEN) {
		/* Print screen plus other key combinations */
		get_print_screen_scode(sc2, pressed, ctrl_down,
				       alt_down, shift_down);
	} else if (key_num == KM_PAUSE) {
		/* Pause/break scancodes */
		get_pause_scode(sc2, ctrl_down);
	} else {
		/* These are regular keys in the QWERTY key without any
		 * esoteric key combination
		 */
		uint8_t i = 0U;

		/* Protect against overflow */
		if (key_num >= MAX_SC2_TABLE_SIZE) {
			sc2->len = 0U;
			return;
		}
		const struct scan_code *code = &scan_code2[key_num];

		if (code->len != 0U) {
			if (pressed) {
				sc2->typematic = true;
				while (i < code->len) {
					sc2->code[i] = code->code[i];
					i++;
				}

				sc2->len = i;
			} else {
				sc2->typematic = false;
				int j = 0;

				while (i < code->len) {
					/* Extended scan codes  change
					 * when numlock and shift are combined.
					 * Especially for kb with keypad.
					 * This feature can be extended in
					 * in your custom kb implementaton.
					 * These key combinations are ignored
					 * here.
					 */
					if (code->code[i] != 0xE0) {
						sc2->code[j] = 0xF0U;
						j++;
					}
					sc2->code[j++] = code->code[i++];
				}

				sc2->len = j;
			}
		}
	}
}

static void make_key(uint8_t key_num)
{
	struct scan_code sc2;

	memsets(&sc2, 0, sizeof(sc2));
	sc2.typematic = false;

	/* Stop if we are in a current typematic state and
	 * a new key has been pressed whitout releasing the
	 * previus key
	 */
	k_timer_stop(&typematic_timer);

	if (key_num == KM_FN_KEY) {
		set_fn_key(true);
		return;
	}

	/* Fn + key presed combination */
	if (fn_with_valid_keynum(key_num)) {
		struct fn_data data;

		/* Retrieve data from custom keyboard implementation */
		keymap_get_fnkey(keymap_api, key_num, &data, true);
		if (data.type == FN_SCAN_CODE) {
			sc2 = data.sc;
			if (sc2.code[0] == SC_UNMAPPED)
				return;
		} else {
			/* Send an SCI. Remember to add 0x80 for SCI break*/
			LOG_DBG("Sci %x", data.sci_code);
			hotkey_detected = true;
			g_acpi_tbl.acpi_hotkey_scan = data.sci_code;
			enqueue_sci(SCI_HOTKEY);
		}
	} else { /* Handle ordinary key presses, qwerty keys + numlock */
		hotkey_detected = false;
		get_scan_code(key_num, &sc2, true);
	}

	if (sc2.len == 0U) {
		LOG_DBG("Invalid make code for key num = %d", key_num);
		return;
	}

	/* Translate keys from scancode 2 to scancode 1 */
	make_tpmatic_code.len = 0U;

	for (int i = 0; i < sc2.len; i++) {
		uint8_t value = sc2.code[i];

		if (translate_key(*scan_code_set, &value) == 0U &&
		    make_tpmatic_code.len  < MAX_SCAN_CODE_LEN) {
			make_tpmatic_code.code[make_tpmatic_code.len++] = value;
		}
	}

	kbs_callback(make_tpmatic_code.code, make_tpmatic_code.len);

	if (sc2.typematic) {
		/* Start timer to send scan codes while holding down
		 * the current key
		 */
		k_timer_start(&typematic_timer,
			K_MSEC(typematic_delay[typematic_delay_idx]),
			K_MSEC(typematic_period[typematic_period_idx]));
	}
}

static void break_key(uint8_t key_num)
{
	struct scan_code sc2;
	struct scan_code break_code;

	memsets(&sc2, 0, sizeof(sc2));
	sc2.typematic = false;

	k_timer_stop(&typematic_timer);

	if (key_num == KM_FN_KEY) {
		set_fn_key(false);
		return;
	}

	/* Fn + key release combination */
	if (fn_with_valid_keynum(key_num)) {
		struct fn_data data;

		/* Retrieve data from custom keyboard implementation */
		keymap_get_fnkey(keymap_api, key_num, &data, false);
		if (data.type == FN_SCAN_CODE) {
			sc2 = data.sc;
			if (sc2.code[0] == SC_UNMAPPED)
				return;
		}
	} else { /* Handle ordinary key releases, qwerty keys + numlock */
		get_scan_code(key_num, &sc2, false);
	}

	if (sc2.len == 0U) {
		LOG_DBG("Invalid break code for keynum = %d", key_num);
		return;
	}

	/* Translate keys from scancode 2 to scancode 1 */
	break_code.len = 0U;

	for (int i = 0; i < sc2.len; i++) {
		uint8_t value = sc2.code[i];

		if (translate_key(*scan_code_set, &value) == 0U &&
		    break_code.len  < MAX_SCAN_CODE_LEN) {
			break_code.code[break_code.len++] = value;

		}
	}

	kbs_callback(break_code.code, break_code.len);
}

static void typematic_callback(struct k_timer *timer)
{
	kbs_callback(make_tpmatic_code.code, make_tpmatic_code.len);
}

#ifdef CONFIG_EARLY_KEY_SEQUENCE_DETECTION
bool kbs_keyseq_boot_detect(enum kbs_keyseq_type type)
{
	LOG_INF("%s type:%d detected: %d", __func__, type,
		keyseq_det[type].detected);

	return keyseq_det[type].detected;
}

int kbs_keyseq_define(uint8_t modifiers, uint8_t key,
		      kbs_key_seq_detected callback)
{
	/* We only allow 1 runtime hot-key sequence */
	if (keyseq_det[KEYSEQ_RUNTIME].trigger_key != 0) {
		LOG_ERR("%s key trigger already defined", __func__);
		return -EINVAL;
	}

	if (keyseq_det[KEYSEQ_RUNTIME].handler != 0) {
		LOG_ERR("%s callback already registered", __func__);
		return -EINVAL;
	}

	LOG_INF("%s %x %d", __func__, modifiers, key);
	keyseq_det[KEYSEQ_RUNTIME].trigger_key = key;
	keyseq_det[KEYSEQ_RUNTIME].modifiers = modifiers;
	keyseq_det[KEYSEQ_RUNTIME].handler = callback;

	return 0;
}

int kbs_keyseq_register(enum kbs_keyseq_type index,
			kbs_key_seq_detected callback)
{
	/* Only 1 callback allow per key sequence */
	if (keyseq_det[index].handler != 0) {
		LOG_ERR("%s callback already registered", __func__);
		return -EINVAL;
	}

	keyseq_det[index].handler = callback;
	return 0;
}

static bool is_modifier(uint8_t last_key)
{
	switch (last_key) {
	case KM_LCNTRL_KEY:
	case KM_RCNTRL_KEY:
	case KM_LALT_KEY:
	case KM_RALT_KEY:
	case KM_LSHIFT_KEY:
	case KM_RSHIFT_KEY:
	case KM_NUMLOCK_KEY:
	case KM_SCLOCK_KEY:
		return true;
	default:
		break;
	}

	return false;
}

static void fw_hotkeyseq_detection(bool pressed, uint8_t key)
{
	static int trigger_key;

	LOG_DBG("flags: %x key: %x press:%d mod:(%d %d %d)", kscan_flags, key,
		pressed, ctrl_pressed(), alt_pressed(), shift_pressed());

	if (!is_modifier(key)) {
		trigger_key = key;
	}

	for (uint8_t i = 0; i < ARRAY_SIZE(keyseq_det); i++) {
		bool match_mod = ((kscan_flags & keyseq_det[i].modifiers) ==
				   keyseq_det[i].modifiers);

		/* Undefined runtime hotkey */
		if (keyseq_det[i].modifiers == 0) {
			continue;
		}

		LOG_DBG("exp_modifiers: %x exp_key_num %d",
			keyseq_det[i].modifiers, keyseq_det[i].trigger_key);

		/* Timeout hotkey is composed of modifier keys only */
		if (match_mod && i == KEYSEQ_TIMEOUT) {
			LOG_WRN("%s Timeout keyseq detected ", __func__);
			keyseq_det[i].detected = true;
			trigger_key = 0;
		} else if (trigger_key == keyseq_det[i].trigger_key &&
			   match_mod) {
			LOG_INF("%s keyseq %d detected ", __func__, i);
			keyseq_det[i].detected = true;
			trigger_key = 0;
		}

		if (keyseq_det[i].detected && keyseq_det[i].handler) {
			keyseq_det[i].handler(pressed);
		}
	}
}
#endif

static void kscan_callback(const struct device *dev, uint32_t row,
			   uint32_t col, bool pressed)
{
	ARG_UNUSED(dev);
	int last_key =  keymap_get_keynum(keymap_api, col, row);

	LOG_DBG("Keymap: %d col: %d row: %d", last_key, col, row);

	if (pressed) {
		make_key(last_key);
	} else {
		break_key(last_key);
	}

#ifdef CONFIG_EARLY_KEY_SEQUENCE_DETECTION
	/* Do not consume or alter in any way */
	fw_hotkeyseq_detection(pressed, last_key);
#endif
}

