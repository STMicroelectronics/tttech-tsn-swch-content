#!/bin/bash

echo "sudo rm /usr/lib/libbase.s*"
sudo rm /usr/lib/libbase.s*
echo "sudo rm -r /usr/include/base/"
sudo rm -r /usr/include/base/

sudo make clean

#make all DEBUG=-DDEBUG VM_COMPILE=-DVM_COMPILE
make all VM_COMPILE=-DVM_COMPILE

echo "sudo cp ./build/libbase.s* /usr/lib"
sudo cp ./build/libbase.s* /usr/lib
echo "sudo cp -a ../libbase/include/ /usr/include/base/"
sudo cp -a ../libbase/include/ /usr/include/base/

sudo make dyn_app DEBUG=-DDEBUG VM_COMPILE=-DVM_COMPILE
#sudo make dyn_app VM_COMPILE=-DVM_COMPILE

echo "./build/libbase_test_app_dynamic"
./build/libbase_test_app_dynamic

