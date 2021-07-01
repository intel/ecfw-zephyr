EC-based fan control.

This branch is a preview of upcoming support in EC open source.
The current version is based on Zephyr v2.3 nevertheless, feature will be
release officially using Zephyr v2.5

EC-based fan control depends on PECI driver to retrieve CPU temperature.
The PECI driver that is part of Zephyr v2.3 doesn't contain the latest
fixes.
In order to evaluate need to apply Zephyr kernel delta patches.

Note: Below steps assume environment has been setup as indicated in the
main Intel Open source EC FW documentation.


1) Obtain the EC ADL thermal app note code
cd ecfw-zephyr
git checkout adl_thermal_support

2) Apply zephyr delta required for thermal
cd zephyr_snapshot
git am ../ecfw-zephyr/zephyr_patches/

3) Building the code for ADL-S
west build  -c -p always -b mec1501_adl
