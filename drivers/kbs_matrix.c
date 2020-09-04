/*
 * Copyright (c) 2020 Intel Corportation
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/kscan.h>
#include "kbs_keymap.h"
#include "kbs_matrix.h"
#include "board_config.h"
#include "keyboard_utility.h"
#include <logging/log.h>
LOG_MODULE_DECLARE(kbchost, CONFIG_KBCHOST_LOG_LEVEL);

static struct device *kscan_dev;
static struct k_timer typematic_timer;
static kbs_matrix_callback kbs_callback;
static void typematic_callback(struct k_timer *timer);
static void kscan_callback(struct device *dev, u32_t row,
			   u32_t col, bool pressed);

/* This is received and forwarded by kbchost */
static const u8_t *scan_code_set;
static struct km_api *keymap_api;

/* Track current state of key modifiers/combinations */
u32_t kscan_flags;

/* Translated buffer with key data*/
static struct scan_code make_tpmatic_code;

/* Default typematic constants.
 * This corresponds to 92 ms of repeat rate and 500 ms of delay.
 * Default typematic settigs are loadded when the host sends
 * 0xF6 set default
 * 0xF5 disable
 * 0XF4 enable
 */
static const u8_t dflt_typematic_delay_rate = 0x01U | 0xBU;

/* This is the typematic rate index
 * Bits 4 - 0
 */
static u8_t typematic_period_idx;
/* Delay index before repeating a key
 */
static u8_t typematic_delay_idx;

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

static const u16_t typematic_period[] = {
	33U,  37U,  42U,  46U,  50U,  54U,  58U,  63U,
	67U,  75U,  83U,  92U, 100U, 109U, 116U, 125U,
	133U, 149U, 167U, 182U, 200U, 217U, 232U, 250U,
	270U, 303U, 333U, 370U, 400U, 435U, 470U, 500U
};

static const u16_t typematic_delay[] = { 250U, 500U, 750U, 1000U };

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
	{{0x59U},		0U},	/* 57 */
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

int kbs_matrix_init(kbs_matrix_callback callback, u8_t *initial_set)
{
	if (!callback) {
		LOG_ERR("Bad callback");
		return -EINVAL;
	}

	kscan_dev = device_get_binding(KSCAN_MATRIX);
	if (!kscan_dev) {
		LOG_ERR("kscan device %s not found", KSCAN_MATRIX);
		return -ENODEV;
	}

	kbs_write_typematic(dflt_typematic_delay_rate);
	kscan_config(kscan_dev, kscan_callback);

	k_timer_init(&typematic_timer, typematic_callback, NULL);

	kbs_callback = callback;
	scan_code_set = (const u8_t *)initial_set;

	/* Get a keyboard layout instance */
	keymap_api = keymap_init_interface();
	if (!keymap_api) {
		LOG_ERR("Custom keyboard layout init failed");
		return -ENODEV;
	}

	return 0;
}

void kbs_write_typematic(u8_t data)
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
	k_timer_start(&typematic_timer,
		K_MSEC(typematic_delay[typematic_delay_idx]),
		K_MSEC(typematic_period[typematic_period_idx]));
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

/* This function excludes Fn because it does not produce any scan code */
static void update_modifier_keys(u8_t key_num, bool pressed)
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

