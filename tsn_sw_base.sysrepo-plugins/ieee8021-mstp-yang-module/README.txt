## ieee8021-mstp sysrepo plugin

Yang file is created from MIB

NOTE: 
Leaf ieee8021MstpCistBridgePriority implemented as config false with int32 type.
Leaf ieee8021MstpRootPort will not be visible in get request if function get_mstp_root_port returns "none" (if root port value is "none" on board), because this leaf is uint32 inside ieee8021-mstp.yang.
Function that gets value for leaf ieee8021MstpPortDesignatedPort edited so return value is integer now. Leaf type is IEEE8021BridgePortNumber (unit32) so its range is 1-65535.
If function returns "0" then that leaf will not be filled up, so it will not be visible in get request.

## module specific dependencies

Defines and functions from common directory.

Libraries: libbase, libbsd, libyang

Look at the sysrepo-plugins README.txt for sysrepo and netopeer dependencies.


## install plugin

> $ cd YANG_modules_repository/

> $ sudo sysrepoctl --install --yang=ieee8021-mstp.yang

> $ cd ieee8021-mstp-yang-module/

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

> 	> get --filter-xpath /ieee8021-mstp:*

There should be the ieee8021-mstp module config and state data. 

