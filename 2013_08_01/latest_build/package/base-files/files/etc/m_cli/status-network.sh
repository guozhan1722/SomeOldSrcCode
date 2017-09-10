#!/bin/sh

. /usr/lib/webif/webif.sh

show_dns(){
   dns=""
   x=0
   if [ -s /etc/resolv.conf ]; then
      linecount=`cat /etc/resolv.conf 2>&1 | grep nameserver | wc -l | tr -d ' '` 
      while [ "$x" -lt "$linecount" ]
      do
         let x=x+1
         dns=`cat /etc/resolv.conf |grep nameserver| head -n $x | tail -n 1 | awk '{print $2}'`
         echo -e "\tDNS Servers : \t$dns"
      done
   else
      echo -e "\tDNS Servers : \t$dns"
   fi
}

preinterface() {
	local iface="$1"
	local iname="$2"
	equal "$iface" "" || equal "$iname" "" && return 1
	equal "$iface" "lo" && return 1
	equal "`ifconfig "$iface" 2>/dev/null`" "" && return 1
	echo ""
	case "$iname" in
		WAN) echo "Interfaces Status | WAN Interface" ;;
		LAN) echo "Interfaces Status | LAN Interface" ;;
		*)   echo "Interfaces Status Other|Interface $iname" ;;
	esac
	hwaddr="`ifconfig "$iface" |grep HWaddr|awk '{print $5}' 2>/dev/null`"
	echo -e "\tMac address : \t$hwaddr"
	ipaddr="`ifconfig "$iface" |grep "inet addr"|awk '{print $2}'|awk -F ":" '{print $2}'`"
	echo -e "\tIP address  : \t$ipaddr"	
	netmask="`ifconfig "$iface" |grep "Mask"|awk '{print $4}'|awk -F ":" '{print $2}'`"
	echo -e "\tnet mask    : \t$netmask"	
	rxpackets="`ifconfig "$iface" |grep "RX packets"|awk '{print $2}'|awk -F ":" '{print $2}'`"
        show_dns
	echo -e "\tRX packets  : \t$rxpackets"	
	txpackets="`ifconfig "$iface" |grep "TX packets"|awk '{print $2}'|awk -F ":" '{print $2}'`"
	echo -e "\tTX packets  : \t$txpackets"	
	rxbytes="`ifconfig "$iface" |grep "RX bytes"|awk '{print $2}'|awk -F ":" '{print $2}'`"
	echo -e "\tRX bytes    : \t$rxbytes Bytes"	
	txbytes="`ifconfig "$iface" |grep "TX bytes"|awk '{print $6}'|awk -F ":" '{print $2}'`"
	echo -e "\tTX bytes    : \t$txbytes Bytes"	
}

preinterface "br-lan" "LAN"
preinterface "br-wan" "WAN"

