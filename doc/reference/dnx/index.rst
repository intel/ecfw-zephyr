.. _download_and_execute:

EC support for Intel Download and Execute
#########################################

.. contents::
    :local:
    :depth: 3

This note describes the requirements and implementation details for EC FW
to support Intel Download and execute (DnX) flows. It covers an overview of the
EC role in the DnX flow validated in Intel RVP and a set of optional features
available for derivative board designs.

DnX overview
************

DnX (download and execute) is a capability wherein Intel SoC CSE component can
use USB port on the device and download content from a host machine. CSE can
download content as an execution unit (it verifies the content and executes it)
or as a data unit (it writes the content in SPI/eMMC). Primary purposes:

1) User-initiated update/downgrade
2) User-initiated provision a system for debug
3) Recover detectable corruption
4) Provision virgin system
5) Token injection

.. note:: For additional details do refer to Intel DnX Firmware specification.

EC functional requirements for DnX
**********************************
A) Disable EC timeout mechanism after gaining awareness of system DnX entry.
B) Detect user desire to trigger DnX entry (optional).
C) Perform handshake with Intel SoC to trigger DnX mode entry (optional).

.. note::
  In earlier designs EC played a role to put USB-C port in device mode,
  which is required in designs where DnX capable USB port is exposed via USB-C
  connector. This is no longer required in newer architectures.


1. EC DnX awareness and EC timeout
==================================
As indicated in Intel `EC FW reference documentation`_, during system
transitions (boot, resume, hibernate, shutdown) EC will check if certain
conditions are met, if any of these conditions is not fulfilled within a
predetermined time interval the power sequencing will stop and timeout.

During DnX entry, the power sequence is intentionally stopped in early stages,
so once SoC enters DnX and EC FW becomes aware of DnX condition, it should not
perform any timeouts to avoid disrupt the DnX flow, i.e. shutting down the
primary well.

.. note:: For additional details do refer to Intel DnX Firmware specification.
  If eSPI is not supported, Intel RVP and this reference code showcase the
  EC HW strap, that allows to disable EC FW timeout.


2. User-triggered EC-assisted DnX entry
=======================================
Intel RVP does provide a jumper setting that allow to trigger DnX entry.
The jumper is directly connected to SoC HW strap that triggers DnX when the
system is powered up (G3 exit).

In such scenario, implementing EC DnX awareness alone satisfies the main DnX
requirement.

.. image:: jumper_assisted_dnx_entry.png
   :align: center


If said jumper is not available in a derived board design, EC can assist the
DnX entry of the system after intercepting the user desire to enter DnX mode.
In such case, EC will directly control the Intel SoC DNX_FORCE_RELAD strap.

.. image:: ec_assisted_dnx_entry.png
   :align: center

EC can detect any user-trigger based on pre-determined action via any of the
peripherals connected to EC, i.e. holding volume up + volume down, pressing
multiple keys in a sequence in keyboard matrix (hotkey), etc.

EC can perform such detection as early as G3 exit, but there are some
considerations based on system SPI boot configurations.

  .. image:: spi_boot_config.png
     :align: center

The main constraint in Master-attached flash (MAF) mode is that DnX HW strap
is sampled on RSMRST signal de-assertion, which in MAF is handled by EC ROM
bootloader directly, preventing EC FW to detect any user action prior to
this action.

Also, needs to be considered that volume keys and matrix keyboards are not part
of all form factors and require user physical interaction with system which
could be difficult in remote debug sessions.

For such reasons, the most scalable method is to be pre-determined USB keyboard
hotkey as user DnX trigger while the system is already powered on (S0).

Since EC is not aware of USB keyboard activity, BIOS must propagate the user
desire to EC.

  .. image:: user_dnx_entry_triggers.png
     :align: center

.. note:: Reference code provides support to trigger DnX entry via BIOS to EC
  command so alternatively BIOS can also implement a menu, or EFI application.

3. Non-user triggered DnX entry
===============================

Handle of virgin or blank SPI / corrupt EC FW.

If a system has not yet been programmed with SPI flash image or its content is
detected by CSE as corrupt, CSE automatically identifies this condition and
enters DnX state to allow to update the SPI flash.

From most EC ROM vendor viewpoint these are considered as non-recoverable
failures, however once DnX is entered, user can still update the SPI image.

Main constraint in this scenario is that EC cannot perform any eSPI DnX warn
virtual wire handshake or perform system reset (followed by EC soft reset)
whenever the global reset occurs.
In such cases, a manual system power cycle will be required.

Implementation details
**********************

By default, in the reference code implementation only enables the EC DnX
awareness feature. The additional DnX-related features can be enabled via
KConfig.

