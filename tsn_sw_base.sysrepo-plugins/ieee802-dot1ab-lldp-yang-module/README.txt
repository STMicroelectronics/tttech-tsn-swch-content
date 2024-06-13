## ieee802-dot1ab-lldp sysrepo plugin

Yang file is adapted from [ieee802-dot1ab-lldp.yang](https://github.com/YangModels/yang/blob/master/standard/ieee/802.1/draft/ieee802-dot1ab-lldp.yang).

## module specific dependencies

Defines and functions from common directory.

Libraries: libbase, liblldpctl, libbsd, libyang

Look at the sysrepo-plugins README.txt for sysrepo and netopeer dependencies.


## install plugin

> $ cd YANG_modules_repository/

> $ sudo sysrepoctl --install ieee802-dot1ab-lldp.yang

> $ sudo sysrepoctl --change ieee802-dot1ab-lldp -p 666 --owner devel

> $ cd ieee802-dot1ab-lldp-yang-module/

> $ mkdir build; cd build

> $ cmake ..

> $ make

> $ sudo make install

## Run sysrepo-plugind and netopeer2-server

> $ sudo sysrepo-plugind -v 4 -d

> $ sudo netopeer2-server -d -v 9

> $ netopeer2-cli

> 	> connect --host 192.168.0.1 --login root --port 830

> 	> get --filter-xpath /ieee802-dot1ab-lldp:*

There should be the ieee802-dot1ab-lldp module config and state data. 

