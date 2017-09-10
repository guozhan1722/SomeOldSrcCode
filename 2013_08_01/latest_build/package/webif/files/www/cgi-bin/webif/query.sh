#!/usr/bin/webif-page

<?
. /usr/lib/webif/webif.sh


get_carrier() 
{

    [ -f "/var/run/VIP4G_status" ] && {
        . /var/run/VIP4G_status
    }

    DESC_FILE="/usr/lib/hardware_desc"
    [ -f $DESC_FILE ] && . $DESC_FILE

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
            if [ "$ippassthrough" = "Ethernet" ];then
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

    FORM_tech_avail=$tech_avail
    FORM_tech_avail=$(echo $FORM_tech_avail|sed 's/^[[:digit:]], //g')
    if [ "$FORM_have_simcard" = "0" ];then
        FORM_tech_avail="Not Available"
    fi
    [ -z "$FORM_tech_avail" ] && FORM_tech_avail="Unknown"

    FORM_tech_supp=$tech_supp
    FORM_tech_supp=$(echo $FORM_tech_supp|sed 's/^[[:digit:]], //g')
    [ -z "$FORM_tech_supp" ] && FORM_tech_supp="Unknown"

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
Content-Type: text/plain; charset=UTF-8

{"statusreport":{"Type":"carrier","ActiveStatus":"$FORM_connect_status","RSSI":"$FORM_rssi","IP":"$wan_ip","Network":"$FORM_network","Servicetype":"$state","Roaming":"$FORM_roaming","Temperature":"$FORM_temp","IMEI":"$FORM_imei","ICCID":"$FORM_simid","Phone":"$FORM_phonenum"}}

EOF
}

query_type() 
{
    if [ "$FORM_statusreportrequest" = "1" ]; then
        get_carrier
#    elif [ "$FORM_statusreportrequest" = "2" ]; then
    else
cat <<EOF
Content-Type: text/plain; charset=UTF-8

Error

EOF
    fi
}

query_type ?>
 

