#
# Copyright (c) 2024 Intel Corporation.
#
# SPDX-License-Identifier: Apache-2.0
#

find_file (NPCX_SPI_CFG NAMES spi_cfg.txt PATHS ${BOARD_DIR}/support)
find_file (NPCX_SPI_GEN NAMES npcx_spi_img_gen.sh PATHS ${BOARD_DIR})

set_property(GLOBAL APPEND PROPERTY extra_post_build_commands
    COMMAND pwd
    COMMAND echo "${NPCX_SPI_CFG}"
    COMMAND echo "${NPCX_SPI_GEN}"
    COMMAND echo "${PROJECT_BINARY_DIR}"
    COMMAND echo "${NPCX_SPI_GEN} -i ${NPCX_SPI_CFG} -f ${PROJECT_BINARY_DIR}/zephyr.npcx.bin -o ${PROJECT_BINARY_DIR}/spi_image.bin"
    COMMAND ${NPCX_SPI_GEN} ${NPCX_SPI_CFG} ${PROJECT_BINARY_DIR}/zephyr.npcx.bin ${PROJECT_BINARY_DIR}/spi_image.bin
  )
