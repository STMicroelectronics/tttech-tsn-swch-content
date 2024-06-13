## ieee802-dot1q-bridge sysrepo plugin

Yang file is download from [ieee802-dot1q-bridge-yang-module.yang](https://https://github.com/YangModels/yang/blob/master/standard/ieee/published/802.1/ieee802-dot1q-bridge.yang).

## module specific dependencies

Defines and functions from common directory.

Libraries: libbase, liblldpctl, libbsd, libyang

Look at the sysrepo-plugins README.txt for sysrepo and netopeer dependencies.


## install plugin

> $ cd YANG_modules_repository/

> $ sudo sysrepoctl --install --yang=ieee802-dot1q-bridge.yang

> $ cd ieee802-dot1q-bridge-yang-module/

> $ mkdir build; cd build

> $ cmake ..

> $ make

> $ sudo make install

## Run sysrepo and netopeer

> $ sudo sysrepod

> $ sudo sysrepo-plugind -d -l 4

> $ sudo netopeer2-server -d -v 9

> $ netopeer2-cli

> 	> connect --host 192.168.0.1 --login root --port 830

> 	> get --filter-xpath /ieee802-dot1q-bridge:*

There should be the ieee802-dot1q-bridge module config and state data. 

