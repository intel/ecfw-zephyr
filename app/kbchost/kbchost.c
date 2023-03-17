/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/espi.h>
#include "kbchost.h"
#include "ps2kbaux.h"
#include "gpio_ec.h"
#include "espi_hub.h"
#include "kbs_matrix.h"
#include "pwrplane.h"
#include "board_config.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(kbchost, CONFIG_KBCHOST_LOG_LEVEL);

#define MAX_HOST_REQ_SIZE 4

/* KBC input port dummy bits */
#define OUTPUT_PORT_PS2_DATA_IN			(1 << 7)
#define OUTPUT_PORT_PS2_CLK_IN			(1 << 6)
#define OUTPUT_PORT_PS2_AUXCLK_IN		(1 << 3)
#define OUTPUT_PORT_PS2_AUXDATA_IN		(1 << 2)
#define OUTPUT_PORT_A20_EN			(1 << 1)
#define INPUT_PORT_PS2_DATA_IN			(1 << 0)
#define INPUT_PORT_PS2_AUXDATA_IN		(1 << 1)

/* The "command byte" is part of the keyboard controller RAM. In
 * this realization we just use a variable to represent its
 * internal state.
 */
static uint8_t cmdbyte = KBC_8042_KBD_DIS | KBC_8042_MOUSE_DIS |
			KBC_8042_HOST_SYS_FLAG | KBC_8042_TRANSLATE;

static uint8_t resend_cmd[MAX_HOST_REQ_SIZE];
static uint8_t resend_cmd_len;

/* We place the keyboards behind an factory/strategy API */
struct host_byte {
	uint8_t data;
	/* 1 = Command or 0 = Data */
	uint8_t cmd;
};

#define TO_HOST_LEN	16U
#define MAX_TO_HOST_RETRIES 3U
#define MAX_RST_ATTEMPTS 3U
/* Period unit in ms */
#define MB_RESET_PERIOD 8U
#define KBC_RETRY_PERIOD 1U
#define TOHOST_RETRY_PERIOD 2U
#define GAP_FOR_DUMMY_COMMANDS 5U

K_MSGQ_DEFINE(from_host_queue, sizeof(struct host_byte), 8, 4);
K_MSGQ_DEFINE(to_host_kb_queue, sizeof(uint8_t), TO_HOST_LEN, 4);
K_SEM_DEFINE(kb_p60_sem, 0, 1);
K_MUTEX_DEFINE(led_mutex);
#ifdef CONFIG_PS2_MOUSE
static atomic_t ps2_reset;
#endif
static int kbc_init(void);
static void purge_kb_queue(void);
static void send_kb_to_host(uint8_t data);

static uint8_t current_scan_code = 2;

static enum {
	DEFAULT_STATE = 0,
	WRITE_CMD_BYTE_STATE,
	HOTKEY_EVENT_STATE,
	WRITE_OUTPUT_PORT_STATE,
	WRITE_KBD_OUTPUT_REG_STATE,
	ECHO_MOUSE_STATE,
	SEND_TO_MOUSE_STATE,
	/*KB STATES */
	SET_LEDS_STATE,
	SET_GET_SCANCODE_STATE,
	SET_TYPEMATIC_RATE_STATE,

} data_port_state = DEFAULT_STATE;

/* Led internal states is tracked in a variable */
static uint8_t current_leds_state;

/* Command byte operations implicity affecting keyboard
 * and mouse. TODO add kscan APIs
 */
static inline void write_command_byte(uint8_t new_cmdbyte)
{
	cmdbyte = new_cmdbyte;
}

static inline void cmdbyte_enable_kbd(void)
{
	write_command_byte(cmdbyte & ~KBC_8042_KBD_DIS);
}

static inline void cmdbyte_disable_kbd(void)
{
	write_command_byte(cmdbyte |  KBC_8042_KBD_DIS);
}

static inline bool cmdbyte_kbd_enabled(void)
{
	return cmdbyte & KBC_8042_KBD_DIS ? false : true;
}

static inline void cmdbyte_enable_mb(void)
{
	write_command_byte(cmdbyte & ~KBC_8042_MOUSE_DIS);
}

