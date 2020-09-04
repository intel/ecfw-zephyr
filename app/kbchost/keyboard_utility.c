#include <zephyr.h>
#include "keyboard_utility.h"

#define START_BREAK_CODE 0xf0U

static bool found_break_code;

int translate_key(enum scan_code_set scan_code, u8_t *data)
{
	switch (scan_code) {
	case SCAN_CODE_SET1:
		/* Do nothing */
		break;
	case SCAN_CODE_SET2:
		/* If scan code set 2, then translate to set 1 */
		if (*data == START_BREAK_CODE) {
			found_break_code = true;
			return -EINVAL;
		}
		*data = kb_translation_table[0xFF - *data];
		if (found_break_code) {
			*data |= 0x80;
			found_break_code = false;
		}
		break;
	default:
		return -ENOTSUP;
	}

	return 0;
}
