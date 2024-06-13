## ietf-interfaces sysrepo plugin

Implementation of YANG Data Model for Interface Management according to [RFC 7223](https://tools.ietf.org/html/rfc7223).

The ietf-interfaces has implemented augment for preemption data according to the ieee802-dot1q-preemption yang module.
The ietf-interfaces has implemented augment for preempt data according to the ieee802-dot3br-preempt yang module.
The ietf-interfaces has implemented augment for sched data according to the ieee802-dot1q-sched yang module.
The ietf-interfaces has implemented augment for ethernet data according to the ieee802-ethernet-interface yang module.
The ietf-interfaces has implemented augment for q-bridge data according to the ieee802-dot1q-bridge yang module.

## module specific dependencies
Sources directory with source and header for preemption data implementation.

Defines and functions from common directory.

Libraries: libbase, libtsn, libbsd, libyang

Look at the sysrepo-plugins README.txt for sysrepo and netopeer dependencies.


## install plugin
> $ cd YANG_modules_repository/

> $ sudo sysrepoctl --install --yang=ietf-interfaces.yang-module

> $ sudo sysrepoctl --install --yang=iana-if-type.yang 				[need for identity-ref type]

> $ sudo sysrepoctl --install --yang=ieee802-dot1q-preemption.yang

> $ sudo sysrepoctl --install --yang=ieee802-dot3br-preempt.yang

> $ sudo sysrepoctl --install --yang=ieee802-dot1q-sched.yang

> $ sudo sysrepoctl --install --yang=ieee802-dot1q-bridge.yang

> $ sudo sysrepoctl --install --yang=ieee802-ethernet-interface.yang

> $ sudo sysrepoctl --feature-enable frame-preemption --module ieee802-dot1q-preemption		[need to enable feature to implement preemption data]

> $ sudo sysrepoctl --feature-enable mac-merge --module ieee802-dot3br-preempt				[need to enable feature to implement preempt data]

> $ sudo sysrepoctl --feature-enable scheduled-traffic --module ieee802-dot1q-sched			[need to enable feature to implement sched data]

> $ sudo sysrepoctl --feature-enable ethernet-pfc --module ieee802-ethernet-interface		[need to enable feature to implement ethernet data]

> $ sudo sysrepoctl --feature-enable ethernet-pause --module ieee802-ethernet-interfacen	[need to enable feature to implement ethernet data]

> $ cd ietf-interfaces-yang-module/

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

> 	> get --filter-xpath /ietf-interfaces:*

There should be the ietf-interfaces module config and state data. 

