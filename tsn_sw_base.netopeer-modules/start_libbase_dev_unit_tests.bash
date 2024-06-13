#!/bin/bash

#vendor/ceedling is needed - TAKE CARE ABOUT VERSIONS
if [ ! -d "../vendor/ceedling" ]
then
    cd ..
    mkdir -p -m=777 vendor/ceedling
    wget https://github.com/ThrowTheSwitch/Ceedling/archive/v0.28.3.tar.gz
    tar -xvf v0.28.3.tar.gz -C vendor/ceedling --strip-components 1
    #unity sources
    wget https://github.com/ThrowTheSwitch/Unity/archive/v2.4.1.tar.gz
    tar -xvf v2.4.1.tar.gz
    cp -r Unity-2.4.1/src vendor/ceedling/vendor/unity
    cp -r Unity-2.4.1/auto vendor/ceedling/vendor/unity
    cp -r Unity-2.4.1/release vendor/ceedling/vendor/unity
    #cmock sources
    wget https://github.com/ThrowTheSwitch/CMock/archive/v2.4.5.tar.gz
    tar -xvf v2.4.5.tar.gz
    cp -r CMock-2.4.5/src vendor/ceedling/vendor/cmock
    cp -r CMock-2.4.5/lib vendor/ceedling/vendor/cmock
    cp -r CMock-2.4.5/release vendor/ceedling/vendor/cmock
    cp -r CMock-2.4.5/config vendor/ceedling/vendor/cmock
	cd netopeer-modules
fi

#/test/support is needed
if [ ! -d "../test/support" ]
then
    cd ..
    mkdir -p test/support
	cd netopeer-modules
fi

#libbase ceedling test suit generation
#ceedling new libbase 
cd libbase
#execute libbase tests
ceedling test:all
ceedling clean