static void get_print_screen_scode(struct scan_code *sc2,
				   bool *typematic, bool pressed,
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
			*typematic = true;
		} else {
			sc2->code[0] = 0xe0U;
			sc2->code[1] = 0xf0U;
			sc2->code[2] = 0x7cU;
			sc2->code[3] = 0xe0U;
			sc2->code[4] = 0xf0U;
			sc2->code[5] = 0x12U;
			sc2->len = 6U;
			*typematic = false;
		}
	/* emulation of sys-req */
	} else if (alt_pressed) {
		if (pressed) {
			sc2->code[0] = 0x84U;
			sc2->len = 1U;
			*typematic = true;
		} else {
			sc2->code[0] = 0xf0U;
			sc2->code[1] = 0x84U;
			sc2->len = 2U;
			*typematic = false;
		}
	} else { /* ctrl or shift down */
		if (pressed) {
			sc2->code[0] = 0xe0U;
			sc2->code[1] = 0x7cU;
			sc2->len = 2U;
			*typematic = true;
		} else {
			sc2->code[0] = 0xf0;
			sc2->code[1] = 0xe0;
			sc2->code[2] = 0x7c;
			sc2->len = 3U;
			*typematic = false;
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
}

/* This function is only exercised in keyboards with numlock button */
void get_numpad_scode(u8_t key_num, struct scan_code *sc2,
			       bool *typematic, bool pressed)
{
	u8_t scan_code = 0x0U;
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
			*typematic = true;

		} else {
			sc2->code[0] = 0xF0U;
			sc2->code[1] = scan_code;
			sc2->len = 2U;
			*typematic = false;
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
static void get_scan_code(u8_t key_num, struct scan_code *sc2,
			      bool *typematic, bool pressed)
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
		get_numpad_scode(key_num, sc2,
					  typematic, pressed);
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
		get_print_screen_scode(sc2,
				       typematic, pressed, ctrl_down,
				       alt_down, shift_down);
	} else if (key_num == KM_PAUSE) {
		/* Pause/break scancodes */
		get_pause_scode(sc2, ctrl_down);
		*typematic = false;
	} else {
		/* These are regular keys in the QWERTY key without any
		 * esoteric key combination
		 */
		u8_t i = 0U;

		/* Protect against overflow */
		if (key_num >= MAX_SC2_TABLE_SIZE) {
			sc2->len = 0U;
			return;
		}
		const struct scan_code *code = &scan_code2[key_num];

		if (code->len != 0U) {
			if (pressed) {
				*typematic = true;
				while (i < code->len) {
					sc2->code[i] = code->code[i];
					i++;
				}

				sc2->len = i;
			} else {
				*typematic = false;
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

static void make_key(u8_t key_num)
{
	struct scan_code sc2;

	bool typematic = false;

	/* Stop if we are in a current typematic state and
	 * a new key has been pressed whitout releasing the
	 * previus key
	 */
	k_timer_stop(&typematic_timer);

	if (key_num == KM_FN_KEY) {
		set_fn_key(true);
		return;
	}

	get_scan_code(key_num, &sc2, &typematic, true);

	if (sc2.len == 0U) {
		LOG_DBG("Invalid make code for key num = %d", key_num);
		return;
	}

	/* Translate keys from scancode 2 to scancode 1 */
	make_tpmatic_code.len = 0U;

	for (int i = 0; i < sc2.len; i++) {
		u8_t value = sc2.code[i];

		if (translate_key(*scan_code_set, &value) == 0U &&
		    make_tpmatic_code.len  < MAX_SCAN_CODE_LEN) {
			make_tpmatic_code.code[make_tpmatic_code.len++] = value;

		}
	}

	kbs_callback(make_tpmatic_code.code, make_tpmatic_code.len);

	if (typematic) {
		/* Start timer to send scan codes while holding down
		 * the current key
		 */
		k_timer_start(&typematic_timer,
			K_MSEC(typematic_delay[typematic_delay_idx]),
			K_MSEC(typematic_period[typematic_period_idx]));
	}
}

static void break_key(u8_t key_num)
{
	struct scan_code sc2;
	struct scan_code break_code;

	bool __attribute__ ((unused)) typematic;

	k_timer_stop(&typematic_timer);

	if (key_num == KM_FN_KEY) {
		set_fn_key(false);
		return;
	}

	get_scan_code(key_num, &sc2, &typematic, false);

	if (sc2.len == 0U) {
		LOG_DBG("Invalid break code for keynum = %d", key_num);
		return;
	}

	/* Translate keys from scancode 2 to scancode 1 */
	break_code.len = 0U;

	for (int i = 0; i < sc2.len; i++) {
		u8_t value = sc2.code[i];

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

static void kscan_callback(struct device *dev, u32_t row,
			   u32_t col, bool pressed)
{
	ARG_UNUSED(dev);
	int last_key =  keymap_get_keynum(keymap_api, col, row);

	LOG_DBG("Keymap:  %d\n", last_key);

	if (pressed) {
		make_key(last_key);
	} else {
		break_key(last_key);
	}
}

