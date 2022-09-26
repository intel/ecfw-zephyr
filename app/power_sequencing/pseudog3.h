/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __PSEUDO_G3_H__
#define __PSEUDO_G3_H__

enum pg3_counter {
	PG3_COUNTER_AC,
	PG3_COUNTER_DC,
	PG3_COUNTER_AC_2_DC,
	PG3_COUNTER_DC_2_AC,
	PG3_COUNTER_TOTAL,
	PG3_COUNTER_UNDEF
};

/**
 * @brief Enable or disable pseudo g3.
 *
 * @param mode set or reset pseudo g3.
 */
void pseudo_g3_enable(bool state);

/**
 * @brief Get the current state of Pseudo G3.
 *
 * @return true if system is in pseudo g3 otherwise false.
 */
bool pseudo_g3_get_state(void);

/**
 * @brief Get the previous state of Pseudo G3.
 *
 * @return true if system is in pseudo g3 otherwise false.
 */
bool pseudo_g3_get_prev_state(void);

/**
 * @brief Program counter value for respective Pseudo G3 counter.
 *
 * @param counter pg3_counter to be programmed.
 * @param count is a 32bit value in seconds.
 */
void pseudo_g3_program_counter(enum pg3_counter counter, uint32_t count);

/**
 * @brief Manage pseudo g3 state.
 *
 * This function should be called from the power management task.
 *
 */
void manage_pseudog3(void);

#endif /*__PSEUDO_G3_H__*/