static inline void cmdbyte_disable_mb(void)
{
	write_command_byte(cmdbyte | KBC_8042_MOUSE_DIS);
}

static inline bool cmdbyte_mb_enabled(void)
{
	return cmdbyte & KBC_8042_MOUSE_DIS ? false : true;
}

/**
 * Handle the port 0x64 writes from host.
 * This function process commands sent to KBC 8042, keyboard and mouse.
 * If the host wants the EC to process a KBC command, we switch to a state
 * for processing keyboard commands. Otherwise we don't transition to any
 * state. Commands intended for the keyboard are dispatched through port 0x60
 * which is quite confusing because port 0x60 is also used for sending command
 * data to the 8042 KBC.
 * Remember keyboard commands are sent trough port 0x64 which means the EC
 * and client driver will see this as data. It is responsability of the
 * calling code to drive information appropiately.
 * This functions returns the number of bytes stored in *output buffer but
 * bytes will appear at port 0x60.
 *
 */
static int process_keyboard_command(uint8_t command, uint8_t *output)
{
	uint8_t out_len = 0;

#if !defined(CONFIG_PS2_MOUSE)
	if (command == KBC_8042_DIS_MOUSE || command == KBC_8042_ENA_MOUSE ||
	    command == KBC_8042_TEST_MOUSE) {
		return out_len;
	}
#endif

	switch (command) {
	case KBC_8042_READ_CMD_BYTE:
		output[out_len++] = cmdbyte;
		break;
	case KBC_8042_WRITE_CMD_BYTE:
		data_port_state = WRITE_CMD_BYTE_STATE;
		break;
	case KBC_8042_TEST_PASSWORD:
		output[out_len++] = NO_PASSWORD;
		break;
#if defined(CONFIG_PS2_MOUSE)
	case KBC_8042_DIS_MOUSE:
		cmdbyte_disable_mb();
		break;
	case KBC_8042_ENA_MOUSE:
		cmdbyte_enable_mb();
		break;
	case KBC_8042_TEST_MOUSE:
		/* We don't touch the command byte here */
		output[out_len++] = 0U;
		break;
#endif
	case KBC_8042_RESET_SELF_TEST:
		purge_kb_queue();
		output[out_len++] = TEST_PASSED;
		break;
	case KBC_8042_TEST_KB_PORT:
		output[out_len++] = 0U;
		break;
	case KBC_8042_DIAGNOSTIC_DUMP:
		output[out_len++] = 0U;
		break;
	case KBC_8042_DIS_KB:
		cmdbyte_disable_kbd();
		purge_kb_queue();
		break;
	case KBC_8042_ENA_KB:
		cmdbyte_enable_kbd();
		break;
	case KBC_8042_READ_INPUT_PORT:
		/* Write dummy values back to the host */
		output[out_len++] =  INPUT_PORT_PS2_DATA_IN |
					INPUT_PORT_PS2_AUXDATA_IN;
		break;
	case KBC_8042_INS_HOTKEY_EVENT:
		/* Cmd 0xC3 : This is not defined in the 8042 spec. This
		 * expects data from host so we transition to a new state
		 * and wait for data
		 */
		data_port_state = HOTKEY_EVENT_STATE;
		break;
	case KBC_8042_READ_OUTPUT_PORT:
		/* Write dummy information, back to the host */
		output[out_len++] = OUTPUT_PORT_PS2_DATA_IN |
					OUTPUT_PORT_PS2_CLK_IN |
					OUTPUT_PORT_PS2_AUXCLK_IN |
					OUTPUT_PORT_PS2_AUXDATA_IN |
					OUTPUT_PORT_A20_EN;
		break;
	case KBC_8042_WRITE_OUTPUT_PORT:
		data_port_state = WRITE_OUTPUT_PORT_STATE;
		break;
	case KBC_8042_WRITE_KBD_OUTPUT_REG:
		data_port_state = WRITE_KBD_OUTPUT_REG_STATE;
		break;
	case KBC_8042_ECHO_MOUSE:
		data_port_state = ECHO_MOUSE_STATE;
		break;
	case KBC_8042_SEND_TO_MOUSE:
		data_port_state = SEND_TO_MOUSE_STATE;
		break;
	case KBC_8042_PULSE_OUTPUT:
		/* TODO : This is an espi VW transaction to reset the host */
		break;
	default:
		purge_kb_queue();
		output[out_len++] = KBC_8042_NACK;
		data_port_state = DEFAULT_STATE;
		break;
	}

	return out_len;
}

