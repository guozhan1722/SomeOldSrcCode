#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
#. /var/run/VIP4G_status
DESC_FILE="/usr/lib/hardware_desc"

[ -f $DESC_FILE ] && . $DESC_FILE

FORM_connect_status="Disconnected"
FORM_network="Unknown"
FORM_pinr="Unknown, Unknown"
FORM_rat="3,3,7"
FORM_sertype="Unknown"
FORM_rssi="Unknow:0"
FORM_rsrp="Unknow:0"
FORM_rsrq="Unknow:0"
FORM_cell_id="Unknown,Unknown"
FORM_tech="Unknown"
FORM_tech_avail="Unknown"
FORM_connect_duration="0"

serpid=$(ps aux|grep mpci-4g_server|grep -v grep|awk '{print $1}')
ser362pid=$(ps aux|grep E362_server|grep -v grep|awk '{print $1}')
server_stopped=0

if [ -z "$serpid" -a -z "$ser362pid" ]; then
    server_stopped=1
#    rm /var/run/VIP4G_status
#    /etc/init.d/lte stop
#    /etc/init.d/lte start
fi

[ -f "/var/run/VIP4G_status" ] && {
    . /var/run/VIP4G_status
}

[ -n "$FORM_interval" ] || FORM_interval=20

! empty "$FORM_refreshstop" && {
	FORM_interval=0
}

[ "$FORM_interval" -gt 0 ] >/dev/null 2>&1 && {
	meta_refresh="<meta http-equiv=\"refresh\" content=\"$FORM_interval;url=$SCRIPT_NAME?interval=$FORM_interval\" />"
}

carr_iface="eth2"    

#serpid=$(ps aux|grep mpci-4g_server|grep -v grep|awk '{print $1}')

#if [ -z "$serpid" ]; then
#    killall -9 mpci-4g
#    /sbin/mpci-4g -u >/dev/null 2>&1
#else
#    kill -s 17 $serpid
#fi

#/sbin/mpci-4g -u >/dev/null 2>&1

get_human_time()
{
    local duration_second="$1"
    FORM_connect_duration=""
    duration_day=$(echo $duration_second |awk '{printf "%i",$1/(60*60*24)}')

    if [ "$duration_day" -ge "1" ];then
        FORM_connect_duration="$duration_day day(s)"
        duration_second=$(echo $duration_second $duration_day  |awk '{printf "%i",$1-($2*60*60*24)}')
    fi

    duration_hour=$(echo $duration_second |awk '{printf "%i",$1/(60*60)}')
    if [ "$duration_hour" -ge "1" ];then
        FORM_connect_duration="$FORM_connect_duration $duration_hour hour"
        duration_second=$(echo $duration_second $duration_hour|awk '{printf "%i",$1-($2*60*60)}')
    fi

    duration_min=$(echo $duration_second |awk '{printf "%i", $1/60}')
    if [ "$duration_min" -ge "1" ];then
        FORM_connect_duration="$FORM_connect_duration $duration_min min"
        duration_second=$(echo $duration_second $duration_min |awk '{printf "%i",$1-($2 * 60)}')
    fi

    FORM_connect_duration="$FORM_connect_duration $duration_second sec"
}

