## ieee802-dot1q-fqtss sysrepo plugin

Yang file notes:
- Yang generated from MIB
- ieee8021BridgeBaseComponentId and ieee8021BridgeBasePort objects are manualy added to yang module
- Yang module indepentent of bridge yang module
	
	
## module specific dependencies

Defines and functions from common directory.

Libraries: libbase, libtsn, libbsd, libyang

Look at the sysrepo-plugins README.txt for sysrepo and netopeer dependencies.


## install plugin

> $ cd YANG_modules_repository/

> $ sudo sysrepoctl --install --yang=ieee802-dot1ab-lldp.yang

> $ cd ieee802-dot1q-fqtss-yang-module/

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

> 	> get --filter-xpath /ieee802-dot1q-fqtss:*

There should be the ieee802-dot1q-fqtss module config and state data. 