/**
 * Handle the port 0x60 writes from host.
 *
 * This functions returns the number of bytes stored in *output buffer.
 */
static int process_keyboard_data(uint8_t data, uint8_t *output)
{
	uint8_t out_len = 0;
	int save_cmd = 1;
	uint32_t kbd_flags;

	switch (data_port_state) {
	case WRITE_CMD_BYTE_STATE:
		kbd_flags = KBC_8042_HOST_SYS_FLAG;
		/* Optionally we can introduce E8042_SET_KBC_STS
		 * instead of individual flags
		 */
		if (data & KBC_8042_HOST_SYS_FLAG) {
			espihub_kbc_write(E8042_SET_FLAG, kbd_flags);
		} else {
			espihub_kbc_write(E8042_CLEAR_FLAG, kbd_flags);
		}

		write_command_byte(data);
		if (data & KBC_8042_KBD_DIS) {
			purge_kb_queue();
			cmdbyte_disable_kbd();
		} else {
			cmdbyte_enable_kbd();
		}

		if (data & KBC_8042_MOUSE_DIS) {
			cmdbyte_disable_mb();

		} else {
			cmdbyte_disable_mb();
		}

		data_port_state = DEFAULT_STATE;
		break;
	case WRITE_OUTPUT_PORT_STATE:
		/* Write output port does nothing */
		data_port_state = DEFAULT_STATE;
		break;
	case HOTKEY_EVENT_STATE:
		/* TODO : implement hot key event */
		break;
	case WRITE_KBD_OUTPUT_REG_STATE:
		/* Write output buf returns the data back to host */
		output[out_len++] = data;
		data_port_state = DEFAULT_STATE;
		break;
	case ECHO_MOUSE_STATE:
#if defined(CONFIG_PS2_KEYBOARD) || defined(CONFIG_PS2_MOUSE)
		/* Echo sends back data to host through aux register */
		espihub_kbc_write(E8042_WRITE_MB_CHAR, data);
#endif
		data_port_state = DEFAULT_STATE;
		break;
	case SEND_TO_MOUSE_STATE:
#ifdef CONFIG_PS2_MOUSE
		if (data == KBC_8042_RESET) {
			atomic_set(&ps2_reset, 1U);
			int attempt = 0;

			/* Resets tend to return NACK while a user
			 * is moving the mouse, and a reset command is
			 * being sent. Therefore we retry
			 */
			while (attempt <= MAX_RST_ATTEMPTS) {
				if (atomic_get(&ps2_reset) == 0U) {
					break;
				}
				LOG_WRN("Reset aux attempt: %d", attempt);
				ps2_mouse_write(KBC_8042_RESET);
				k_msleep(MB_RESET_PERIOD);
				attempt++;
			}
		} else {
			ps2_mouse_write(data);
		}

#endif
		data_port_state = DEFAULT_STATE;
		break;
	case SET_LEDS_STATE:
		/* Control RVP leds */
		kbc_set_leds(data);
#if defined(CONFIG_PS2_KEYBOARD)
		/*  Send the byte to keyboard */
		ps2_keyboard_write(data);
#endif
		/* Note: We ACKed in DEFAULT_STATE too */
		output[out_len++] = KBC_8042_ACK;
		data_port_state = DEFAULT_STATE;
		break;
	case SET_GET_SCANCODE_STATE:
		/* When data is 0 it retrieves the current scan code */
		output[out_len++] = KBC_8042_ACK;

		if (!data) {
			output[out_len++] = current_scan_code;
		} else {
			purge_kb_queue();
			current_scan_code = data;
#if defined(CONFIG_PS2_KEYBOARD)
			ps2_keyboard_write(data);
#endif
		}
		data_port_state = DEFAULT_STATE;
		break;
	case SET_TYPEMATIC_RATE_STATE:
		output[out_len++] = KBC_8042_ACK;

#if defined(CONFIG_PS2_KEYBOARD)
		ps2_keyboard_write(data);
#endif
#if defined(CONFIG_KSCAN_EC)
		kbs_write_typematic(data);
#endif
		data_port_state = DEFAULT_STATE;
		break;
	default:
		/* DEFAULT_STATE
		 * Commmands arriving from port 0x60
		 */
		switch (data) {
		case KBC_8042_ECHO_KEYBOARD:
			/* Echo without ACK. Mandatory for scan matrix too */
			output[out_len++] = KBC_8042_ECHO_KEYBOARD;
			break;
		case KBC_8042_SET_LEDS:
#if defined(CONFIG_PS2_KEYBOARD)
			ps2_keyboard_write(data);
#endif
			output[out_len++] = KBC_8042_ACK;
			data_port_state = SET_LEDS_STATE;
			break;
		case KBC_8042_SET_GET_SCANCODE:
			output[out_len++] = KBC_8042_ACK;
			data_port_state = SET_GET_SCANCODE_STATE;
			break;
		case KBC_8042_READ_ID:
			/* We just put any ID we don't care */
			output[out_len++] = KBC_8042_ACK;
			output[out_len++] = 0xab;
			output[out_len++] = 0x83;
			break;
		case KBC_8042_SET_TYPEMATIC_RATE:
			/* We ACK and wait for the parameters */
			output[out_len++] = KBC_8042_ACK;
			data_port_state = SET_TYPEMATIC_RATE_STATE;
			break;
		case KBC_8042_EN_KEYBOARD:
			purge_kb_queue();
#if defined(CONFIG_PS2_KEYBOARD)
			ps2_keyboard_write(data);
#endif
			output[out_len] = KBC_8042_ACK;
			break;
		case KBC_8042_DEFAULT_DIS:
			/* Set default and disable */
			output[out_len++] = KBC_8042_ACK;
			current_scan_code = KBC_8042_DEFAULT_SCAN_CODE;
			purge_kb_queue();
#if defined(CONFIG_PS2_KEYBOARD)
			ps2_keyboard_write(data);
#endif
#if defined(CONFIG_KSCAN_EC)
			kbs_keyboard_set_default();
#endif
			cmdbyte_disable_kbd();
			break;
		case KBC_8042_SET_DEFAULT:
			output[out_len++] = KBC_8042_ACK;
			current_scan_code = KBC_8042_DEFAULT_SCAN_CODE;
			purge_kb_queue();
#if defined(CONFIG_PS2_KEYBOARD)
			ps2_keyboard_write(data);
#endif
#if defined(CONFIG_KSCAN_EC)
			kbs_keyboard_set_default();
#endif
			break;
		case KBC_8042_RESEND:
			save_cmd = 0;
			for (int i = 0; i < resend_cmd_len; i++) {
				output[out_len++] = resend_cmd[i];
			}
			break;
		case KBC_8042_RESET:
			espihub_kbc_write(E8042_CLEAR_OBF, 0);
			purge_kb_queue();
#if defined(CONFIG_PS2_KEYBOARD)
			ps2_keyboard_write(data);
#endif
#if defined(CONFIG_KSCAN_EC)
			kbs_keyboard_set_default();
#endif
			/* Set the scan code set to 2 */
			current_scan_code = KBC_8042_DEFAULT_SCAN_CODE;
			output[out_len++] = KBC_8042_ACK;
			output[out_len++] = KBC_8042_BAT;
			break;
		default:
			output[out_len++] = KBC_8042_NACK;
		}
	}

	/* Keep the current result in case the host decides to
	 * issue a resend command
	 */
	if (out_len && save_cmd) {
		for (int i = 0; i < out_len; i++) {
			resend_cmd[i] = output[i];
		}
		resend_cmd_len = out_len;
	}

	return out_len;
}

