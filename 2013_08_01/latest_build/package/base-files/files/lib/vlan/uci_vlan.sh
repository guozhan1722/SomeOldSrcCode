#!/bin/sh
# Copyright (C) 2011 Bob Sheng Microhard Systems Inc

. /etc/functions.sh

VLAN_RUNTIME="/var/run/vlan_enabled"

EBTABLES="echo ebtables"
EBTABLES=ebtables

BRCTL="echo brctl"
BRCTL=brctl
VCONFIG="echo vconfig"
VCONFIG=vconfig

VLAN_DISABLED=1
VLAN_MANAGEMENT=vlan1

config_clear
include /lib/network
scan_interfaces

CONFIG_APPEND=1
config_load vlan

VLAN_LIST=

CUSTOM_CHAINS=1
DEF_INPUT=DROP
DEF_OUTPUT=DROP
DEF_FORWARD=DROP
CONNTRACK_ZONES=
NOTRACK_DISABLED=

find_item() {
	local item="$1"; shift
	for i in "$@"; do
		[ "$i" = "$item" ] && return 0
	done
	return 1
}


vlan_general() {
	[ -n "$GENERAL_APPLIED" ] && {
		echo "Error: multiple general config sections detected"
		return;
	}
	GENERAL_APPLIED=1

	config_get_bool disabled $1 disabled 1
	config_get management $1 management 1
	VLAN_DISABLED=$disabled
	VLAN_MANAGEMENT=$management
	
	logger "DISABLED=$VLAN_DISABLED MANAGEMENT_VLAN=$VLAN_MANAGEMENT"
}

vlan_network() {
	local vlanid

	config_get vlanid $1 id
	[ -n "$vlanid" ] && {
	   append VLAN_LIST $vlanid
	   $BRCTL addbr br-vlan$vlanid >/dev/null 2>&1
	   ifconfig br-vlan$vlanid up >/dev/null 2>&1
	}
}

set_port_vlan() {                                                       
        local ifname="$1" 
        local n="$2" 
        local tagged_vlan="$3" 
        
	    if [ "$n" = "1" ]; then
	    	#native vlan
	        [ "$VLAN_MANAGEMENT" != "vlan1" ] && {
	           # management vlan on other vlan we need to create a bridge for native vlan traffic
	           # otherwise will use br-lan
	           $BRCTL delif br-lan $ifname >/dev/null 2>&1
	           $BRCTL addif br-vlan$n $ifname >/dev/null 2>&1
	        }
	    else
	        [ "$VLAN_MANAGEMENT" == "vlan$n" ] && {
	            [ "$tagged_vlan" == "$n" ] && {
	                $BRCTL delif br-vlan1 $ifname >/dev/null 2>&1
	                $BRCTL addif br-lan $ifname >/dev/null 2>&1
	            } || {
	                $VCONFIG add $ifname $n >/dev/null 2>&1
	                ifconfig $ifname.$n up >/dev/null 2>&1
	                $BRCTL delif br-lan $ifname >/dev/null 2>&1
	                $BRCTL addif br-lan $ifname.$n >/dev/null 2>&1
	            }
	        }
	        
	        [ "$VLAN_MANAGEMENT" != "vlan$n" ] &&  {
	            [ "$tagged_vlan" == "$n" ] && {
	                $BRCTL delif br-lan $ifname >/dev/null 2>&1
	                $BRCTL delif br-vlan1 $ifname >/dev/null 2>&1
	                $BRCTL addif br-vlan$n $ifname >/dev/null 2>&1
	            } || {
	                $VCONFIG add $ifname $n >/dev/null 2>&1
	                ifconfig $ifname.$n up >/dev/null 2>&1
	                $BRCTL addif br-vlan$n $ifname.$n >/dev/null 2>&1
	            }
	        }
	    fi

}

vlan_port() {                         
        local ifname                  
        local vlan_set                
        local tagged_vlan            
                                                 
        config_get ifname $1 ifname
        config_get vlan_set $1 vlan
        config_get tagged_vlan $1 tag 1
                                                                                                                
        iface_set=$(cat $VLAN_RUNTIME | grep $ifname)
        
        for iface in $iface_set; do
            $EBTABLES -t broute -A BROUTING -p 802_1Q -i $iface -j DROP
                                                                             
            for vlan in $vlan_set; do           
                set_port_vlan $iface $vlan $tagged_vlan
            done    
        done                                          
} 

is_port_in_lan() {                                                 
        local ifname 
        config_get ifname $1 ifname
        
        lan_ifaces=$($BRCTL show br-lan | grep $ifname | awk -F' ' '{print $4}')
        [ -z $lan_ifaces ] && lan_ifaces=$($BRCTL show br-lan | grep $ifname | awk -F' ' '{print $1}')
            
        [ -z $lan_ifaces ] && uci_set vlan $1 is_bridged 0 \
            || uci_set vlan $1 is_bridged 1 
            
        [ "$VLAN_DISABLED" == 0 ] && { 
            for lan_iface in $lan_ifaces; do
                echo $lan_iface >> $VLAN_RUNTIME
            done
        }
}
        
vlan_unload_port() {                                            
        local ifname 
                                                                              
        config_get ifname $1 ifname 
        
        iface_set=$(ifconfig | grep $ifname | awk -F' ' '{print $1}')
        
        for iface in $iface_set; do
             $VCONFIG rem $iface >/dev/null 2>&1
        done
}        
            
vlan_init() {
	GENERAL_APPLIED=

	echo "Loading general configuration"
	config_foreach vlan_general general

        echo "Scanning interfaces in lan"                          
        config_foreach is_port_in_lan port                                  
        # commit the is_bridged flag                                        
        uci commit vlan 	
	
	[ "$VLAN_DISABLED" == 0 ] && {
	    echo "Loading vlan networks"
	    config_foreach vlan_network vlan
	    echo "Loading ports"
	    config_foreach vlan_port port
	}
}

vlan_stop() {
        
       # flush all ebtable filters
       $EBTABLES -t broute -F
       
       [ -f $VLAN_RUNTIME ] || return
       
       # remove all br-vlans
       br_vlan_set=$($BRCTL show | grep br-vlan | awk -F' ' '{print $1}')
       for ifname in $br_vlan_set; do
            ifconfig $ifname down
            $BRCTL delbr $ifname
       done 

       # unload ports
       config_foreach vlan_unload_port port
       	    
       #restart network
       lan_ifaces=$(cat $VLAN_RUNTIME)
       for ifname in $lan_ifaces; do
            ifconfig $ifname up >/dev/null 2>&1
            $BRCTL addif br-lan $ifname >/dev/null 2>&1
       done
       
       # remove the runtime file
       rm -rf $VLAN_RUNTIME
}