get_traffic()
{
    rx_byte=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $1}')
    rx_pkts=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $2}')
    rx_errs=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $3}')
    rx_drop=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $4}')
    rx_fifo=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $5}')
    rx_fram=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $6}')
    rx_comp=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $7}')
    rx_mult=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $8}')

    tx_byte=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $9}')
    tx_pkts=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $10}')
    tx_errs=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $11}')
    tx_drop=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $12}')
    tx_fifo=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $13}')
    tx_coll=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $14}')
    tx_carr=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $15}')
    tx_comp=$(cat /proc/net/dev |grep "$1" |awk -F ':' '{print $2}' | awk '{print $16}')
    
    if [ -z "$rx_byte" ];then
        rx_byte="0"
    fi
    if [ -z "$rx_pkts" ];then
        rx_pkts="0"
    fi
    if [ -z "$rx_errs" ];then
        rx_errs="0"
    fi
    if [ -z "$rx_drop" ];then
        rx_drop="0"
    fi
    if [ -z "$rx_fifo" ];then
        rx_fifo="0"
    fi
    if [ -z "$rx_fram" ];then
        rx_fram="0"
    fi
    if [ -z "$rx_comp" ];then
        rx_comp="0"
    fi
    if [ -z "$rx_mult" ];then
        rx_mult="0"
    fi

    if [ -z "$tx_byte" ];then
        tx_byte="0"
    fi
    if [ -z "$tx_pkts" ];then
        tx_pkts="0"
    fi
    if [ -z "$tx_errs" ];then
        tx_errs="0"
    fi
    if [ -z "$tx_drop" ];then
        tx_drop="0"
    fi
    if [ -z "$tx_fifo" ];then
        tx_fifo="0"
    fi
    if [ -z "$tx_coll" ];then
        tx_coll="0"
    fi
    if [ -z "$tx_carr" ];then
        tx_carr="0"
    fi
    if [ -z "$tx_compl" ];then
        tx_comp="0"
    fi

    if [ "$rx_byte" -lt "1024" ]; then
        rx_byte=$(echo "$rx_byte")B
    elif [ "$rx_byte" -ge "1024" -a "$rx_byte" -lt "1048576"  ]; then
        rx_byte=$(echo "$rx_byte" | awk '{printf("%0.3f",$1/1024)}')KB
    elif [ "$rx_byte" -ge "1048576" -a "$rx_byte" -lt "1073741824" ];then
        rx_byte=$(echo "$rx_byte" |awk '{printf("%0.3f",$1/1024/1024)}')MB
    else
        rx_byte=$(echo "$rx_byte" |awk '{printf("%0.3f",$1/1024/1024/1024)}')GB
    fi
    
    if [ "$tx_byte" -lt "1024" ]; then
        tx_byte=$(echo "$tx_byte")B
    elif [ "$tx_byte" -ge "1024" -a "$tx_byte" -lt "1048576"  ]; then
        tx_byte=$(echo "$tx_byte" | awk '{printf("%0.3f",$1/1024)}')KB
    elif [ "$tx_byte" -ge "1048576" -a "$tx_byte" -lt "1073741824" ];then
        tx_byte=$(echo "$tx_byte" |awk '{printf("%0.3f",$1/1024/1024)}')MB
    else
        tx_byte=$(echo "$tx_byte" |awk '{printf("%0.3f",$1/1024/1024/1024)}')GB
    fi

}

header_inject_head=$(cat <<EOF
$meta_refresh
EOF

)

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	#case "$cfg_type" in
	#	general)
	#		append general_cfgs "$cfg_name" "$N"
	#	;;
	#esac
}

uci_load lte
conf="connect"

header "Carrier" "Status" "@TR<<Carrier Status>>" '' "$SCRIPT_NAME"

config_get FORM_disabled $conf disabled 

#FORM_have_simcard=0
#FORM_have_simcard=$have_simcard  
no_simcard=$(cat /sys/class/button/SIM_STAT/status)
if [ "$no_simcard" = "0" ];then
    FORM_have_simcard="1"
else
    FORM_have_simcard="0"
fi

