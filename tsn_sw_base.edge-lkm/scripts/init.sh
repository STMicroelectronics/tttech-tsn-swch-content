#!/bin/bash

howmany=1
if [ $# -eq 1 ]
then
    howmany=$1
fi
echo "Creating" $howmany "bridge(s)"

pushd `dirname $0` > /dev/null
sdir="$(pwd -P)"
lkmdir=$sdir"/.."
popd > /dev/null

echo " sdir   = "$sdir
echo " lkmdir = "$lkmdir

$sdir/shutdown.sh $1;                                   \
clear;                                                  \
sudo dmesg -c;                                          \
clear;                                                  \
sudo insmod $lkmdir/edgx_hif_platform.ko;               \
sudo modprobe ptp;                                      \
sudo insmod $lkmdir/edgx_sw_core.ko netif="eth0:0";       \
clear;                                                  \
dmesg
#sudo insmod $lkmdir/edgx_mdio.ko

for (( sw=0; sw<$howmany; sw++))
do
    nbr="br"$sw
    echo "  bridge "$nbr
    sudo ip link add name $nbr type bridge ageing_time 100
    for pt in {0..4}
    do
        npt="  sw"$sw"p"$pt
        echo "   |- "$npt
        sudo ip link set dev $npt master $nbr
        sudo ip link set dev $npt up
    done
    sudo ip link set dev $nbr up
done


# Note: this doesn't work in script but DOES work on the command line ...
#bridge vlan add dev sw0p1 vid 120 pvid     self
#bridge vlan add dev sw0p1 vid 120 untagged self

# bridge vlan { add | del } dev DEV vid VID [ pvid ] [ untagged ]  [ self ]  [ master ]
# bridge vlan [ show ] [ dev DEV ]

# : 'self' causes action of physical device
# : 'pvid' the vlan is a PVID at ingress - untagged frames will be assigne to this VLAN.
# : 'untagged' the vlan is to be treated as untagged on egress

