#!/bin/bash

if [[ $# -ne 4 ]]; then
	echo "Illegal number of parameters"
	usage()
fi

PCAP=$1
IFACE=$2
TIMES=$3

tcpreplay -i$IFACE -l$TIMES $PCAP

