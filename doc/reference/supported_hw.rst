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

Custom HW
=========
For generic guidelines to adapt basic EC open source to custom HW design.

.. toctree::
   :maxdepth: 1

   porting_ecfw_custom_hw/index.rst


MECC card HW limitations
========================

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
|                  |  MECC spec               | 1.0     | 1.0     | 1.0     |
+------------------+--------------------------+---------+---------+---------+
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
