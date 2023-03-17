.. _mec1501_mtl_p:

MEC1501 MTL_P
##############

Overview
********

The MEC1501 MTL-P is Intel referance platform for Meteorlake.

Hardware
********

- MEC1501HB0SZ ARM Cortex-M4 Processor
- 256 KB RAM and 64 KB boot ROM
- PS2 Keyboard and Mouse interface
- UART0, UART1, and UART2
- FAN0 header
- FAN PWM interface
- JTAG/SWD, ETM and MCHP Trace ports
- PECI interface 3.0
- I2C voltage translator
- Port-80 display
- eSPI header
- CAPS LOCK, NUM LOCK and SCROLL LOCK LEDs
- Volume up, down and power buttons
- Two external PCA9555 I/O port with jumper selectable I2C address
- One I2C interface to add-in card based Type-C header

For more information about the SOC please see the `MEC1501 Reference Manual`

Supported Features
==================

The mec1501_mtl_p board configuration supports the following hardware
features:

+-----------+------------+-------------------------------------+
| Interface | Controller | Driver/Component                    |
+===========+============+=====================================+
| NVIC      | on-chip    | nested vector interrupt controller  |
+-----------+------------+-------------------------------------+
| SYSTICK   | on-chip    | systick                             |
+-----------+------------+-------------------------------------+
| UART      | on-chip    | serial port                         |
+-----------+------------+-------------------------------------+
| GPIO      | on-chip    | gpio                                |
+-----------+------------+-------------------------------------+
| I2C       | on-chip    | i2c                                 |
+-----------+------------+-------------------------------------+
| PINCTRL   | on-chip    | pinctrl                             |
+-----------+------------+-------------------------------------+
| PS/2      | on-chip    | ps2                                 |
+-----------+------------+-------------------------------------+

Other hardware features are not currently supported by Zephyr (at the moment)

The default configuration can be found in the
:zephyr_file:`boards/arm/mec1501_mtl_p/mec1501_mtl_p_defconfig`
Kconfig file.

Connections and IOs
===================

Please refer MTL RVP schematics for EC GPIO allocations and IOs.
https://intel.sharepoint.com/:x:/r/sites/ccgmtlclientsystempdt/ArchLZPRD/MTL_M_P_RVP_Architecture_HAS/GPIO_Mappin_SoC_EC/MTL-P_MEC1521_GPIO_MAPPING_Rev0p75_WW25P3.xlsx

System Clock
============

The MEC1501 MCU is configured to use the 48Mhz internal oscillator with the
on-chip PLL to generate a resulting EC clock rate of 12 MHz. See Processor clock
control register in chapter 4 "4.0 POWER, CLOCKS, and RESETS" of the data sheet in
the references at the end of this document.

Serial Port
===========

UART2 is configured for serial logs.

Building
========
#west build -c -p auto -b mec1501_mtl_p

References
**********
.. target-notes::

.. _MEC1501 Preliminary Data Sheet:
    https://github.com/MicrochipTech/CPGZephyrDocs/blob/master/MEC1501/MEC1501_Datasheet.pdf
.. _MEC1501 Reference Manual:
    https://github.com/MicrochipTech/CPGZephyrDocs/blob/master/MEC1501/MEC1501_Datasheet.pdf
.. _MEC15xx EVB Schematic:
    https://github.com/MicrochipTech/CPGZephyrDocs/blob/master/MEC1501/Everglades%20EVB%20-%20Assy_6853%20Rev%20A1p1%20-%20SCH.pdf
.. _MEC1501 Daughter Card Schematic:
    https://github.com/MicrochipTech/CPGZephyrDocs/blob/master/MEC1501/MEC1501%20Socket%20DC%20for%20EVERGLADES%20EVB%20-%20Assy_6883%20Rev%20A0p1%20-%20SCH.pdf
.. _MEC1503 Daughter Card Schematic:
    https://github.com/MicrochipTech/CPGZephyrDocs/blob/master/MEC1501/MEC1503%20Socket%20DC%20for%20EVERGLADES%20EVB%20-%20Assy_6856%20Rev%20A1p0%20-%20SCH.pdf
.. _SPI Dongle Schematic:
    https://github.com/MicrochipTech/CPGZephyrDocs/blob/master/MEC1501/SPI%20Dongles%20and%20Aardvark%20Interposer%20Assy%206791%20Rev%20A1p1%20-%20SCH.pdf
.. _SPI Image Gen:
    https://github.com/MicrochipTech/CPGZephyrDocs/tree/master/MEC1501/SPI_image_gen
