.. _getting_started:

Getting started
###############

.. contents::
    :local:
    :depth: 3

This project is based on Zephyr RTOS, so a Zephyr development environment is
required.

Pre-requisites
==============
* GitHub account

* Ubuntu 18.04 LTS or later.

.. note:: Alternatively `Ubuntu shell`_ installed on Windows can be used.

.. note:: If working in VPN or intranet environment need to set-up company proxy
          correctly for the command line tools which may include but not limited
          to apt-get wget, pip, git.

.. _Ubuntu shell:
   https://ubuntu.com/tutorials/ubuntu-on-windows#1-overview


Setting Up Zephyr environment
=============================
Refer to the official `Zephyr's Getting Started Guide`_ while reviewing
considerations below during each step.

.. note:: Latest EC FW Open source project is based on Zephyr v3.4 so need to
          refer to Zephyr v3.4 documentation and use Zephyr SDK 0.16.1.

1) OS selection
---------------
Recommended OS is Ubuntu since environment setup is simpler and Zephyr SDK is not
available in Windows.


2) Install dependencies
-----------------------
Steps 2.1 and 2.2 from Zephyr guide are mandatory.

.. note:: If the tool is already present in the Ubuntu host, uninstall it and
          get the latest from a third-party Ubuntu repository as indicated.


3) Get Zephyr and Python dependencies
-------------------------------------
Steps 3.1, 3.2, 3.3, 3.4, 3.5 and 3.7 from Zephyr guide are mandatory.
This will retrieve a copy Zephyr RTOS and install all python dependencies.

.. note:: Zephyr copy cloned during this step is the latest kernel version under
          development and is not used as part of EC FW project.

4) Install Zephyr SDK and toolchain
-----------------------------------
Steps 4.1, 4.2 and 4.3 from Zephyr guide are mandatory.
Step 4.4 is only required if flashing HW via command line instead of using
Dediprog.

.. note:: A toolchain is needed in some cases to compile for a specific EC SoC.
          Zephyr SDK already contains several toolchains.
          This project uses Microchip MEC15xx/MEC172x which is an arm-based
          microcontroller, so gnu arm-eabi toolchain included in SDK can be used.

5) EC SoC vendor-specific setup
-------------------------------
Currently only MEC15xx and MEC172x are supported.
Perform steps 1 to 3 from Setup section in `MEC15xx EVB Setup guide`_ and
steps 2 to 4 from Setup section in `MEC172x EVB Setup guide`_

.. note::  GitHub repo root folder is `MEC SPI generator tools`_


Getting EC FW framework code
============================

1) Clone main repository
------------------------
Create a folder sandbox and clone ecfw-zephyr project

.. code-block:: bash

   mkdir ~/sandbox
   cd ~/sandbox
   git clone https://github.com/intel/ecfw-zephyr

2) Obtain the dependencies
--------------------------
Navigate to west manifest location inside ecfw-zephyr and re-initialize west

.. code-block:: bash

   cd ecfw-zephyr
   west init -l

See EC FW's dependencies

.. code-block:: bash

   west list

+---------------+-----------------------+-------------+-----------------------------------------------------+
| repo          | destination           | revision    | external repository                                 |
+===============+=======================+=============+=====================================================+
| zephyr        | zephyr_fork           | v3.4.0      | https://github.com/zephyrproject-rtos/zephyr        |
+---------------+-----------------------+-------------+-----------------------------------------------------+
| cmsis         | modules/hal/cmsis     | 74981bf     | https://github.com/zephyrproject-rtos/cmsis         |
+---------------+-----------------------+-------------+-----------------------------------------------------+
| hal_microchip | modules/hal/microchip | 5d079f1     | https://github.com/zephyrproject-rtos/hal_microchip |
+---------------+-----------------------+-------------+-----------------------------------------------------+

Retrieve all external project dependencies

.. code-block:: bash

   west update


Your directory structure should look like this:

.. code-block::

   sandbox
   |____ecfw-zephyr               This repository
   |____ecfwwork/modules          Zephyr RTOS modules includes EC SoC HAL
   |____ecfwwork/zephyr_fork      Snapshot from Zephyr RTOS


