## ieee802-dot1cb-stream-identification sysrepo plugin

## module specific dependencies

Defines and functions from common directory.

Libraries: libbase, libtsn, liblldpctl, libbsd, libyang

Look at the sysrepo-plugins README.txt for sysrepo and netopeer dependencies.


## install plugin

> $ cd YANG_modules_repository/

> $ sudo sysrepoctl --install --yang=ieee802-dot1cb-stream-identification.yang

> $ cd ieee802-dot1cb-stream-identification-yang-module/

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

> 	> get --filter-xpath /ieee802-dot1cb-stream-identification:*

There should be the ieee802-dot1cb-stream-identification module config and state data. 

