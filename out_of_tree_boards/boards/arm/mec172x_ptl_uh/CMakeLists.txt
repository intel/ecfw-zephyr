#
# Copyright (c) 2024 Microchip Technology Inc.
#
# SPDX-License-Identifier: Apache-2.0
#

#Allow users to pre-specify the tool using '-DMEC172X_SPI_GEN=<path-to-tool>/toolname'
if (NOT DEFINED MEC172X_SPI_GEN)
  set(MEC172X_SPI_GEN $ENV{MEC172X_SPI_GEN})
endif()
find_program (MEC172X_SPI_GEN NAMES mec172x_spi_gen_lin_x86_64 mec172x_spi_gen)
if(MEC172X_SPI_GEN STREQUAL MEC172X_SPI_GEN-NOTFOUND)
  message(WARNING "Microchip SPI Image Generation tool is not available. SPI Image will not be generated.")
endif()

if (NOT DEFINED MEC172X_SPI_CFG)
  set(MEC172X_SPI_CFG $ENV{MEC172X_SPI_CFG})
endif()
find_file (MEC172X_SPI_CFG NAMES spi_cfg_4MBit.txt spi_cfg_128MBit.txt spi_cfg.txt PATHS ${BOARD_DIR}/support)
if(MEC172X_SPI_CFG STREQUAL MEC172X_SPI_CFG-NOTFOUND)
  message(WARNING "Microchip SPI Image Generation tool configuration file is not available. SPI Image will not be generated.")
endif()

  if(DEFINED ENV{SPI_IMAGE_NAME})
    set(SPI_IMAGE_NAME $ENV{SPI_IMAGE_NAME})
  else()
    set(SPI_IMAGE_NAME ECFW.bin)
  endif()

if (NOT MEC172X_SPI_GEN STREQUAL MEC172X_SPI_GEN-NOTFOUND AND NOT MEC172X_SPI_CFG STREQUAL MEC172X_SPI_CFG-NOTFOUND)

  # SPI generator has a limitation to generate files with a size multiple of 128KB
  # Generate dummy file to allow merging into it the output from SPI generator
  set_property(GLOBAL APPEND PROPERTY extra_post_build_commands
    COMMAND dd if=/dev/zero ibs=1k count=576 | tr "\\000" "\\377" >
    ${BOARD_DIR}/support/dummy576.bin
  )

  set_property(GLOBAL APPEND PROPERTY extra_post_build_commands
    COMMAND ${MEC172X_SPI_GEN}
    -i ${MEC172X_SPI_CFG}
    -o ${PROJECT_BINARY_DIR}/${SPI_IMAGE_NAME}
    -m ${BOARD_DIR}/support/dummy576.bin
  )

  unset(MEC172X_SPI_GEN)
  unset(MEC172X_SPI_CFG)
endif()