3) Apply back-ported fixes
--------------------------
Some additional patches are required to be applied to the Zephyr kernel
for building the open source EC FW application. The latest release is based out
of Zephyr v3.4 and hence these patches need to be applied on that branch.

These patches are expected to be part of the future Zephyr releases (if
they are not already integrated).

Note: Below steps assume environment has been setup as indicated in the
main Intel Open source EC FW documentation.

1) Check out the main branch

.. code-block:: bash

   cd ecfw-zephyr
   git checkout master

2) Apply zephyr patches on to the kernel branch

.. code-block:: bash

   cd ../ecfwwork/zephyr_fork
   git am ../../ecfw-zephyr/zephyr_patches/patches_v3.4.patch


Troubleshoot
------------
If west init throws an error during the step above, it means it detected a previous
west installation. It's safe to delete/backup pre-existing ".west" folder

For other problems refer to `west documentation`_.

Building and flashing
=====================

1) Set zephyr environment variables
-----------------------------------
Run the following command to indicate to Zephyr SDK tool active Zephyr path.

.. code-block:: bash

   cd ~/sandbox/ecfwwork/zephyr_fork
   source zephyr-env.sh

.. note:: Multiple zephyr trees can coexist under same environment, setting
          ZEPHYR_BASE environment variable through this script allows to switch
          between them.

2) Build EC FW
--------------
Go back to ecfw-zephyr main folder and build the application and all its dependencies.
Below commands indicate details for each board. See Supported hardware section
for more details.

.. code-block:: bash

   cd ~/sandbox/ecfw-zephyr
   # Building for MTL-S/ARL -S (on-board EC)
   west build -c -p auto -b mec172x_mtl_s

   # Building for MTL-P (on-board EC)
   west build -c -p auto -b mec1501_mtl_p

   # Building for TGL + MECC card (deprecated)
   west build -c -p always -b mec1501modular_assy6885 -- -DCONFIG_MEC15XX_AIC_ON_TGL=y

   # Building for MTL-P + MECC card (i.e. mec172x)
   west build -c -p always -b mec172xmodular_assy6930

.. note:: Additional EC vendors are enabling their MECC cards in Zephyr.
          Similar build is possible replacing -b <modular board>.


If build is successful, zephyr.bin and ksc.bin will be generated.

.. note:: For modular cards this steps generates spi_image.bin instead.

.. code-block:: bash

   ls  ~/sandbox/ecfw-zephyr/build/zephyr/*.bin

Troubleshoot
------------
1) If build command gives error indicating no ZEPHYR_BASE or no zephyr
repository, make sure that step 1) is executed correctly.

.. code-block:: bash

   printenv | grep ZEPHYR_BASE

2) If spi_image.bin is not generated, revisit EC vendor-specific setup.

    a) Ensure SPI generator is available

    .. code-block:: bash

       printenv | grep SPI_GEN

    b) Confirm the file is executable

    .. code-block:: bash

       ls -l /usr/local/bin/everglades_spi_gen_lin64

    If file is not executable, make it executable.

    .. code-block:: bash

       chmod +x /usr/local/bin/everglades_spi_gen_lin64


Flash EC FW
-----------
Flash EC FW as indicated in Official Intel documentation in RVP user guide using
Dediprog. Refer to `Intel documentation`_.


.. _Zephyr's Getting Started Guide:
    https://docs.zephyrproject.org/3.2.0/develop/getting_started/index.html

.. _west documentation:
   https://docs.zephyrproject.org/latest/develop/west/index.html

.. _MEC15xx EVB Setup guide:
   https://docs.zephyrproject.org/latest/boards/arm/mec1501modular_assy6885/doc/index.html#programming-and-debugging

.. _MEC172x EVB Setup guide:
   https://docs.zephyrproject.org/latest/boards/arm/mec172xevb_assy6906/doc/index.html#programming-and-debugging

.. _MEC SPI generator tools:
   https://github.com/MicrochipTech/CPGZephyrDocs

.. _Intel documentation:
   https://www.intel.com/content/www/us/en/design/resource-design-center.html