+--------------+--------+-------+---------------------------------------+
|              | DnX    |Timeout|                                       |
| Boot SPI cfg | jumper |jumper | EC FW support recommendation          |
+==============+========+=======+=======================================+
| G3/SAF/MAF   | Yes    |  Yes  | * EC timeout-disable HW strap         |
|              |        |       |                                       |
+--------------+--------+-------+---------------------------------------+
| G3/SAF/MAF   | Yes    |  No   | * EC DnX awareness                    |
|              |        |       |                                       |
+--------------+--------+-------+---------------------------------------+
| G3/SAF       | No     |  No   | * Detect user trigger for DnX entry   |
|              |        |       | * Communicate DnX intention to CSME   |
+--------------+--------+-------+---------------------------------------+
| MAF          | No     |  No   | * Detect user trigger for DnX entry   |
|              |        |       | * Initiate/wait for system restart    |
|              |        |       | * Detect system restart               |
|              |        |       | * Communicate DnX intention to CSME   |
|              |        |       |                                       |
+--------------+--------+-------+---------------------------------------+

EC FW DnX awareness
===================

Platform requirements:
	eSPI-enabled platform
	IFWI

  .. image:: ec_dnx_awareness_flow.png
     :align: center

Flow:

1) EC shall intercept eSPI DnX virtual wire warning sent by eSPI controller
as soon as EC slave boot done is sent to eSPI host indicating EC FW has been
retrieved.

2) Upon receiving DnX WARN VW = 1, EC shall stop flash access, disable timeout
mechanism and send back DnX ACK VW = 1.

3) Upon receiving DnX WARN VW = 0, EC shall send back DNX ACK = 0, any features
requiring EC flash access are allowed.

4) If a global reset occurs while DnX WARN = 1, in addition to platform reset
usually performed when eSPI reset occurs, EC shall perform a soft reset.
This is required since this may be used to indicate FW update through DnX.


EC FW DnX-related optional features
===================================

1) User-triggered EC-assisted DnX entry

As indicated above, EC can perform user intention via different actions, the
reference code showcase BIOS to EC command support. Upon receiving this command
EC needs to set the DnX awareness flag for later usage.

As described in Intel DnX documentation, the DnX entry can achieved by
setting DNX_FORCE_RELOAD HW strap, so EC needs to assert this pin as part of the
DnX trigger sequence.

.. note:: Intel SoC samples HW straps in RSMRST going high, so recommended flow
  relies in pseudo-G3 entry/exit flow.

Recommended configuration:
  1) Pseudo-G3 enabled in BIOS
  2) Battery inserted

High level flow:
  1) EC receives command via EFI shell for DnX trigger
  2) EC sets DnX HW strap (DNX_FORCE_RELOAD)
  3) Manually enter S5 i.e. press power button.
  4) EC de-asserts PCH_PWROK as part of S5 entry.
  5) EC evaluates pseudo-G3 conditions and sets EC_DS3.
  6) Board logic performs pseudo-G3 entry.
  7) Board logic performs RSMRST/DPWROK de-assertion.
  8) Manually exit S5 via power button.
  9) Pseudo-G3 exit sequence starts.
  10) DnX strap re-evaluated, DnX is entered.
  11) DnX Enumeration seen in the HOST connected to system.

  .. image:: ec_assisted_dnx_entry_via_smc_command_flow.png
     :align: center

3) EC DnX non-volatile awareness flag

This could allow to save DnX awareness as non-volatile flag, allowing
persistence across power cycles.
In this case, EC will perform same GPIO handshake with SoC mentioned above.

4) Instead of BIOS-triggered system restart, EC can request the system restart
over eSPI once it gets notification of user desire to enter DnX.

Other considerations
====================

1) DnX awareness flag persistence.

If DnX awareness flag is a non-volatile special handling is required to clear
the flag. Recommend cases are:

* Whenever DnX warn = 0 is received.
* Whenever EC FW image gets updated in SPI flash.
* Platform boots to S0 after been in DnX mode.

2) EC-assisted vs jumper-assisted DnX entry coexistence.

Whenever there is dedicated DnX jumper in the system design EC should ensure
GPIO pin is not driven or if board circuitry permits it to configure as open
drain output to allow control if required.


Supporting MAF boot when controlling DNX_FORCE_RELOAD
=====================================================
Systems support MAF need to consider part of the boot flow is handled by
EC-vendor specific bootloader. In most of cases it is as follows.

1) Detect MAF SPI boot configuration
2) De-assert RSMRST
3) Perform eSPI handshake with Intel SoC
4) Fetch EC FW over eSPI bus flash channel.
5) Ensure validity of the FW image and handover control to EC FW.

This limits any DnX flow triggered by user in MAF in the first G3 exit,
since RSMRST is de-asserted by EC-vendor bootloader prior to EC FW been able
to detect user trigger. Additionally, this requires EC vendor support to
acknowledge Intel SoC eSPI warning when DnX is entered.

.. _EC FW reference documentation:
    https://intel.github.io/ecfw-zephyr/reference/power_sequencing/index.html#id7
