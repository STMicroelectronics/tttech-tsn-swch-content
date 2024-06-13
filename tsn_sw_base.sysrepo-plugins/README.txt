## sysrepo-plugins 
Implementation of sysrepo-plugins for YANG data modules.

## directory structure
The YANG_modules_repository contains the YOUNG files for all implemented YANG modules.

There are stored also all necessary import modules and augment modules.

Each implemented YANG module has a directory with source code, headers with module specific defines and CMakeLists.txt.

There is a directory 'common' which contains headers and sources with commond defines and common functions used for YANG module implementation.


## dependencies
To implement one YANG module as sysrepo-plugins it is necessary to install libnetconf2, libssh, libyang, Netopeer2, and sysrepo.

To install these libraries and tools just follow the instructions from GitHub:

- [sysrepo](https://github.com/sysrepo/sysrepo)
- [Netopeer2](https://github.com/CESNET/Netopeer2)
- [libyang](https://github.com/cesnet/libyang)
- [libnetconf2](https://github.com/CESNET/libnetconf2)


## build procedure
To install the new sysrepo-plugin first install yang module to the sysrepo.


> $ sudo sysrepoctl --install --yang=/path/to/module/directory/module_name.yang

Go to the module plugin directory and run next commands to build and install sysrepo plugin:

> $ mkdir build

> $ cd build

> $ cmake ..

> $ make

> $ sudo make install


## Start sysrepo and get data over NETOCONF

Run sysrepod and sysrepo-plugind:

> $ sudo sysrepod

> $ sudo sysrepo-plugind -d -l 4

Run the netopeer2 server:

> $ sudo netopeer2-server -d -v 9

Start netopeer2 cli and get data:

> $ netopeer2-cli

> 	> connect --host 192.168.0.1 --login root --port 830

> 	> get

This 'get' command will return all implemented data.
