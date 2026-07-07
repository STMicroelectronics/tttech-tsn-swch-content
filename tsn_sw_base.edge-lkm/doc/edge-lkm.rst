==========
EDGE - LKM
==========

This document describes the structure and usage of the EDGE-LKM device driver.
These procedures apply for systems based on Altera Cyclon V SoC Reference design
e.g. DE-Evaluation Board. 

Structure
=========

EDGE-LKM consists of the following kernel modules:

* Altera Parallel I/O to Linux GPIO Driver (*altera_pio.ko*)
    + typically used to reset PHY devices
* EDGE Platform Host Interface Driver (*edgx_hif_platform.ko*)
    + implements the HW-interface specific part of the driver
* EDGE Switch Core Driver (*edgx_sw_core.ko*)
    + implements the HW-interface independent part of the driver
* EDGE MDIO Driver (*edgx_mdio.ko*)
    + used to create MDIO buses, they can be used later to connect 
      to PHY devices

Building
========

To compile EDGE-LKM natively for the host system environment use: ::

  make modules

To install EDGE-LKM on the host: ::

  make modules_install

To cross-compile EDGE-LKM for a specific architecture:

1. Set-up your cross compilation toolchain

2. Compile kernel modules: ::

     make ARCH=your_architecture CROSS_COMPILE=your_toolchain_prefix- INSTALL_MOD_PATH=path_to_your_target modules

3. Install modules on the target: ::

     make ARCH=your_architecture CROSS_COMPILE=your_toolchain_prefix- INSTALL_MOD_PATH=path_to_your_target modules_install

Configuration
=============

Device Tree Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~

Required device tree entries:

* EDGE Platform Host Interface Driver: ::

    deip-sw@BRIDGE_DEVICE_BASE_ADDRESS {
        compatible = "ttt,deip-sw";
        reg = <BRIDGE_DEVICE_BASE_ADDRESS BRIDGE_MEMORY_REGION_SIZE>;
        interrupts = <BRIDGE_INTERRUPT_VECTOR>;
    };

* EDGE MDIO Driver: ::

    eth_mdio@MDIO_DEVICE_BASE_ADDRESS {
        compatible = "flx,eth-mdio";
        reg = <MDIO_DEVICE_BASE_ADDRESS MDIO_MEMORY_REGION_SIZE>;
    };

* Altera Parallel I/O to Linux GPIO Driver: ::

    pio0: gpio@GPIO_DEVICE_BASE_ADDRESS {
        compatible = "flx,pio";
        reg = <GPIO_DEVICE_BASE_ADDRESS GPIO_MEMORY_REGION_SIZE>;
        gpio-controller;
        width = <1>;
        direction = <0x1>;
   };

Please refer to your device user manual for more information.

Example device tree for the DE-Evaluation Board: ::

    soc {
        deip-sw@c2000000 {
            compatible = "ttt,deip-sw";
            reg = <0xc2000000 0x02020000>;
            interrupts = <0 40 0x4>;
        };

        eth_mdio@c00f0200 {
            compatible = "flx,eth-mdio";
            reg = <0xc00f0200 0x100>;
        };

        pio0: gpio@c00f0f00 {
            compatible = "flx,pio";
            reg = <0xc00f0f00 0x20>;
            gpio-controller;
            width = <1>;
            direction = <0x1>;
       };
   };

EDGE Switch Core Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The EDGE Switch Core Driver module exposes the following parameters:

* *netif*
    Bridge- and End Station tunnelling network interface 
    (xMII connectivity only!)

* *mgmttc*
    Management Traffic Class; defaults to highest available traffic class (uint)

Example: ::

  modprobe edgx_sw_core netif="eth0:0"

For each bridge port, PHY delays shall be configured for each supported 
transmission speed via these sysfs interfaces: ::

  /sys/class/net/BRIDGE_PORT_INTERFACE/phy/delay10tx
  /sys/class/net/BRIDGE_PORT_INTERFACE/phy/delay100tx
  /sys/class/net/BRIDGE_PORT_INTERFACE/phy/delay1000tx
  /sys/class/net/BRIDGE_PORT_INTERFACE/phy/delay10rx
  /sys/class/net/BRIDGE_PORT_INTERFACE/phy/delay100rx
  /sys/class/net/BRIDGE_PORT_INTERFACE/phy/delay1000rx

Loading
=======

Perform the following steps on the target system in the prescribed order 
to start EDGE-LKM:

* Load *edgx_hif_platform.ko* and *altera_pio.ko*
* Reset PHY devices using the *altera_pio* driver
* Load *edgx_mdio.ko*
* Bring the management interface up (i.e. *eth0*)
* Load *edgx_sw_core.ko* with the management interface parameter
* Assign an MDIO bus to each bridge port
* Set MAC addresses on each bridge port
* Set delay parameters on each bridge port

Please refer to start-up scripts in the reference design software package 
e.g. DE-Evaluation Board.