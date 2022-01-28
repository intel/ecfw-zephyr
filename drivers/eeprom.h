/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief EEPROM APIs.
 *
 */

#ifndef __EEPROM_H__
#define __EEPROM_H__

/**
 * @brief Read a byte from a EEPROM offset.
 *
 * @param offset to read from EEPROM.
 * @param data read from EEPROM.

 * @retval 0 if success, error otherwise.
 */
int eeprom_read_byte(uint16_t offset, uint8_t *data);

/**
 * @brief Read a word from given offset.
 *
 * @param offset to read from EEPROM.
 * @param data read from EEPROM.
 *
 * @retval 0 if success, error otherwise.
 */
int eeprom_read_word(uint16_t offset, uint16_t *data);

/**
 * @brief Read EEPROM block from given offset.
 *
 * @param offset from EEPROM.
 * @param length the amount of bytes to read.
 * @param buffer pointer where data will be copied.
 *
 * @retval 0 if success, error otherwise.
 */
int eeprom_read_block(uint16_t offset, uint8_t read_len, uint8_t *buffer);

/**
 * @brief Write EEPROM byte to given offset.
 *
 * @param offset from EEPROM to perform write.
 * @param data to write in EEPROM.
 * @param buffer pointer to the information to write in EEPROM.
 *
 * @retval 0 if success, error otherwise.
 */
int eeprom_write_byte(uint16_t offset, uint8_t data);


/**
 * @brief Write EEPROM block to given offset.
 *
 * @param offset from EEPROM.
 * @param data the information to write in EEPROM.
 *
 * @retval 0 if success, error otherwise.
 */
int eeprom_write_word(uint16_t offset, uint16_t data);

/**
 * @brief Write EEPROM block to given offset.
 *
 * @param offset from EEPROM.
 * @param length the amount of bytes to write.
 * @param buffer pointer to data.
 *
 * @retval 0 if success, error otherwise.
 */
int eeprom_write_block(uint16_t offset, uint8_t write_len, uint8_t *buffer);

#endif /* __EEPROM_H__ */
