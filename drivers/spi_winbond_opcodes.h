/*
 * Copyright (c) 2022 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SPI_WINBOND_OPCODES_H__
#define __SPI_WINBOND_OPCODES_H__

/* Winbond SPI commands */
#define READ_STATUS_1_OPCODE           0x05U
#define WRITE_ENABLE_OPCODE            0x06U
#define READ_STATUS_2_OPCODE           0x35U
#define BLOCK_ERASE_32K_OPCODE         0x52U
#define ENABLE_RESET_OPCODE            0x66U
#define ERASE_SUSPEND_OPCODE           0x75U
#define ERASE_RESUME_OPCODE            0x7AU
#define RESET_OPCODE                   0x99U
#define CONTINUOUS_MODE_OPCODE         0xA5U
#define FOUR_BYTE_ADDRESS_ENTER_OPCODE 0xB7U
#define EXIT_QPI_OPCODE                0xFFU
#define WRITE_STATUS2_OPCODE           0x31U
#define WRITE_ENABLE_VS_OPCODE         0x50U
#define POWER_DOWN                     0xB9U
#define RELEASE_POWER_DOWN             0xABU

#define READ_VOLATILE_CFG_OPCODE       0x85U
#define WRITE_VOLATILE_CFG_OPCODE      0x81U

/* Map opcodes based on SPI flash capacity, since first require use 2-byte
 * addresses and later required 3-byte addresses.
 */
#if (CONFIG_SAF_SPI_CAPACITY == 16)
#define PAGE_PROGRAM_OPCODE            0x02U
#define SECTOR_ERASE_OPCODE            0x20U
#define BLOCK_ERASE_64K_OPCODE         0xD8U
#define FAST_READ_DUAL_IO_OPCODE       0xBBU
#define FAST_READ_QUAD_IO_OPCODE       0xEBU
#define QUAD_WRITE_DATA_OPCODE         0x32U
#elif (CONFIG_SAF_SPI_CAPACITY == 32)
#define PAGE_PROGRAM_OPCODE            0x12U
#define SECTOR_ERASE_OPCODE            0x21U
#define BLOCK_ERASE_64K_OPCODE         0xDCU
#define FAST_READ_DUAL_IO_OPCODE       0xBCU
#define FAST_READ_QUAD_IO_OPCODE       0xECU
#define QUAD_WRITE_DATA_OPCODE         0x34U
#else
#pragma error "Unsupported SPI capacity"
#endif

#endif /* __SPI_WINBOND_OPCODES_H__ */