static void handle_from_to_host(struct host_byte host_data)
{
	uint8_t out_len;
	uint8_t data_to_host[MAX_HOST_REQ_SIZE] = {0};
	uint32_t host_char;

	/* If cmd = 1, then host sent data */
	if (host_data.cmd) {
		out_len = process_keyboard_command(host_data.data,
						   data_to_host);
		LOG_DBG("data: %x, state: %d, out_len: %d",
			host_data.data, data_port_state, out_len);
	} else {
		out_len = process_keyboard_data(host_data.data,
						data_to_host);
		LOG_DBG("data: %x, state: %d, out_len: %d",
			host_data.data, data_port_state, out_len);

	}

	/* Return dummy data to the host. For PS2 keyboard, we
	 * could do it in its isr. However, the kescan matrix
	 * processing needs to return dummy data for all the
	 * commands issued. Therefore, dummy values are returned
	 * to account for both keyboard implementations.
	 */
	if (out_len) {
		int i = 0;
		int obf_retries = 0;
		/* This is where we send dummy commands from ps/2 kbd,
		 * keyboard controller and scan matrix, and we don't send reply
		 * values inmediatly. This is because a ps/2 keyboard may be
		 * doing real processing and the host could send another
		 * command while the device is busy.
		 */
		k_msleep(GAP_FOR_DUMMY_COMMANDS);
		do {
			uint32_t kb_data = *(data_to_host + i);

			espihub_kbc_read(E8042_OBF_HAS_CHAR, &host_char);
			if (host_char) {
				k_msleep(KBC_RETRY_PERIOD);
				obf_retries++;
				LOG_WRN("Send kbc/kb attempt: %d", obf_retries);
			} else {
				espihub_kbc_write(E8042_WRITE_KB_CHAR,
						 kb_data);
				LOG_DBG("kbc/kb data: %x", kb_data);
				i++;
				obf_retries = 0;
			}
		} while (i < out_len &&
			 obf_retries <  MAX_TO_HOST_RETRIES);

		/* Data is placed in the kb queue in case host is busy */
		if (unlikely(obf_retries == MAX_RST_ATTEMPTS)) {
			while (i < out_len) {
				send_kb_to_host(*(data_to_host + i));
				i++;
			}
		}
	}
}

