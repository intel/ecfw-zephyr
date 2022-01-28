/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __KBS_BOOT_KEY_SEQ_H__
#define __KBS_BOOT_KEY_SEQ_H__

typedef void (*kbs_key_seq_detected)(bool pressed);

enum kbs_keyseq_type {
	KEYSEQ_TIMEOUT,
	KEYSEQ_RUNTIME,
	KEYSEQ_CUSTOM0,
	KEYSEQ_CUSTOM1,
	KEYSEQ_MAX_SEQ_COUNT
};

struct kbs_keyseq {
	uint8_t trigger_key;
	uint8_t modifiers;
	kbs_key_seq_detected handler;
	bool detected;
};

#endif /* __KBS_BOOT_KEY_SEQUENCE_H__ */
