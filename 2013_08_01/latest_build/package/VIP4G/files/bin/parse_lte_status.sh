#!/bin/sh
. /etc/functions.sh
DEBUG_LOG="logger"

DESC_FILE="/usr/lib/hardware_desc"
ORI_STATUS_FILE="/var/run/VIP4G_status"
STD_STATUS_FILE="/var/run/std_lte_statue"

[ -f $DESC_FILE ] && . $DESC_FILE
[ -f $ORI_STATUS_FILE ] && . $ORI_STATUS_FILE

carr_iface="eth2"

Std_file_refresh()
{
    keyword=$(cat  $ORI_STATUS_FILE|awk -F "=" '{print $1}')
    rm -rf $STD_STATUS_FILE
    echo "modem_type=$modem_type" >$STD_STATUS_FILE

    for kword in $keyword; do
        eval itemvalue="\$FORM_$kword"
        echo "$kword=$itemvalue" >>$STD_STATUS_FILE
    done
    echo "cellid=$DFORM_cellid">>$STD_STATUS_FILE
    echo "lac=$DFORM_lac">>$STD_STATUS_FILE
    echo "ser_mode=$ser_mode">>$STD_STATUS_FILE
    echo "domain=$domain">>$STD_STATUS_FILE
    echo "state=$state">>$STD_STATUS_FILE
}

init_val()
{
    keyword=$(cat  $ORI_STATUS_FILE|awk -F "=" '{print $1}')

    for item in $keyword; do
        itemname=FORM_$item
        itemvalue=$(cat $ORI_STATUS_FILE | grep "${item}=" |awk -F "=" {'print $2'})
        eval $itemname=$itemvalue
    done
}

init_val
RTT=$(echo $FORM_tech |grep "1xRTT")
EvDO=$(echo $FORM_tech |grep "EvDO")

FORM_temp=$(echo $FORM_temp |cut -d " " -f1|sed 's/ //g')
FORM_imei=$(echo $FORM_imei|sed 's/ //g')
FORM_simpin=$(echo $FORM_simpin|sed 's/ //g')
FORM_pinr=$(echo $FORM_pinr |cut -d "," -f2)
FORM_tech=$(echo $FORM_tech|sed 's/^[[:digit:]], //g')
FORM_rssi=$(echo $FORM_rssi |awk -F ":" '{print $2+0}')
FORM_rsrp=$(echo $FORM_rsrp |cut -d ":" -f2|sed 's/ //g')
FORM_rsrq=$(echo $FORM_rsrq |cut -d ":" -f2|sed 's/ //g')

#prevent rssi show 0 when no sim card 
[ "$FORM_rssi" = "0" ] && {
    FORM_rssi="N/A"
} || {
    FORM_rssi="-$FORM_rssi"
}

[ "$simid" = "No SIM Card" -o "$simid" = "SIM Card Locked" ] && {
    FORM_connect_duration="0"
} || {
    FORM_connect_duration=$(echo $FORM_connect_duration |cut -d " " -f1)
}
FORM_tech_avail=$(echo $FORM_tech_avail|sed 's/^[[:digit:]], //g')
FORM_tech_supp=$(echo $FORM_tech_supp|sed 's/^[[:digit:]], //g')

#if simid not a number  will shown it N/A
[ "$simid" = "No SIM Card" -o "$simid" = "SIM Card Locked" ] && {
    FORM_simid="N/A"
} || {
    FORM_simid=$(echo $simid|sed 's/ //g')
}

if [ "$modem_type" = "E362" ];then
    FORM_cellid=$(echo $FORM_cell_id|cut -d "," -f1|sed 's/ //')
    DFORM_cellid=$FORM_cellid

    FORM_lac=$(echo $FORM_cell_id|cut -d "," -f2|sed 's/ //')
    DFORM_lac=$FORM_lac
else
    FORM_cellid=$(echo $FORM_cell_id|cut -d "," -f4|sed 's/ //')
    DFORM_cellid=$(printf "%d" "0x$FORM_cellid" 2>/dev/null )

    FORM_lac=$(echo $FORM_cell_id|cut -d "," -f3|sed 's/ //')
    DFORM_lac=$(printf "%d" "0x$FORM_lac" 2>/dev/null)
    #if cellid and lac is 0 will show them like N/A
    [ "$DFORM_cellid" = "0" ] && DFORM_cellid="N/A"
    [ "$DFORM_lac" = "0" ] && DFORM_lac="N/A"
fi

if [ "$FORM_tech" != "LTE" ];then
    FORM_rsrp="N/A"
    FORM_rsrq="N/A"
fi

#if phonenumber not a number  will shown it N/A
[ "$FORM_phonenum" = "No SIM Card" -o "$FORM_phonenum" = "SIM Card Locked" ] && {
    FORM_phonenum="N/A"
} || {
    FORM_phonenum=$(echo $FORM_phonenum|cut -d "," -f2|sed 's/ //g')
}
if [ "$modem_type" = "E362" ];then
    if [ -n "$RTT" -o -n "$EvDO" ];then
        FORM_phonenum="N/A"
    fi