void to_from_host_thread(void *p1, void *p2, void *p3)
{
	struct host_byte host_data;

	kbc_init();
	espihub_add_kbc_handler(kbc_handler);

	while (true) {
		k_msgq_get(&from_host_queue, &host_data, K_FOREVER);

		/* Address host requests and sends request respose
		 * back to the host
		 */
		handle_from_to_host(host_data);
	}
}

void to_host_kb_thread(void *p1, void *p2, void *p3)
{
	uint32_t kb_data;
	uint32_t host_char;
	uint8_t obf_retries = 0;

	while (true) {
		k_sem_take(&kb_p60_sem, K_FOREVER);
		while (true) {

			/* Process the keyboard queue. If the amount of
			 * retries is exceeded due to a storm of keys, then
			 * Windows is going to show keys presed in past
			 * keystrokes because it couldn't empty its queue.
			 * This is why it is better to purge the kb queue
			 * and let user see keys as pressed
			 */
			if (k_msgq_num_used_get(&to_host_kb_queue) != 0U) {
				espihub_kbc_read(E8042_OBF_HAS_CHAR,
						 &host_char);
				if (host_char) {
					/* If the host is polling, then it is
					 * highly probable that this retry
					 * code is going to be exercised.
					 */
					if (obf_retries++  >
					    MAX_TO_HOST_RETRIES) {
						purge_kb_queue();
						obf_retries = 0;
						break;

					}
					k_msleep(TOHOST_RETRY_PERIOD);
				} else {
					/* Wake the Host if system is in S3 on
					 * detection of first key press.
					 */
					if (pwrseq_system_state() == SYSTEM_S3_STATE) {
						smc_generate_wake(WAKE_KBC_EVENT);
					}
					/* Send more kb data to the host */
					k_msgq_get(&to_host_kb_queue,
						   &kb_data, K_NO_WAIT);
					espihub_kbc_write(E8042_WRITE_KB_CHAR,
							  kb_data);
					LOG_DBG("kb data: %x", kb_data);
					obf_retries = 0;
				}
			} else {
				/* Go to suspended state if kb queue is empty */
				break;
			}
		}
	}
}

