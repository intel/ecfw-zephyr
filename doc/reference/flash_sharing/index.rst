.. _flash_sharing:

Support for EC flash sharing
############################

.. contents::
    :local:
    :depth: 3

eSPI Flash sharing overview
***************************

Enhanced Serial Peripheral Interface (eSPI) is a public system management interface specification led by Intel; license free to OEMs/EC vendors.
Flash sharing is one of the features supported by eSPI which its main intention is to save the cost of the dedicated EC flash (known sometimes as G3 boot in Intel documentation).
This is achieved by removing the need to have dedicated internal/external SPI flash for the embedded controller, where EC will be sharing SPI flash with the rest of platform FW (Integrated firmware).
The eSPI flash sharing model allows 2 additional configurations Controller Attached Flash (previously known as MAF) and Target Attached Flash (aka SAF).

  .. image:: espi_spec_flash_sharing.png
     :align: center
     :scale: 80%

Controller Attached Flash
=========================
In CAF (MAF) case, the SPI flash is directly connected to Intel SoC.
EC ROM bootloaders commonly retrieve the EC FW by issuing eSPI Flash Channel read operations managed by the PCH SPI Flash Controller.
Note that PCH SPI Flash Controller HW enforces security from all requesters.

  .. image:: caf_flash_sharing_boot_config.png
     :align: center
     :scale: 60%

Target Attached Flash
=====================
In TAF (SAF) case, the SPI flash is connected to EC. Intel SoC IPs retrieve their FW over eSPI.
PCH access SPI flash by issuing eSPI Flash Channel read and write operations which are overseen internally by the EC’s SPI Flash Controller.
Concisely, EC needs to function as a bridge translating eSPI flash requests into SPI flash requests.

  .. image:: taf_flash_sharing_boot_config.png
     :align: center
     :scale: 60%

Dedicated EC SPI flash (G3)
===========================
Even though it is possible that the EC SoC has an internal flash, Intel RVP uses the same SPI flash as the rest of Intel FW.
The HW design allows EC temporary access to it prior to releasing RSMRST.
While EC asserts RSMRST#, the PCH tristate the SPI bus pins, and the EC has exclusive access to the SPI flash. If RSMRST# is de-asserted, PCH has exclusive access to it.

.. note:: If EC needs to access SPI flash in runtime after RSRMRST is de-asserted, EC can use eSPI driver flash APIs

  .. image:: g3_flash_sharing_boot_config.png
     :align: center
     :scale: 60%

EC FW requirements for flash sharing
************************************
Intel RVP designs are by default configured as CAF(MAF); however, the design allows us to evaluate the other flash sharing boot configurations via well-defined reworks.
Since the EC FW is the same across all these configurations, it needs to discriminate in runtime the flash sharing boot configuration.

.. note:: In a real design the boot configuration detection is known, so this detection flow is unnecessary.
          EC FW should only perform the actions relevant to individual configuration.

EC FW flash sharing boot config detection in Intel RVP
======================================================

  .. image:: flash_sharing_boot_config_detection_flow.png
     :align: center
     :scale: 70%


CAF(MAF) requirements
=====================
Since EC ROM bootloader de-asserted RSMRST and performed eSPI channel negotiation handshake and Zephyr eSPI driver managed the rest.
EC FW application responsibility regarding CAF is ensuring that during RSMRST pin configuration this is glitch free.

TAF(SAF) requirements
=====================

1. EC FW prepares SPI flash device for TAF bridge access.

2. EC FW initializes EC HW TAF (SAF) bridge for autonomous flash access passing any additional information not available in the Zephyr TAF device tree node.

3. EC FW activates the EC HW TAF (SAF) bridge.

4. EC de-asserts RSMRST to indicate Intel SoC, this indicates that eSPI handshake can initiate.

  .. image:: flash_sharing_boot_config_detection_flow_details.png
     :align: center


SPI flash vendor requirements for TAF
-------------------------------------
Enabling SPI flash read continuous mode makes, the read instruction opcode optional; this reduces overhead during PCH eSPI flash read operations which is highly desirable.
For this reason, TAF bridge assumes the SPI flash device used supports Quad mode (all SPI lines used) and continuous read mode.

SPI flash preparations
----------------------
As indicated before, EC FW TAF module prepares SPI flash device for TAF bridge access, which is enabling Quad and continuous modes if not already enabled.
This should be done before de-asserting RSMRST to guarantee that Intel SoC does not attempt to perform any operations before the TAF bridge is ready.

.. note:: An EC Zephyr flash driver could perform this configuration too.
     Basic EC Open-source project uses CONFIG_FLASH Kconfig to discriminate these cases. Refer to the diagram above for details.


Basic TAF bridge configuration
------------------------------
Prior to the TAF bridge HW activation, need to configure the following SPI flash device details:

 * Number of SPI flash devices
 * Capacity of each SPI flash device

Additionally, it may be required to specify any opcodes for the specific SPI flash device not already supported by this project.

 * Write enable.
 * Erase suspend.
 * Program suspend.
 * Read.
 * Read status.

+-----------------------------+--------------------------+-------------+
| SPI flash vendor            | Status register opcode   | Busy bit    |
+=============================+==========================+=============+
|  Winbond W25R512NWEIQ       | 15h                      | 0           |
+-----------------------------+--------------------------+-------------+
|  Gigadevice GD25LR256EYIGR  | 15h                      | 0           |
+-----------------------------+--------------------------+-------------+
|  Macronix MX77U51250FZ4I42  | 15h                      | 0           |
+-----------------------------+--------------------------+-------------+

Table 1: Common opcode across supported vendors to perform read status.


+-----------------------------+--------------------------+-------------+
| SPI flash vendor            | Status2 register opcode  | Suspend bit |
+=============================+==========================+=============+
|  Winbond W25R512NWEIQ       | 35h                      | 7           |
+-----------------------------+--------------------------+-------------+
|  Gigadevice GD25LR256EYIGR  | 70h                      | 6           |
+-----------------------------+--------------------------+-------------+
|  Macronix MX77U51250FZ4I42  | 2Bh                      | 3           |
+-----------------------------+--------------------------+-------------+

Table 2: Different read status2 opcode across supported vendors.

.. note:: The use of the correct Register Status2 opcode to obtain the suspend bit status is key when enabling Intel SoC out-of-order TAF(SAF) requests support.


.. note:: Commonly EC FW performs the SPI flash opcode configuration via Zephyr eSPI TAF API in runtime.
         However, it is possible that EC-specific Zephyr flash driver performs these settings using the static values in its device tree node properties.
         Refer to EC-vendor specific documentation regarding how to set these parameters.



Security TAF bridge configuration
---------------------------------
In TAF(SAF) configuration, all Intel SoC/PCH SPI Flash transactions are routed over the eSPI Flash Access channel to EC.
So these SPI Flash access transactions are not subject to Intel SoC and region security checks, instead EC is responsible for it all related access and security controls. This security mechanism is based on region protection and Tags in EC.
This should be enabled via espi_saf_set_protection_regions API prior to TAF bridge activation.
Basic EC open source does not provided reference code for this, refer to EC-specific vendor examples.
