.. _supported_hardware:

Supported Hardware
##################

.. contents::
    :local:
    :depth: 3

The main hardware used is Intel Validation Platform (RVP) used in conjunction
with a modular embedded control card (MECC) or when applicable the
on-board EC.

The reference HW used to showcase EC FW open source takes advantage of the
Modular Embedded controller card (MECC) specification, which defines both
pin mapping and provides general guidelines to use different Embedded Controller
Soc to the on-board EC through an add-on card. This allows to evaluate different
EC vendors in an Intel RVP.

.. note:: Reworks are required on RVP to be able to control the RVP from MECC
          card. Refer to `Platform design guide`_ for details.

+--------------------+--------------------------+-------------------------+
| Board              | EC HW configuration      | Support                 |
+====================+==========================+=========================+
|  Tigerlake U       | RVP + MEC152x card       | Deprecated              |
+--------------------+--------------------------+-------------------------+
|  Alderake S        | RVP (on-board MEC152x)   | Deprecated              |
+--------------------+--------------------------+-------------------------+
|  Alderake P        | RVP (on-board MEC152x)   | Deprecated              |
+--------------------+--------------------------+-------------------------+
|  Raptorlake S      |  RVP (on-board MEC152x)  | Not supported           |
+--------------------+--------------------------+-------------------------+
|  Raptorlake P      |  RVP (on-board MEC152x)  | Not supported           |
+--------------------+--------------------------+-------------------------+
|  Meteorlake S      |  RVP (on-board MEC172x)  | Supported               |
+--------------------+--------------------------+-------------------------+
|  Meteorlake P      |  RVP (on-board MEC152x)  | * To be deprecated      |
|                    |                          |                         |
|                    |  RVP + MEC172x card      |                         |
+--------------------+--------------------------+-------------------------+
|  Lunarlake M       | RVP (on-board MEC172x)   | No plans to support     |
+--------------------+--------------------------+-------------------------+
| Pantherlake U/P    | RVP (on board MEC172x)   | * To be added           |
|                    |                          |                         |
|                    | RVP + NPCX4 card         |                         |
|                    |                          |                         |
|                    | RVP + ITE8002 card       |                         |
+--------------------+--------------------------+-------------------------+

Other EC SoC
============
In order to use a different MECC card with a different EC SoC vendor, the vendor
should add its Hardware Abstraction Layer (HAL) and board support package (BSP)
to Zephyr RTOS. See `Zephyr's porting guide`_

The list below presents the minimum drivers required by the EC application to
boot an eSPI-based Intel platform.

* UART
* GPIO
* RTOS timer
* I2C - Port 80 visualization if board supports it.
* eSPI - Required by power sequencing module
* ACPI - Required by SMC host module (EC - BIOS interactions).

The following drivers are needed to exercise other non-boot critical features:

* PS/2 - for PS/2 devices management
* Keyscan - for keyboard matrix

These drivers are required to enable advanced features such as thermal and fan
control.

* PECI - for thermal module
* TACH - for thermal module
* ADC - for thermal module
* PWM - for fan control

Customize EC FW framework for other EC SoC
==========================================

Zephyr uses device tree to describe hardware both EC SoC and board's peripherals.
The EC FW abstracts the device tree using friendly macros which can be customized when
using different EC SoC and/or different board. See boards/<vendor> for more details

.. code-block:: c

   #define I2C_BUS_0   DT_NODELABEL(i2c_smb_0)

For more details, see `Zephyr's device tree guide`_.

GPIO initialization
===================
EC FW app doesn't control/configure all GPIOs on the chip but only the ones used
by the app. This is an important delegation to BSP's where generic app pins
are not configured.

.. note::  For current supported HW via MECC card, see boards folder.

When a pin is intended to be controlled by the EC FW framework it should be
mapped under boardname_chipversion.h to corresponding Zephyr GPIO port and pin.

.. code-block:: c

   #define PCH_PWROK    EC_GPIO_106  /* Board #1 Port A, Pin B */
   #define PCH_PWROK    EC_GPIO_036  /* Board #2 Port B, Pin C */

