#!/bin/bash

usage() {
	echo -e "usage:\t$0 <interface> <iterations> <pcap_file>"
}

if [[ $# -ne 3 ]]; then
	echo "Illegal number of parameters"
	usage
	exit 1
fi

IFACE=$1
TIMES=$2
PCAP=$3

tcpreplay -i$IFACE -l$TIMES $PCAP

