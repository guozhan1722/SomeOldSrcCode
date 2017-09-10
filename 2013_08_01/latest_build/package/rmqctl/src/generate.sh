#!/bin/sh
. /etc/functions.sh

IPTABLES="echo iptables"
IPTABLES=iptables
TC="echo tc"
TC=tc 

ENABLE_QOS=0
LOCAL_QOS=0
LAN_QOS=0
WAN_QOS=0
IFACE=""

config_clear
config_load qos

add_qoscrule() {
    $IPTABLES -t mangle -I $1 \
        ${proto:+-p $proto} \
        ${src_ip:+-s $src_ip} \
        ${src_port:+--sport $src_port} \
        ${src_mac:+-m mac --mac-source $src_mac} \
        ${dest_ip:+-d $dest_ip} \
        ${dest_port:+--dport $dest_port} \
        -j MARK --set-mark "$mark" 
}

QosGeneral() {
    local enabled

    config_get enabled $1 enabled
    GLOABLE_QOS="$enabled" 
}

QosGloble() {
    local target
    local src_ip
    local src_port
    local dest_ip
    local dest_port
    local proto
    local dscp

    config_get target $1 target
    config_get src_ip $1 src_ip
    config_get src_port $1 src_port
    config_get dest_ip $1 dest_ip
    config_get dest_port $1 dest_port
    config_get proto $1 proto
    config_get dscp $1 dscp

    [ "$TC" = "echo tc" ] && echo QosGloble $target $dscp

    add_rule() {
        $IPTABLES -t mangle -I PREROUTING \
            ${proto:+-p $proto} \
            ${src_ip:+-s $src_ip} \
            ${src_port:+--sport $src_port} \
            ${src_mac:+-m mac --mac-source $src_mac} \
            ${dest_ip:+-d $dest_ip} \
            ${dest_port:+--dport $dest_port} \
            -j DSCP --set-dscp "$dscp" 

        $IPTABLES -t mangle -I OUTPUT \
            ${proto:+-p $proto} \
            ${src_ip:+-s $src_ip} \
            ${src_port:+--sport $src_port} \
            ${src_mac:+-m mac --mac-source $src_mac} \
            ${dest_ip:+-d $dest_ip} \
            ${dest_port:+--dport $dest_port} \
            -j DSCP --set-dscp "$dscp" 
    }
    [ "$proto" == "tcpudp" -o -z "$proto" ] && {
        proto=tcp
        add_rule
        proto=udp
        add_rule
        return
    }
    add_rule   
}

QosInterface() {
    local iface
    local enabled
    local bandwidth
    local target
    local prio
    local cnt=0

    config_get iface $1 iface
    config_get enabled $1 enabled
    config_get bandwidth $1 bandwidth
    config_get target $1 target
    config_get prio $1 prio

    [ "$enabled" = "0" ] && return
    LOCAL_QOS="1"
    [ "$iface" = "br-lan" ] && LAN_QOS="1"
    [ "$iface" = "br-wan" ] && WAN_QOS="1"       
    IFACE=""$IFACE" "$iface""    
    
    [ "$TC" = "echo tc" ] && echo IFACE:"$IFACE" prio:"$prio"
    $TC qdisc add dev $iface root handle 1: htb default 1"$prio"
}

QosClass() {
    local iface
    local target
    local rate
    local ceil
    local burs
    local prio

    config_get iface $1 iface
    config_get target $1 target
    config_get rate $1 rate
    config_get ceil $1 ceil
    config_get burs $1 burs
    config_get prio $1 prio

    [ "$TC" = "echo tc" ] && echo QosClass $iface $target $prio

    for i in $IFACE; do
        [ "$i" = "$iface" ] && 
        $TC class add dev $iface parent 1:0 classid 1:1"$prio" htb rate $rate ceil $ceil prio $prio burst $burs                                     
    done    
}

QosClassify() {
    local iface
    local target
    local src_ip
    local src_port
    local dest_ip
    local dest_port
    local proto
    local prio
    local mark

    config_get iface $1 iface
    config_get target $1 target
    config_get src_ip $1 src_ip
    config_get src_port $1 src_port
    config_get dest_ip $1 dest_ip
    config_get proto $1 proto
    config_get dest_port $1 dest_port
    config_get prio $1 prio
    mark=1"$prio"

    [ "$TC" = "echo tc" ] && echo QosClassify $iface $mark

    [ "$proto" == "tcpudp" -o -z "$proto" ] && {
        proto=tcp
        add_qoscrule PREROUTING
        proto=udp
       add_qoscrule PREROUTING
        return
    }
    add_qoscrule PREROUTING

    for i in $IFACE; do
        [ "$i" = "$iface" ] && 
        $TC filter add dev $iface parent 1:0 protocol ip handle $mark fw classid 1:"$mark"                                     
    done
}

