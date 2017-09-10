#!/bin/sh

. /usr/lib/webif/webif.sh

###################################################################
# TCP/IP status page
#
# Description:
#	Shows connections to the router, netstat stuff, routing table..
#
# Author(s) [in order of work date]:
#	Original webif developers
#	Jeremy Collake <jeremy.collake@gmail.com>
#
# Major revisions:
#
# NVRAM variables referenced:
#	todo
#
# Configuration files referenced:
#		none
#
echo "Physical Connections|Ethernet/Wireless Physical Connections"
cat /proc/net/arp
echo ""
echo "Routing Table|Routing Table"
netstat -rn 
echo ""
echo "Router Listening Ports|Router Listening Ports"
netstat -ln 2>&- | awk '$0 ~ /^Active UNIX/ {ignore = 1}; ignore != 1 { print $0 }' 
echo ""
echo "Router Connections|Connections to the Router"
netstat -n 2>&- | awk '$0 ~ /^Active UNIX/ {ignore = 1}; ignore != 1 { print $0 }' 