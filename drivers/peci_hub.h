/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/**
 * @brief PECI interface APIs.
 *
 */

#ifndef __PECI_HUB_H__
#define __PECI_HUB_H__

/* Delay to allow SOC to accept PECI update command */
#define SOC_RDY_PECI_CMD_DELAY_MS 1U

/**
 * @brief Initialize peci driver interface..
 *
 * This function initializes peci driver interface.
 * User should call this function before calling anyother
 * API in this interface..
 *
 * @retval 0 on success and failure code on error.
 */
int peci_init(void);

enum peci_devices {
	CPU = 0,
	GPU,
};

/**
 * @brief Get CPU temperature.
 *
 * This function fetches cpu core temperature using peci.
 *
 * @param *temperature address of temperature variable.
 * @retval 0 on success and failure code on error.
 */
int peci_get_temp(enum peci_devices dev, int *temperature);

/**
 * @brief Get CPU maximum junction temperature.
 *
 * This function fetches maximum cpu core juntion
 * temperature using peci.
 *
 * @param *tjmax address of temperature variable.
 * @retval 0 on success and failure code on error.
 */
int peci_get_tjmax(enum peci_devices dev, uint8_t *tjmax);

/**
 * @brief Read package config using peci.
 *
 * This function fetches package config using peci.
 *
 * @param *req_buf request buffer.
 * @param *resp_buf response buffer.
 * @param *rd_len read length.
 * @retval 0 on success and failure code on error.
 */
int peci_rdpkg_config(enum peci_devices dev, uint8_t *req_buf,
		      uint8_t *resp_buf, uint8_t rd_len);

/**
 * @brief Write package config using peci.
 *
 * This function writes package config to cpu using peci.
 *
 * @param *req_buf request buffer.
 * @param *resp_buf response buffer.
 * @param *wr_len write length.
 * @retval 0 on success and failure code on error.
 */
int peci_wrpkg_config(uint8_t *req_buf, uint8_t *resp_buf, uint8_t wr_len);

/**
 * @brief Read IA MSR.
 *
 * This function fetches Model Specific Registers (MSR) using peci.
 *
 * @param *req_buf request buffer.
 * @param *resp_buf response buffer.
 * @param *rd_len read length.
 * @retval 0 on success and failure code on error.
 */
int peci_rd_ia_msr(enum peci_devices dev, uint8_t *req_buf,
		   uint8_t *resp_buf, uint8_t rd_len);

/**
 * @brief Write IA MSR.
 *
 * This function writes Model Specific Registers (MSR) using peci.
 *
 * @param *req_buf request buffer.
 * @param *resp_buf response buffer.
 * @param *wr_len write length.
 * @retval 0 on success and failure code on error.
 */
int peci_wr_ia_msr(enum peci_devices dev, uint8_t *req_buf,
		   uint8_t *resp_buf, uint8_t wr_len);

/**
 * @brief Read DIB.
 *
 * This function fetches Device Information Byte (DIB) using peci.
 *
 * @param *dev_info device information byte.
 * @param *rev_num revision number byte.
 * @retval 0 on success and failure code on error.
 */
int peci_get_dib(enum peci_devices dev, uint8_t *dev_info, uint8_t *rev_num);

/**
 * @brief Execute PECI command.
 *
 * This function executes the peci command sent by the peci stress
 * tool via the private host interface.
 *
 * @param *req_buf request buffer.
 * @param *resp_buf response buffer.
 * @param *max_req_buf_size size of the request buffer.
 * @retval 0 on success and failure code on error.
 */
int peci_cmd_execute(uint8_t *req_buf, uint8_t *resp_buf,
		     uint8_t max_req_buf_size);

/**
 * @brief Config peci access mode.
 *
 * This routine set the peci access mode sent by bios
 *
 * @param mode peci access mode.
 */
void peci_access_mode_config(uint8_t mode);

/**
 * @brief Update PL4 offset.
 *
 * This routine updates total source power in PL4 offset.
 *
 * @param mode pl4_value to be updated in pl4 offset.
 * @retval 0 on success and failure code on error.
 */
int peci_update_pl4_offset(uint32_t pl4_value);


#endif /* __PECI_HUB_H__ */