#if defined(CONFIG_PS2_KEYBOARD)
/* Callback passed to the PS2 instance handling the keyboard */
static void keyboard_callback(uint8_t data)
{

	/* We return the dummy ACKs when processing the keyboard
	 * related commands. This is because we want to process
	 * ps2 keyboard and kscan matrix in the case that both
	 * are enabled. This is why we only queue data typed
	 * from the keyboard.
	 */
	if (cmdbyte_kbd_enabled() && data != KBC_8042_ACK
	   && data != KBC_8042_NACK) {
		send_kb_to_host(data);
	}
}
#endif

#if defined(CONFIG_PS2_MOUSE)
/* Callback passed to the PS2 instance handling the mouse */
static void mouse_callback(uint8_t data)
{

	/* For the mouse we enqueue data to host under two different conditions.
	 * The first one is when the mouse interface is disabled, but the
	 * data from the mouse is an ACK or NACK. This allows to sequence the
	 * mouse initialization without placing delays.
	 * The other scenario is when the mouse is enabled (bit 5 in cmd byte)
	 * and ready to send mouse data such as x,y coordinates and button
	 * interaction.
	 */
	if (atomic_get(&ps2_reset) == 1U) {
		if (data == KBC_8042_ACK) {
			atomic_set(&ps2_reset, 0U);
			LOG_WRN("Reset aux: %x", data);
			espihub_kbc_write(E8042_WRITE_MB_CHAR, data);
		}
	} else {
		if ((!cmdbyte_mb_enabled() &&
		     (data == KBC_8042_ACK || data == KBC_8042_NACK))
			|| cmdbyte_mb_enabled()) {
			espihub_kbc_write(E8042_WRITE_MB_CHAR, data);
		}
	}
}
#endif

#if defined(CONFIG_KSCAN_EC)
/* All the kb data is being pushed to kbc host in a single shot */
static void mtx_keyboard_callback(uint8_t *data, uint8_t len)
{

	if (cmdbyte_kbd_enabled() && !kbs_is_hotkey_detected()) {
		for (int i = 0; i < len; i++) {
			send_kb_to_host(data[i]);
		}
	}
}
#endif

static int kbc_init(void)
{
	int ps2_kb_err = 0;
	int ps2_mb_err = 0;

#if defined(CONFIG_PS2_MOUSE)
	ps2_mb_err = ps2_mouse_init(mouse_callback);
#endif

#if defined(CONFIG_PS2_KEYBOARD)
	ps2_kb_err = ps2_keyboard_init(keyboard_callback,
				       &current_scan_code);
#endif
	/* In case of a problem with one of the PS2 devices
	 * report it back to the calller
	 */
	if (ps2_mb_err || ps2_kb_err) {
		return -ENOTSUP;
	}

#if defined(CONFIG_KSCAN_EC)
	int mtx_kb_err;

	mtx_kb_err = kbs_matrix_init(mtx_keyboard_callback,
				       &current_scan_code);
	if (mtx_kb_err) {
		return -ENOTSUP;
	}
#endif

	return 0;
}

int kbc_disable_interface(void)
{
	int ret;

	cmdbyte_disable_kbd();
	cmdbyte_disable_mb();

#if defined(CONFIG_PS2_KEYBOARD)
	ps2_keyboard_disable();
#endif
#if defined(CONFIG_PS2_MOUSE)
	ps2_mouse_disable();
#endif
#if defined(CONFIG_KSCAN_EC)
	kbs_keyboard_disable();
#endif
	/* Disables 8042 IBF irq */
	ret = espihub_kbc_write(E8042_PAUSE_IRQ, 0);
	if (ret) {
		LOG_ERR("%s fail: %d", __func__, ret);
		return ret;
	}

	return 0;
}