Exec_Global(){
    iptables -t mangle -I PREROUTING -p tcp --tcp-flags SYN,ACK,FIN,RST RST -j MARK --set-mark 11
    iptables -t mangle -I PREROUTING -p icmp -j MARK --set-mark 11
    iptables -t mangle -I POSTROUTING -p tcp --tcp-flags SYN,ACK,FIN,RST RST -j MARK --set-mark 11
    iptables -t mangle -I POSTROUTING -p icmp -j MARK --set-mark 11
    iptables -t mangle -I FORWARD -p tcp --tcp-flags SYN,ACK,FIN,RST RST -j MARK --set-mark 11
    iptables -t mangle -I FORWARD -p icmp -j MARK --set-mark 11
    iptables -t mangle -I OUTPUT -p tcp --tcp-flags SYN,ACK,FIN,RST RST -j MARK --set-mark 11
    iptables -t mangle -I OUTPUT -p icmp -j MARK --set-mark 11
    for i in $IFACE; do
        $TC filter add dev $i parent 1:0 protocol ip u32 match ip tos 0x86 0xff at 1 flowid 1:11
        $TC filter add dev $i parent 1:0 protocol ip u32 match ip tos 0x85 0xff at 1 flowid 1:12  
        $TC filter add dev $i parent 1:0 protocol ip u32 match ip tos 0x84 0xff at 1 flowid 1:13  
        $TC filter add dev $i parent 1:0 protocol ip u32 match ip tos 0x83 0xff at 1 flowid 1:14
        $TC filter add dev $i parent 1:0 protocol ip u32 match ip tos 0x82 0xff at 1 flowid 1:15  
        $TC filter add dev $i parent 1:0 protocol ip u32 match ip tos 0x81 0xff at 1 flowid 1:16  
        $TC filter add dev $i parent 1:0 protocol ip handle 11 fw classid 1:11
        $TC filter add dev $i parent 1:0 protocol ip handle 12 fw classid 1:12
        $TC filter add dev $i parent 1:0 protocol ip handle 13 fw classid 1:13
        $TC filter add dev $i parent 1:0 protocol ip handle 14 fw classid 1:14
        $TC filter add dev $i parent 1:0 protocol ip handle 15 fw classid 1:15
        $TC filter add dev $i parent 1:0 protocol ip handle 16 fw classid 1:16
        $TC qdisc add dev $i parent 1:11 handle 11: sfq perturb 5
        $TC qdisc add dev $i parent 1:12 handle 12: sfq perturb 5
        $TC qdisc add dev $i parent 1:13 handle 13: sfq perturb 5
        $TC qdisc add dev $i parent 1:14 handle 14: sfq perturb 5
        $TC qdisc add dev $i parent 1:15 handle 15: sfq perturb 5
        $TC qdisc add dev $i parent 1:16 handle 16: sfq perturb 5
    done         
}

admintool() {
    local iface
    local target
    local src_ip
    local src_port
    local dest_port
    local proto="tcp"
    local prio="1"
    local mark="11"
    local http
    local ssl_port
    local lan_ip
    local wan_ip

    http=`uci get httpd.@httpd[0].port`
    config_clear
    config_load webif
    config_get ssl_port general ssl_port

    [ "$LAN_QOS" = "1" ] && 
    {
        $TC filter add dev br-lan parent 1:0 protocol ip prio 1 handle 11 fw classid 1:11
        src_ip="$(ifconfig br-lan | grep inet | grep -v inet6 | awk -F'[: ]+' '{print $4}')"
        src_port="$http"
        [ -n "$src_port" ] && add_qoscrule OUTPUT
        src_port="$ssl_port"
        [ -n "$src_port" ] && add_qoscrule OUTPUT
        src_port=21
        [ -n "$src_port" ] && add_qoscrule OUTPUT
        src_port=22
        [ -n "$src_port" ] && add_qoscrule OUTPUT
        src_port=23
        [ -n "$src_port" ] && add_qoscrule OUTPUT
    }

    [ "$WAN_QOS" = "1" ] && 
    {
        $TC filter add dev br-wan parent 1:0 protocol ip prio 1 handle 11 fw classid 1:11
        src_ip="$(ifconfig br-wan | grep inet | grep -v inet6 | awk -F'[: ]+' '{print $4}')"
        src_port="$http"
        [ -n "$src_port" ] && add_qoscrule OUTPUT
        src_port="$ssl_port"
        [ -n "$src_port" ] && add_qoscrule OUTPUT
        src_port=21
        [ -n "$src_port" ] && add_qoscrule OUTPUT
        src_port=22
        [ -n "$src_port" ] && add_qoscrule OUTPUT
        src_port=23
        [ -n "$src_port" ] && add_qoscrule OUTPUT
    }   
}

case "$1" in
    start)
        iptables -t mangle -F
        iptables -t mangle -X
        tc qdisc del dev br-lan root 1>/dev/null 2>/dev/null
        tc qdisc del dev br-wan root 1>/dev/null 2>/dev/null    
        echo "Checking general"
        config_foreach QosGeneral general
        [ "$GLOABLE_QOS" = "0" ] && return 0        
        echo "Loading QosInterface"
        config_foreach QosInterface interface
        echo "Loading QosGloble"
        config_foreach QosGloble global               
        echo "Loading QosClass"
        config_foreach QosClass class
        Exec_Global        
        [ "$LOCAL_QOS" = "0" ] && return 0      
        echo "Loading QosClassify"
        config_foreach QosClassify classify              
        echo "Loading http&https"
        admintool
        $IPTABLES -t mangle -A PREROUTING -j CONNMARK --save-mark
        $IPTABLES -t mangle -A POSTROUTING -j CONNMARK --save-mark     
        $IPTABLES -t mangle -A FORWARD -j CONNMARK --save-mark 
        $IPTABLES -t mangle -A OUTPUT -j CONNMARK --save-mark     
    ;;
    stop)
        iptables -t mangle -F
        iptables -t mangle -X
        tc qdisc del dev br-lan root 1>/dev/null 2>/dev/null
        tc qdisc del dev br-wan root 1>/dev/null 2>/dev/null 
    ;;
esac
