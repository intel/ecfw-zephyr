/*
 * Copyright (c) 2020 Intel Corporation
 */

/**
 * @brief PECI interface APIs.
 *
 */

#ifndef __PECI_HUB_H__
#define __PECI_HUB_H__

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

/**
 * @brief Get CPU temperature.
 *
 * This function fetches cpu core temperature using peci.
 *
 * @param *temperature address of temperature variable.
 * @retval 0 on success and failure code on error.
 */
int peci_get_temp(int *temperature);

/**
 * @brief Get CPU maximum junction temperature.
 *
 * This function fetches maximum cpu core juntion
 * temperature using peci.
 *
 * @param *tjmax address of temperature variable.
 * @retval 0 on success and failure code on error.
 */
int peci_get_tjmax(u8_t *tjmax);

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
int peci_rdpkg_config(u8_t *req_buf, u8_t *resp_buf, u8_t rd_len);

#endif /* __PECI_HUB_H__ */