int kbc_enable_interface(void)
{
	int ret;

	cmdbyte_enable_kbd();
	cmdbyte_enable_mb();

#if defined(CONFIG_PS2_KEYBOARD)
	ps2_keyboard_enable();
#endif
#if defined(CONFIG_PS2_MOUSE)
	ps2_mouse_enable();
#endif
#if defined(CONFIG_KSCAN_EC)
	kbs_keyboard_enable();
#endif
	/* Resumes 8042 IBF irq */
	ret = espihub_kbc_write(E8042_RESUME_IRQ, 0);
	if (ret) {
		LOG_ERR("%s fail: %d", __func__, ret);
		return ret;
	}

	return 0;
}

/* This handles the configuration data from the host for both,
 * keyboard and mouse
 */
void kbc_handler(uint8_t data, uint8_t cmd_data)
{
	struct host_byte host_data;
	static uint8_t repeated_data_hack;

	host_data.data = data;
	host_data.cmd = cmd_data;

	/* Until we understand why the isrs are retriggered. This hack
	 * is necessary in order to avoid returning FE due to  repeated
	 * data which is processed in the DEFAULT_STATE. Therefore, fe
	 * is returned and the host does not want to process further
	 */

	if (repeated_data_hack == data) {
		return;
	}

	repeated_data_hack = data;
	k_msgq_put(&from_host_queue, &host_data, K_NO_WAIT);
}

void kbc_set_leds(uint8_t leds)
{
	int ret = 0;

	k_mutex_lock(&led_mutex, K_FOREVER);

#if defined(KBC_SCROLL_LOCK) && !defined(CONFIG_DEBUG_LED)
	ret = gpio_write_pin(KBC_SCROLL_LOCK, GET_SCROLL_LOCK(leds));
#endif
	if (ret) {
		LOG_ERR("Unable to write scroll lock led");
	} else {
		if (GET_SCROLL_LOCK(leds)) {
			current_leds_state |= BIT(SCROLL_LOCK_POS);
		} else {
			current_leds_state &= ~BIT(SCROLL_LOCK_POS);
		}
	}

#if defined(KBC_NUM_LOCK) && !defined(CONFIG_DEBUG_LED)
	ret = gpio_write_pin(KBC_NUM_LOCK, GET_NUM_LOCK(leds));
#endif
	if (ret) {
		LOG_ERR("Unable to write num lock led");
	} else {
		if (GET_NUM_LOCK(leds)) {
			current_leds_state |= BIT(NUM_LOCK_POS);
		} else {
			current_leds_state &= ~BIT(NUM_LOCK_POS);
		}
	}

#if defined(KBC_CAPS_LOCK) && !defined(CONFIG_DEBUG_LED)
	ret = gpio_write_pin(KBC_CAPS_LOCK, GET_CAPS_LOCK(leds));
#endif
	if (ret) {
		LOG_ERR("Unable to write caps lock led");
	} else {
		if (GET_CAPS_LOCK(leds)) {
			current_leds_state |= BIT(CAPS_LOCK_POS);
		} else {
			current_leds_state &= ~BIT(CAPS_LOCK_POS);
		}
	}

	k_mutex_unlock(&led_mutex);

#if !defined(KBC_SCROLL_LOCK) &&  !defined(KBC_NUM_LOCK) \
	&& !defined(KBC_CAPS_LOCK)
	LOG_ERR("All leds are undefined");
#endif
}

int kbc_get_leds(void)
{
	int ret;

	k_mutex_lock(&led_mutex, K_FOREVER);
	ret = current_leds_state;
	k_mutex_unlock(&led_mutex);

	return ret;
}

/* Typically data len will by 1 expect for the cases where we have to
 * fake the keyboard ID for Kscan devices.
 */
static void send_kb_to_host(uint8_t data)
{
	k_msgq_put(&to_host_kb_queue, &data, K_NO_WAIT);
	k_sem_give(&kb_p60_sem);
}

static void purge_kb_queue(void)
{
	k_msgq_purge(&to_host_kb_queue);
}