[ "$FORM_disabled" = "1" -o "$server_stopped" = "1" ] && { 
        append connect_info "string|<tr><td colspan=\"2\"><h3 interface=\"warning\">Connect status is disabled or connect server is stopped </h3></td></tr>" "$N"
} || {

    config_get FORM_apn $conf apn "Unknown"
    newapn=`cat /var/run/autoapn | awk '{ print $1 }'`
    if [ "$newapn" ]; then
        FORM_apn=$newapn
    fi 

    FORM_temp=$temp
    FORM_temp=$(echo $FORM_temp |cut -d " " -f1) 
    [ -z "$FORM_temp" ] && FORM_temp="Unknown"
    FORM_imsi=$imsi
    [ -z "$FORM_imsi" ] && FORM_imsi="Unknown"
    FORM_mccmnc=$mccmnc

if [ "$modem_type" = "E362" ];then
cat <<EOF
<div class="settings">
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3>
<strong>Carrier Status</strong></h3></th>
</tr>

</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Current APN</strong></td>
	<td width=28%>$FORM_apn</td>
	<td width=22%><strong>IMSI</strong></td>
	<td width=28%>$FORM_imsi</td>
</tr>
</table>
EOF

else
cat <<EOF
<div class="settings">
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3>
<strong>Carrier Status</strong></h3></th>
</tr>

</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Current APN</strong></td>
	<td width=28%>$FORM_apn</td>
	<td width=22%><strong>Core Temperature(&deg;C)</strong></td>
	<td width=28%>$FORM_temp</td>
</tr>
</table>
EOF
fi
    wan_ip="Unknown"
    FORM_rssi=$rssi
    FORM_rssitype=$(echo $FORM_rssi |awk -F ":" '{print $1}' |grep "NO SERVICE RSSI")
    FORM_connect_status=$connect_status 
    if echo $FORM_connect_status |grep -q "_DISCONNECTED"; then
        FORM_connect_status="Disconnected"
    elif echo $FORM_connect_status |grep -q "_CONNECTED"; then
        local ippassthrough=$(uci get lte.connect.ippassthrough)

        if [ -n "$FORM_rssitype" ]; then
            FORM_connect_status="Disconnected"
            ifdown wan2
        else
            if [ "$ippassthrough" = "Ethernet" -o "$ippassthrough" = "WANport" ];then
                wan_ip=$(cat /var/run/ippth_4g)
                FORM_connect_status="Connected"
            else
                wan_ip="$(ifconfig br-wan2 | grep inet | grep -v inet6 | awk -F'[: ]+' '{print $4}')"
                FORM_connect_status="Connected"
            fi
        fi
    else
        FORM_connect_status="Unknown Connect Status"
    fi

    FORM_tech=$tech
    RTT=$(echo $FORM_tech |grep "1xRTT")
    EvDO=$(echo $FORM_tech |grep "EvDO")

    if [ "$modem_type" = "E362" ];then
        if [ -n "$RTT" -o -n "$EvDO" ];then
            C_IMEI="MEID"
            FORM_imei=$MEID
            FORM_imei=$(echo $FORM_imei| sed 's/'0x'/''/g'|sed 's/ //g')
            [ -z "$FORM_imei" ] &&  FORM_imei="Unknown"

        else
            C_IMEI="IMEI"
            FORM_imei=$(echo $imei|sed 's/ //g')
            [ -z "$FORM_imei" ] &&  FORM_imei="Unknown"
        fi
    else
        C_IMEI="IMEI"
        FORM_imei=$(echo $imei|sed 's/ //g')
        [ -z "$FORM_imei" ] &&  FORM_imei="Unknown"
    fi
    if [ "$FORM_have_simcard" = "0" ];then
        FORM_connect_status="No SIM Card"
    fi

cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Activity Status</strong></td>
	<td width=28%>$FORM_connect_status </td>
	<td width=22%><strong>$C_IMEI</strong></td>
	<td width=28%>$FORM_imei</td>
</tr>
</table>
EOF

    FORM_simpin=$simpin

    if [ "$FORM_connect_status" = "Connected" ] ;then
        FORM_network=$network
        if [ "$modem_type" = "E371" ];then
            FORM_network=$(echo $FORM_network |cut -d "," -f3| sed 's/'\"'/''/g' )
        fi
    else
        FORM_network="Unknown"
    fi

    if [ "$FORM_have_simcard" = "0" ];then
        FORM_simpin="Unknown"
        FORM_network="Unknown"
    fi
    [ -z "$FORM_simpin" ] && FORM_simpin="Unknown"
cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Network</strong></td>
	<td width=28%>$FORM_network</td>
	<td width=22%><strong>SIM PIN</strong></td>
	<td width=28%>$FORM_simpin</td>
</tr>
</table>
EOF
    FORM_roaming=$roaming
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

    FORM_simid=$(echo $simid|sed 's/ //g')

    if [ "$FORM_have_simcard" = "0" ];then
        FORM_simid="No SIM Card"
        FORM_roaming="Unknown"
    fi
    [ -z "$FORM_roaming" ] &&  FORM_roaming="Unknown"
    [ -z "$FORM_simid" ] && FORM_simid="Unknown"

    if [ "$modem_type" = "E362" ];then
        if [ -n "$RTT" -o -n "$EvDO" ];then
            FORM_simid="N/A"
        fi
    fi

cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Home/Roaming</strong></td>
	<td width=28%>$FORM_roaming</td>
	<td width=22%><strong>SIM Number (ICCID)</strong></td>
	<td width=28%>$FORM_simid</td>
</tr>
</table>
EOF
    FORM_pinr=$pinr
    FORM_pinr=$(echo $FORM_pinr |cut -d "," -f2)

    FORM_rat=$rat
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

    FORM_phonenum=$phonenum
    FORM_phonenum=$(echo $FORM_phonenum|cut -d "," -f2|sed 's/ //g')

    if [ "$FORM_have_simcard" = "0" ];then
        FORM_pinr="Unknown"
        FORM_phonenum="Unknown"
    fi

    [ -z "$FORM_phonenum" ] && FORM_phonenum="Unknown"

    if [ "$modem_type" = "E362" ];then
        if [ -n "$RTT" -o -n "$EvDO" ];then
            FORM_phonenum="N/A"
        fi
    fi

cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Service Mode</strong></td>
	<td width=28%>$ser_mode</td>
	<td width=22%><strong>Phone Number</strong></td>
	<td width=28%>$FORM_phonenum</td>
</tr>
</table>
EOF
#    FORM_gps=$gps
#    FORM_gps=$(echo $FORM_gps|sed 's/" "//g')
#    if [ "$FORM_gps" = "0" ]; then
#        FORM_gps="GPS Disabled"
#    elif [ "$FORM_gps" = "1" ]; then
#        FORM_gps="GPS Enabled"
#    else
#        FORM_gps="Unknown"
#    fi

    FORM_tech=$tech
    FORM_tech=$(echo $FORM_tech|sed 's/^[[:digit:]], //g')

    FORM_sertype=$sertype

    FORM_rssi=$(echo $FORM_rssi |awk -F ":" '{print $2+0}')
    if [ "$FORM_rssi" = "0" ];then
        FORM_rssi="0"
    else
        FORM_rssi="$FORM_rssi"
    fi
    FORM_rsrp=$rsrp
    FORM_rsrp=$(echo $FORM_rsrp |cut -d ":" -f2|sed 's/ //g')
    if [ "$FORM_rsrp" != " 0" ];then
        FORM_rsrp="$FORM_rsrp"
    fi
    FORM_rsrq=$rsrq
    FORM_rsrq=$(echo $FORM_rsrq |cut -d ":" -f2|sed 's/ //g')
    if [ "$FORM_rsrq" != " 0" ];then
        FORM_rsrq="$FORM_rsrq"
    fi

    if [ "$FORM_tech" != "LTE" ];then
        FORM_rsrp="N/A"
        FORM_rsrq="N/A"
    fi

    if [ "$FORM_have_simcard" = "0" ];then
        FORM_rssi="0"
        FORM_rsrp="0"
        FORM_rsrq="0"
    fi

Wimg=0
if [ "$FORM_connect_status" = "Connected" ] ;then
    if [ $FORM_rssi -gt 109 ]; then Wimg=1 ; 
    elif [ $FORM_rssi -gt 105 ]; then Wimg=2 ; 
    elif [ $FORM_rssi -gt 100 ]; then Wimg=3 ; 
    elif [ $FORM_rssi -gt 95 ]; then Wimg=4 ; 
    elif [ $FORM_rssi -gt 90 ]; then Wimg=5 ;  
    elif [ $FORM_rssi -gt 85 ]; then Wimg=6 ; 
    elif [ $FORM_rssi -eq 0 ]; then Wimg=0 ; 
    else Wimg=7 ; 
    fi
fi
#logger "rssi type is $FORM_rssitype"
if [ -n "$FORM_rssitype" ];then
    FORM_rssi="NO SERVICE"
else
    if [ "$FORM_rssi" = "0" ];then
        FORM_rssi="0"
    else
        FORM_rssi="-$FORM_rssi"
    fi
fi

cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Service State</strong></td>
	<td width=28%>$state</td>
	<td width=22%><strong>RSSI (dBm)</strong></td>
        <td width=28%>$FORM_rssi &nbsp; <img src="/images/cell$Wimg.gif" ALT='-" $FORM_rssi "dBm'/></td>
</tr>
</table>
EOF
    FORM_cell_id=$cell_id
    if [ "$modem_type" = "E362" ];then
        FORM_cellid=$(echo $FORM_cell_id|cut -d "," -f1|sed 's/ //')
        DFORM_cellid=$FORM_cellid
    
        FORM_lac=$(echo $FORM_cell_id|cut -d "," -f2|sed 's/ //')
        DFORM_lac=$FORM_lac
    else    
        FORM_cellid=$(echo $FORM_cell_id|cut -d "," -f4|sed 's/ //')
        DFORM_cellid=$(printf "%d" "0x$FORM_cellid")
    
        FORM_lac=$(echo $FORM_cell_id|cut -d "," -f3|sed 's/ //')
        DFORM_lac=$(printf "%d" "0x$FORM_lac")
    fi    
    [ -z "$FORM_cellid" ] && FORM_cellid="Unknown"
    [ -z "$FORM_lac" ] && FORM_lac="Unknown"
    [ -z "$FORM_rsrp" ] && FORM_rsrp="Unknown"
    [ -z "$FORM_rsrq" ] && FORM_rsrq="Unknown"

cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Cell ID</strong></td>
	<td width=28%>$DFORM_cellid</td>
	<td width=22%><strong>RSRP (dBm)</strong></td>
	<td width=28%>$FORM_rsrp</td>
</tr>
</table>
EOF
cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>LAC</strong></td>
	<td width=28%>$DFORM_lac</td>
	<td width=22%><strong>RSRQ (dBm)</strong></td>
	<td width=28%>$FORM_rsrq</td>
</tr>
</table>
EOF
    FORM_channelnum=$channelnum
    FORM_freqband=$freqband

    FORM_connect_duration=$connect_duration

    FORM_connect_duration=$(echo $FORM_connect_duration |cut -d " " -f1)

    if [ "$FORM_connect_status" = "Disconnected" -o "$FORM_have_simcard" = "0" ] ;then
        FORM_connect_duration="0"
    else
       get_human_time $FORM_connect_duration
    fi


    if [ "$FORM_have_simcard" = "0" ];then
        FORM_tech="Not Available"
    fi
    [ -z "$FORM_tech" ] && FORM_tech="Unknown"

cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Current Technology</strong></td>
	<td width=28%>$FORM_tech</td>
	<td width=22%><strong>Connection Duration</strong></td>
	<td width=28%>$FORM_connect_duration</td>
</tr>
</table>
EOF

    FORM_tech_avail=$tech_avail
    FORM_tech_avail=$(echo $FORM_tech_avail|sed 's/^[[:digit:]], //g')
    if [ "$FORM_have_simcard" = "0" ];then
        FORM_tech_avail="Not Available"
    fi
    [ -z "$FORM_tech_avail" ] && FORM_tech_avail="Unknown"

    FORM_tech_supp=$tech_supp
    FORM_tech_supp=$(echo $FORM_tech_supp|sed 's/^[[:digit:]], //g')
    [ -z "$FORM_tech_supp" ] && FORM_tech_supp="Unknown"
cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Available Technology</strong></td>
	<td width=28%>$FORM_tech_avail</td>
	<td width=22%><strong>WAN IP Address</strong></td>
	<td width=28%>$wan_ip</td>
</tr>
</table>
EOF
    dns=
    numdns=1
    ds=
    for ns in $(grep nameserver /tmp/resolv.conf.auto | awk '{print $2}'); do
        dns="${dns:+$dns <br/> }$ns"
        ds="DNS Server $numdns"
        DNSSERNAME="${DNSSERNAME:+$DNSSERNAME <br/>}$ds"
        let "numdns+=1"
    done
    [ -z "$dns" ] && dns="None"
    [ -z "$ds" ] && DNSSERNAME="DNS Server"

cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong> </strong></td>
	<td width=28%> </td>
	<td width=22%><strong>$DNSSERNAME</strong></td>
	<td width=28%>$dns</td>
</tr>
</table>
</div>
EOF
    
get_traffic $carr_iface

cat <<EOF
<div class="settings">
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3><strong>Recieved Packet Statistics</strong></h3></th>
<th><h3><strong>Transmitted Packet Statistics</strong></h3></th></tr>

</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Recieve bytes</strong></td>
	<td width=28%>$rx_byte</td>
	<td width=22%><strong>Transmit bytes</strong></td>
	<td width=28%>$tx_byte</td>
</tr>
</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Recieve packets</strong></td>
	<td width=28%>$rx_pkts</td>
	<td width=22%><strong>Transmit packets</strong></td>
	<td width=28%>$tx_pkts</td>
</tr>
</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Recieve errors</strong></td>
	<td width=28%>$rx_errs</td>
	<td width=22%><strong>Transmit errors</strong></td>
	<td width=28%>$tx_errs</td>
</tr>
</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%><strong>Drop packets</strong></td>
	<td width=28%>$rx_drop</td>
	<td width=22%><strong>Drop packets</strong></td>
	<td width=28%>$tx_drop</td>
</tr>
</table>

</div>
EOF


}

display_form <<EOF
$connect_info
EOF

echo "<div class=\"settings\" style=\"float: right;\">"
echo "<form name=\"refresh\" method=\"post\" action=\"$SCRIPT_NAME\">"

[ "$FORM_interval" -gt 0 ] >/dev/null 2>&1 && {
	cat <<EOF
<input type="submit" value=" @TR<<status_comports_Stop_Refreshing#Stop Refreshing>> " name="refreshstop" />
@TR<<status_comports_Interval#Interval>>: $FORM_interval (@TR<<status_comports_in_seconds#in seconds>>)
EOF
} || {
	cat <<EOF
<input type="submit" value=" @TR<<status_comports_Auto_Refresh#Auto Refresh>> " name="refreshstart" />
@TR<<status_comports_Interval#Interval>>:

<select name="interval">
EOF
	for sec in $(seq 3 59); do
		[ "$sec" -eq 20 ] && {
			echo "<option value=\"$sec\" selected=\"selected\">$sec</option>"
		} || {
			echo "<option value=\"$sec\">$sec</option>"
		}
	done
	cat <<EOF
</select>
@TR<<status_comports_in_seconds#in seconds>>
EOF
}
echo "</form></div>"

footer_nosubmit ?>
<!--
##WEBIF:name:Carrier:100:Status
-->
