#!/bin/bash

howmany=1
if [ $# -eq 1 ]
then
    howmany=$1
fi
echo "Destroying" $howmany "bridge(s)"

clear;                           \
sudo rmmod edgx_sw_core;         \
sudo rmmod edgx_mdio;            \
sudo rmmod edgx_hif_platform;    \
dmesg

for ((sw=0; sw<$howmany; sw++))
do
    nbr="br"$sw
    echo "Destrying bridge "$nbr
    sudo ip link set dev $nbr down
    sudo ip link del dev $nbr
done

