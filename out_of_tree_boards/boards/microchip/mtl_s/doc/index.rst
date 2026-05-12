.. _mec172X_mtl_s:

MEC172X MTL_S
##############

Overview
********

The MEC172X MTL-S is Intel referance platform for Meteorlake.

Hardware
********

- MC1723NB0SZ ARM Cortex-M4 Processor
- 416 KB RAM and 128 KB boot ROM
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
- One external PCA9555 I/O port with jumper selectable I2C address.
- One I2C interface to add-in card based Type-C header.

For more information about the SOC please see the `MEC1723 Reference Manual`_

Supported Features
==================

The mec172x_mtl_s board configuration supports the following hardware
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
:zephyr_file:`boards/arm/mec172x_mtl_s/mec172x_mtl_s_defconfig`
Kconfig file.

Connections and IOs
===================

Please refer MTL rvp schematics for EC gpio allocations and IOs.
https://intel.sharepoint.com/:x:/r/sites/ccgmtlclientsystempdt/_layouts/15/Doc.aspx?sourcedoc=%7BDAC0E8C7-16CE-4D52-8346-2EC736864194%7D&file=MTL-S_MEC1728_GPIO_MAPPING_Rev0p12_WW11p4.xlsx
System Clock
============

The MEC1723 MCU is configured to use the 48Mhz internal oscillator with the
on-chip PLL to generate a resulting EC clock rate of 12 MHz. See Processor clock
control register in chapter 4 "4.0 POWER, CLOCKS, and RESETS" of the data sheet in
the references at the end of this document.

Serial Port
===========

UART0 is configured for serial logs.

Building
========
#west build -c -p auto -b mec172x_mtl_s

References
**********
.. target-notes::

.. _MEC172x Preliminary Data Sheet:
    https://github.com/MicrochipTech/CPGZephyrDocs/blob/master/MEC172x/MEC172x-Data-Sheet.pdf
.. _MEC172x Reference Manual:
    https://github.com/MicrochipTech/CPGZephyrDocs/blob/master/MEC172x/MEC172x-Data-Sheet.pdf
.. _MEC17xx EVB Schematic:
    https://github.com/MicrochipTech/CPGZephyrDocs/blob/master/MEC172x/MEC172x-MECC_Assy_6930-A1p0-SCH.pdf
.. _MEC172x Daughter Card Schematic:
    https://github.com/MicrochipTech/CPGZephyrDocs/blob/master/MEC172x/MEC172X-EVB-Assy_6906-A1p0-SCH.pdf
.. _SPI Image Gen:
    https://github.com/MicrochipTech/CPGZephyrDocs/tree/master/MEC172x/SPI_image_gen