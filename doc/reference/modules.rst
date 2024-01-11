.. modules

EC FW application modules
#########################

The embedded controller is divided in multiple software modules.
Currently we support the following modules and planning to add more in the
future.

.. image:: ecfw_zephyr_modules.png
  :width: 600px
  :align: center

Each module section describes the functionality, main flows, implementation
details and configuration options supported.

.. toctree::
   :maxdepth: 1

   task_handling/index.rst
   power_sequencing/index.rst
   peripheral/index.rst
   smc/index.rst
   kbchost/index.rst
   kscan/index.rst
   dnx/index.rst
