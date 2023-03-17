/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Contain key station maps for several scan matrix keyboasrds.
 * Scan code 2 table is also defined here.
 */

#ifndef KBS_KEYMAP_H
#define KBS_KEYMAP_H
#include <zephyr/kernel.h>

/* Keymaps for generic key stations */
#define KM_LCNTRL_KEY		58U
#define KM_RCNTRL_KEY		0U
#define KM_LSHIFT_KEY		44U
#define KM_RSHIFT_KEY		57U
#define KM_LALT_KEY		60U
#define KM_RALT_KEY		62U
#define KM_FN_KEY		255U
#define KM_F1_KEY		112U
#define KM_F2_KEY		113U
#define KM_F3_KEY		114U
#define KM_F4_KEY		115U
#define KM_F5_KEY		116U
#define KM_F6_KEY		117U
#define KM_F7_KEY		118U
#define KM_F8_KEY		119U
#define KM_F9_KEY		120U
#define KM_F10_KEY		121U
#define KM_F11_KEY		122U
#define KM_F12_KEY		123U
#define KM_DEL_KEY		76U
#define KM_INS_KEY		75U
#define KM_LWIN_KEY		127U
#define KM_UP_ARROW_KEY		83U
#define KM_DN_ARROW_KEY		84U
#define KM_LFT_ARROW_KEY	79U
#define KM_RGT_ARROW_KEY	89U
#define KM_NUMLOCK_KEY		90U
#define KM_SCLOCK_KEY		125U

#define KM_PRINT_SCREEN		124U
#define KM_PAUSE		126U

#define KM_RSVD			0U
#define MAX_SCAN_CODE_LEN	8U

/* For keyboard with numlock button but no keypad (numpad),
 * then we use the keymaps for 7-8-9, u-i-o, j-k-l-m.
 * These keymaps also help to obtain a numeric
 * scan code when a user presses Fn key.
 */
#define KM_KEY_7		8U
#define KM_KEY_8		9U
#define KM_KEY_9		10U
#define KM_KEY_0		11U
/* Second row */
#define KM_KEY_U_4		23
#define KM_KEY_5_I		24U
#define KM_KEY_6_O		25U
#define KM_KEY_P_MUL		26U
/* Third row */
#define KM_KEY_1_J		37U
#define KM_KEY_2_K		38U
#define KM_KEY_3_L		39U
#define KM_KEY_MINUS_SEMI	40U
/* Fourth row */
#define KM_KEY_0_M		52U
#define KM_KEY_DOT		54U
#define KM_KEY_PLUS_SLASH	55U

/* This keymaps override the real numeric keys.
 * If we ever use a scan matrix with real numeric keys, then
 * this values are changed to the real keymaps
 */
#define KM_NUMPAD_7		KM_KEY_7
#define KM_NUMPAD_8		KM_KEY_8
#define KM_NUMPAD_9		KM_KEY_9
#define KM_NUMPAD_SLASH		KM_KEY_0
/* Second row */
#define KM_NUMPAD_4		KM_KEY_U_4
#define KM_NUMPAD_5		KM_KEY_5_I
#define KM_NUMPAD_6		KM_KEY_6_O
#define KM_NUMPAD_MUL		KM_KEY_P_MUL
/* Third row */
#define KM_NUMPAD_1		KM_KEY_1_J
#define KM_NUMPAD_2		KM_KEY_2_K
#define KM_NUMPAD_3		KM_KEY_3_L
#define KM_NUMPAD_MINUS		KM_KEY_MINUS_SEMI
/* Fourth row */
#define KM_NUMPAD_0		KM_KEY_0_M
#define KM_NUMPAD_DOT		KM_KEY_DOT
#define KM_NUMPAD_PLUS		KM_KEY_PLUS_SLASH

/* Real numeric key maps */
#define KM_NUMKEY_SLASH		95U
#define KEY_RSVD		0U

/* This refers to a scan code 1 or 2 to indicate that that we
 * must not send anything to the host. This flag is used for
 * scenarios where keys that have make code, but they don't
 * have a defined make code. This is assigned to a scan_code
 * data type.
 */
#define SC_UNMAPPED		0x0U

struct scan_code {
	uint8_t code[MAX_SCAN_CODE_LEN];
	uint8_t len;
	bool typematic;
};

/**
 * @brief fn_data_type enum.
 * Define the message type passed back to the kscan module.
 * The data coming from a concrete keyboard can either be a
 * scancode or an sci code.
 */
enum fn_data_type {
	FN_SCAN_CODE,
	SCI_CODE,
};

/**
 * @brief fn_data enum.
 * Define the message passed back to the kscan module.
 */
struct fn_data {
	enum fn_data_type type;
	union {
		/**
		 * This are expected to be in scan code set 2.
		 */
		struct scan_code sc;
		/** sci data to be enqueued as an sci event. */
		uint8_t sci_code;
	};
};

/**
 * @typedef km_get_keynum_t
 * @brief Define the application callback handler function signature.
 *
 * @param col Colum number in a specific keymap.
 * @param row Row  number in a specific keymap.
 *
 * @return Keymap number coresponding to the x,y coordinates in the
 * keyboard matrix.
 * @return 0 If the keymap is underfined.
 */
typedef int (*km_get_keynum_t)(uint8_t col, uint8_t row);

/* @typedef km_get_fnkey_t
 * @brief Define the application callback handler function signature.
 *
 * Note: Ideally, this routine shoudl only return ACPI. However, sometimes
 * BIOS request scancodes because the ACPI event may not be implemented on
 * their side. This is why we return 2 different data types.
 *
 * @param key_num Hint to determine FN+FX key combination.
 * @param fn_data Represet out data to be generated.
 *
 * Note: This ca be a scancode or an sci depending on what action is
 * desired for an specific Fn + FX key.
 *
 * @param pressed help to determine a make or brake.
 *
 */
typedef int (*km_get_fnkey_t)(uint8_t key_num, struct fn_data *data,
			      bool pressed);

struct km_api {
	km_get_keynum_t get_keynum;
	km_get_fnkey_t get_fnkey;
};

/**
 * @brief Concrete implementatoin for gtech keyboard init.
 *
 * @retval Forward a keyboard API to the caller.
 */
struct km_api *gtech_init();

/**
 * @brief Concrete implementatoin for fujits keyboard init.
 *
 * @retval Forward a keyboard API to the caller.
 */
struct km_api *fujitsu_init();

/**
 * @brief Factory function to select a specific keyboard.
 *
 * @retval Return an keyboard API for a specific implementation.
 * @retval NULL if there a specific keyboard is undefined.
 */
inline struct km_api *keymap_init_interface(void)
{
#if defined(CONFIG_EC_GTECH_KEYBOARD)
	return gtech_init();
#elif defined(CONFIG_EC_FUJITSU_KEYBOARD)
	return fujitsu_init();
#else
	return NULL;
#endif
}

inline int keymap_get_keynum(struct km_api *api, uint8_t col, uint8_t row)
{
	if (api->get_keynum == NULL) {
		return -EINVAL;
	}

	return api->get_keynum(col, row);
}

inline int keymap_get_fnkey(struct km_api *api, uint8_t key_num,
			    struct fn_data *data,
			    bool pressed)
{
	if (api->get_fnkey == NULL) {
		return -EINVAL;
	}

	return api->get_fnkey(key_num, data, pressed);

}

#endif /* KBS_KEYMAP_H */

