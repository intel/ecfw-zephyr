/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief System control interrupt codes.
 */

#ifndef SCI_CODES_H_
#define SCI_CODES_H_

/* SMBus event notification */
#define SCI_SMB                 0x01

/* Device bay insertion */
#define SCI_DEVINSERTION        0x20
/* Device bay removal */
#define SCI_DEVREMOVAL          0x21
/* NewCard insertion event */
#define SCI_NEWCARD             0x22

/* AC insertion */
#define SCI_ACINSERTION         0x30
/* AC removal */
#define SCI_ACREMOVAL           0x31
/* Battery event */
#define SCI_BATTERY             0x32
/* Battery insertion or removal */
#define SCI_BATTERY_PRSNT       0x33
/* Battery Pmax */
#define SCI_BAT_PMAX            0x34
/* Battery Psus */
#define SCI_BAT_PSUS            0x35
/* Battery Cycle Count */
#define SCI_BAT_CYCLE_CNT       0x36
/* Power Source change */
#define SCI_PSRC_CHANGE         0x37
/* Battery RBHF */
#define SCI_BAT_RBHF            0x38
/* Battery VBNL */
#define SCI_BAT_VBNL            0x39
/* Battery CMPP */
#define SCI_BAT_CMPP            0x3A

/* Dock complete */
#define SCI_DOCKED              0x40
/* Undock complete */
#define SCI_UNDOCKED            0x41
/* Undocking request */
#define SCI_UNDOCKREQUEST       0x42
/* Express card insertion/removal */
#define SCI_EXPCARDPRSNT        0x43
/* Virtual Dock */
#define SCI_VIRTDOCK            0x44

/* ring indicate event */
#define SCI_RING                0x50
/* lid */
#define SCI_LID                 0x51
/* keyboard hotkey */
#define SCI_HOTKEY              0x52
/* virtual battery switch */
#define SCI_VB                  0x53
/* Power button */
#define SCI_PWRBTN              0x54
/* Resuming from S3 */
#define SCI_RESUME              0x55
/* CAS Key */
#define SCI_HOTKEY_CAS          0x56

/* Bluetooth power off */
#define SCI_BT_PWR_OFF          0x60
/* Bluetooth power on */
#define SCI_BT_PWR_ON           0x61

/* ALS lux change */
#define SCI_ALS                 0x70

#define SCI_EVNT_USBC_ASYNC     0x76
/* USB-C Attach/Dettach */
#define SCI_USB_ATCH_DTCH_WA    0x77
/* USB-C SW interface event */
#define SCI_EVNT_USBC           0x79

/* Volume Up */
#define SCI_VU_PRES             0x80
/* Volume Down */
#define SCI_VD_PRES             0x81
/* Slatemode switch press */
#define SCI_SLATEMODE_PRESS     0x82
/* Slatemode switch release */
#define SCI_SLATEMODE_RELEASE   0x83
/* Home Button */
#define SCI_HB_PRES             0x85
/* Power Button */
#define SCI_PB                  0x86
/* Rotation lock */
#define SCI_ROT_REL             0x87
/* Volume Up */
#define SCI_VU_REL              0x88
/* Volume Down */
#define SCI_VD_REL              0x89
/* Home Button */
#define SCI_HB_REL              0x8A
/* Home Button */
#define SCI_ROT_PRES            0x8B

/* Burst acknowledge byte */
#define SCI_BURST_ACK           0x90

/* for Geyserville break event */
#define SCI_GYSRVL              0xB0

/* AON Up button */
#define SCI_AON_UP              0xD0
/* AON Down button */
#define SCI_AON_DOWN            0xD1
/* AON Select button */
#define SCI_AON_SELECT          0xD2
/* AON Escape button */
#define SCI_AON_ESC             0xD3
/* AON Global Escape button */
#define SCI_AON_GESC            0xD4
#define SCI_PWRBTN_DOWN         0xD5
#define SCI_PWRBTN_UP           0xD6
#define SCI_OTG_ID              0xD7
#define SCI_VBUS_SENSE          0xD8

/* Thermal transition */
#define SCI_THERMAL             0xF0
/* Thermal trip point transition */
#define SCI_THERMTRIP           0xF1
/* RPM Trip point transition */
#define SCI_RPMTRIP             0xF2

#endif /* SCI_CODES_H_ */