.. note:: EC_GPIO_XXX is a SoC specific macro, which abstracts GPIO port and
          GPIO pin number since Zephyr supports logical GPIO ports and
          EC FW requires the flexibility to map a signal to different pins.

Similarly, boardname_chipversion.c should contain the actual pin configuration
required by the application input/output, open drain and so on.
See `Zephyr's GPIO reference`_ for Zephyr GPIO flags.


HW limitations
==============

Most of the signals used by onboard EC are routed to MECC connector, however
some signals may be missing on MECC compared to onboard EC.
Refer to RVP schematics.

.. note:: Additionally, some MECC cards implement some variations on the MECC
          spec. However, this is being addressed by discussing with the EC
          vendors to adhere to the spec allowing more consistency.

As indicated above some Intel RVP features available are reduced when using MECC
card. Refer to table below for guidance about what can be verified end-to-end.


+------------------+--------------------------+---------+---------+---------+
| Area             | Feature                  | MTL-P + | MTL-S   | MTL-P   |
|                  |                          | MEC172x | onboard | onboard |
+==================+==========================+=========+=========+=========+
| Power sequencing |  ACPI power (Sx)         | Yes     | Yes     | Yes     |
+------------------+--------------------------+---------+---------+---------+
|                  |  Deep sleep (DSx)        | No      | Yes     | No      |
+------------------+--------------------------+---------+---------+---------+
|                  |  Pseudo G3               | No      | No      | Yes     |
+------------------+--------------------------+---------+---------+---------+
|                  |  SAF                     | No      | Yes     | Yes     |
+------------------+--------------------------+---------+---------+---------+
| Human interface  |  Volume buttons          | Yes     | Yes     | Yes     |
+------------------+--------------------------+---------+---------+---------+
|                  |  Power button            | Yes     | Yes     | Yes     |
+------------------+--------------------------+---------+---------+---------+
|                  |  Home button             | No      | No      | Yes     |
+------------------+--------------------------+---------+---------+---------+
|                  |  Lid switch              | Yes     | No      | Yes     |
+------------------+--------------------------+---------+---------+---------+
| Thermal          |  CPU temperature reading | Yes     | Yes     | Yes     |
+------------------+--------------------------+---------+---------+---------+
|                  |  Fan control             | Yes     | Yes     | Yes     |
+------------------+--------------------------+---------+---------+---------+
| PS2/ scan matrix |  PS2 keyboard            | Yes     | Yes     | Yes(2)  |
+------------------+--------------------------+---------+---------+---------+
|                  |  PS2 mouse               | No (1)  | No      | Yes(2)  |
+------------------+--------------------------+---------+---------+---------+
|                  |  Keyboard matrix         | Yes(3)  | Yes     | Yes     |
+------------------+--------------------------+---------+---------+---------+
| Debug            |  Port 80                 | Yes     | Yes     | Yes     |
+------------------+--------------------------+---------+---------+---------+
|                  |  Serial port (UART)      | Yes(4)  | Yes     | Yes     |
+------------------+--------------------------+---------+---------+---------+

.. note:: (1) MEC172x HW revision 2 supports PS2 Keyboard on PortA and PS2 Mouse
          on PortB. Mouse has not been verified though.

.. note:: (2) There is no PS2 connector on the MTL-P board, instead there is
          1x5 header that allow the connection.

.. note:: (3) Intel Meteorlake RVP keyboard matrix connector cannot be used,
          instead need to connect the keyboard matrix directly into the MECC
          card. MEC15xx/MEC17xx cards allow to use Fujitsu keyboard matrix.

.. note:: (4) Intel Meteorlake RVP UART connector cannot be used for EC serial
          logs. Use MECC USB instead.

.. _Platform design guide:
    https://www.intel.com/content/www/us/en/programmable/documentation/lit-index.html

.. _Zephyr's porting guide:
    https://docs.zephyrproject.org/latest/guides/porting/board_porting.html

.. _Zephyr's device tree guide:
    https://docs.zephyrproject.org/latest/guides/dts/index.html

.. _Zephyr's GPIO reference:
   https://docs.zephyrproject.org/latest/reference/peripherals/gpio.html
