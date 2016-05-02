#!/bin/bash


switch=$1
netaddr=$2
netmask=$3

if [ -z "$switch" ]
then
    switch=/tmp/vde.ctl
    echo "switch: $switch"
fi

if [ -z "$netaddr" ]
then
    netaddr=192.168.10.1
    echo "netaddr: $netaddr"
fi

if [ -z "$netmask" ]
then
    netmask=255.255.255.0
    echo "netmask: $netmask"
fi

sudo ifconfig tap0 $netaddr netmask $netmask up 2>/dev/null || ( sudo tunctl && sudo ifconfig tap0 $netaddr netmask $netmask up )
sudo vde_switch -s $switch -tap tap0