fi

roing=$(echo $FORM_roaming |cut -d "," -f2)
if [ "$roing" = "0" ]; then
    FORM_roaming="Not Searching"
elif [ "$roing" = "1" ]; then
    FORM_roaming="Home"
elif [ "$roing" = "2" ]; then
    FORM_roaming="Searching"
elif [ "$roing" = "3" ]; then
    FORM_roaming="Denied"
elif [ "$roing" = "4" ]; then
    FORM_roaming="Unknown"
elif [ "$roing" = "5" ]; then
    FORM_roaming="Roaming"
fi

if [ "$modem_type" = "E371" ];then
    [ "$FORM_network" = "No SIM Card" -o "$FORM_network" = "SIM Card Locked" ] && {
        FORM_network="N/A"
    } || {
        FORM_network=$(echo $FORM_network |cut -d "," -f3| sed 's/'\"'/''/g' )
        [ -z $FORM_network ] && FORM_network="N/A" 
    }
fi

if [ "$modem_type" = "E362" ];then
    if [ -n "$RTT" -o -n "$EvDO" ];then
        FORM_imei=$MEID
        FORM_imei=$(echo $FORM_imei| sed 's/'0x'/''/g'|sed 's/ //g')
    fi
fi

if [ "$modem_type" = "E362" ];then
    if [ -n "$RTT" -o -n "$EvDO" ];then
        FORM_simid="N/A"
    fi
fi

service_mode=$(echo $FORM_rat|cut -d "," -f1)
if [ "$modem_type" = "E362" ];then
    if [ "$service_mode" = "0" ]; then
        ser_mode="Automatic"
    elif [ "$service_mode" = "1" ]; then
        ser_mode="GSM Only"
    elif [ "$service_mode" = "2" ]; then
        ser_mode="WCDMA Only"
    elif [ "$service_mode" = "3" ]; then
        ser_mode="LTE Only"
    elif [ "$service_mode" = "4" ]; then
        ser_mode="CDMA (1xRTT) Only"
    elif [ "$service_mode" = "5" ]; then
        ser_mode="EvDO Only"
    else
        ser_mode="Unknown"
    fi
else
    if [ "$service_mode" = "0" ]; then
        ser_mode="Automatic"
    elif [ "$service_mode" = "1" ]; then
        ser_mode="GSM Only"
    elif [ "$service_mode" = "2" ]; then
        ser_mode="WCDMA Only"
    elif [ "$service_mode" = "3" ]; then
        ser_mode="LTE Only"
    else
        ser_mode="Unknown"
    fi
fi
domain=$(echo $FORM_rat|cut -d "," -f2)
if [ "$domain" = "0" ]; then
    domain="CS Only (Circuit Switched)"
elif [ "$domain" = "1" ]; then
    domain="PS Only (Packet Switched)"
elif [ "$domain" = "2" ]; then
    domain="CS and PS"
else
    domain="Unknown"
fi

state=$(echo $FORM_rat|cut -d "," -f3)
if [ "$state" = "0" ]; then
    state="Searching for Service"
elif [ "$state" = "1" ]; then
    state="WCDMA CS (Circuit Switched)"
elif [ "$state" = "2" ]; then
    state="WCDMA PS (Packet Switched)"
elif [ "$state" = "3" ]; then
    state="WCDMA CS and PS"
elif [ "$state" = "4" ]; then
    state="GSM CS (Circuit Switched)"
elif [ "$state" = "5" ]; then
    state="GSM PS (Packet Switched)"
elif [ "$state" = "6" ]; then
    state="GSM CS and PS "
elif [ "$state" = "7" ]; then
    state="LTE CS (Circuit Switched)"
elif [ "$state" = "8" ]; then
    state="LTE PS (Packet Switched)"
elif [ "$state" = "9" ]; then
    state="LTE CS and PS "
elif [ "$state" = "10" ]; then
    state="CDMA 1xRTT CS (Circuit Switched)"
elif [ "$state" = "11" ]; then
    state="CDMA 1xRTT PS (Packet Switched)"
elif [ "$state" = "12" ]; then
    state="CDMA 1xRTT CS and PS "
elif [ "$state" = "13" ]; then
    state="EvDO CS (Circuit Switched)"
elif [ "$state" = "14" ]; then
    state="EvDO PS (Packet Switched)"
elif [ "$state" = "15" ]; then
    state="EvDO CS and PS "
else
    state="Unknown"
fi


if echo $FORM_connect_status |grep -q "_CONNECTED"; then
    FORM_connect_status="Connected"
else
    FORM_connect_status="Disconnected"
    #remove dns file when lte not connected
    mwan_enable=$(uci get multiwan.config.enabled)
    lte_disable=$(uci get lte.connect.disabled)
    if [ "$mwan_enable" = "0" -a "$lte_disable" = "0" ];then
         echo "" > "/tmp/resolv.conf.auto" 
    fi
fi

Std_file_refresh
