# SPDX-License-Identifier: Apache-2.0


target_sources_ifdef(CONFIG_ESPI_SAF app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/saf_config.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/saf_config.h
    )

target_sources_ifdef(CONFIG_SAF_SPI_WINBOND app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/saf_flash_config.c
    ${CMAKE_CURRENT_LIST_DIR}/saf_spi_winbond.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/saf_spi_winbond.h
    )
