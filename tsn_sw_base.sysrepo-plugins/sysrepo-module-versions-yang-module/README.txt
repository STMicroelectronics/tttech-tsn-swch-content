## sysrepo-module-versions sysrepo plugin

Yang file notes:
- This is proprietary yang module.
- This version represents the updated version of old module-versions yang module. Please look at the yang module revisions to see modifications.
- The file sysrepo_module_versions_data.xml represents the datastore used for filling up state data.

## module specific dependencies

Defines and functions from common directory.

Libraries: libbase.

Look at the sysrepo-plugins README.txt for sysrepo and netopeer dependencies.


## install plugin

> $ cd YANG_modules_repository/

> $ sudo sysrepoctl --install --yang=sysrepo-module-versions.yang

> $ cd sysrepo-module-versions-yang-module/

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

> 	> get --filter-xpath /sysrepo-module-versions:*

There should be the sysrepo-module-versions module config and state data. 

