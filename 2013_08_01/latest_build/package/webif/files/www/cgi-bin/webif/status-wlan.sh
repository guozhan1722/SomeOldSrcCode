#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
. /etc/version

[ -n "$FORM_interval" ] || FORM_interval=20

! empty "$FORM_refreshstop" && {
	FORM_interval=0
}

[ "$FORM_interval" -gt 0 ] >/dev/null 2>&1 && {
	meta_refresh="<meta http-equiv=\"refresh\" content=\"$FORM_interval;url=$SCRIPT_NAME?interval=$FORM_interval\" />"
}

header_inject_head=$(cat <<EOF
$meta_refresh
EOF

)

header "Wireless" "Status" "@TR<<Wireless Interfaces>>"

config_cb() {
	cfg_type="$1"
	cfg_name="$2"
}

parse_target_IPnVT() {

   echo "$1" | awk -v "is_empty=$is_empty" -v "heading=$2" '
   BEGIN {
        rulecntr=0
	odd=1

    }
	/^(#.*)?$/ {next}
	$1 == "Station" {
		if (odd == 1) {
			print "	<tr class=\"odd\">"
			odd--
		} else {
			print "	<tr>"
			odd++
		}
		print "		<td width=\"20%\">" $2 "</td>"
	}
	$1 == "rx" && $2 == "packets:" {
                rxpackets=$3
                if ( rxpackets == 0) rxpackets=1;
	}

	$1 == "rx" && $2 == "dropped:" {
                rxfailed=$3
                rxccq = rxpackets  * 100 / (rxpackets + rxfailed);
	}

	$1 == "tx" && $2 == "packets:" {
                txpackets=$3
                if ( txpackets == 0) txpackets=1;
	}
	$1 == "tx" && $2 == "retries:" {
                txretries=$3
	}
	$1 == "tx" && $2 == "failed:" {
                txfailed=$3
                txccq = txpackets  * 100 / (txpackets + txretries + txfailed);
	}

	$1 == "signal:" {
                sig=$2
	}
	$1 == "noise" && $2 == "floor:" {
                noi=$3
                if (noi == 0) noi=-95;
		print "		<td width=\"20%\">"noi"</td>"
		print "		<td width=\"20%\">"sig-noi"</td>"
	}
	$1 == "rssi:" {
                rssi = $2
                if ( $2 == 0 ) rssi= sig+95;
                RSSIPER= rssi * 100 / 30;
 #               RSSIPER= 12 * 100 / 30;
                if (RSSIPER < 0) RSSIPER=0;
                if (RSSIPER > 100) RSSIPER=100;
                if(RSSIPER <=10){                                 
                    backgcolor="#FF0000";
                    midbackgcolor="#FF0000";
                    per=0.5;
                }else if (RSSIPER>10 && RSSIPER < 35){           
                    backgcolor="#FFFF00";
                    midbackgcolor="#FF0000";
                    per=0.5*10/RSSIPER;
                    ffper2=70-RSSIPER;
                    ffper1=350/RSSIPER;
                }else{                                                       
                    backgcolor="#00FF00";
                    midbackgcolor="#FFFF00";
                    per=0.1*100/RSSIPER;
                    ffper2=235-RSSIPER;
                    ffper1=1000/RSSIPER;
                }                             
		print "		<td width=\"20%\">"rssi-95"</td>"
	}
	$1 == "rx" && $2 == "bitrate:" {

                printf("<td><div class=\"progressbar\" id=\"RSSI\" style=\"width:200px\">");
                printf("<span class=\"progress\" style=\"background-color:%s;background: -webkit-gradient(linear, left top, right top, from(#FF0000), to(%s), color-stop(%s, %s)); background: -moz-linear-gradient(left,#FF0000 0%%,%s %s%%,%s %s%%);filter: progid:DXImageTransform.Microsoft.gradient(GradientType=1,startColorstr=%s, endColorstr=%s);width:%d%%\">%d%%</span>",backgcolor,backgcolor,per,midbackgcolor,midbackgcolor,ffper1,backgcolor,ffper2,midbackgcolor,backgcolor,RSSIPER,RSSIPER);
              	printf("</div></td>");
                printf("</td>");

		print "	</tr>"
		rulecntr++
	}
	END {
	}'
}

parse_target() {

   echo "$1" | awk -v "is_empty=$is_empty" -v "heading=$2" '
   BEGIN {
        rulecntr=0
	odd=1

    }
	/^(#.*)?$/ {next}
	$1 == "Station" {
		if (odd == 1) {
			print "	<tr class=\"odd\">"
			odd--
		} else {
			print "	<tr>"
			odd++
		}
		print "		<td width=\"10%\">" $2 "</td>"
	}
	$1 == "rx" && $2 == "packets:" {
                rxpackets=$3
                if ( rxpackets == 0) rxpackets=1;
	}

	$1 == "rx" && $2 == "dropped:" {
                rxfailed=$3
                rxccq = rxpackets  * 100 / (rxpackets + rxfailed);
	}

	$1 == "tx" && $2 == "packets:" {
                txpackets=$3
                if ( txpackets == 0) txpackets=1;
	}
	$1 == "tx" && $2 == "retries:" {
                txretries=$3
	}
	$1 == "tx" && $2 == "failed:" {
                txfailed=$3
                txccq = txpackets  * 100 / (txpackets + txretries + txfailed);
	}

	$1 == "signal:" {
                sig=$2
	}
	$1 == "noise" && $2 == "floor:" {
                noi=$3
                if (noi == 0) noi=-95;
		print "		<td width=\"10%\">"noi"</td>"
		print "		<td width=\"6%\">"sig-noi"</td>"
	}
	$1 == "rssi:" {
                rssi = $2
                if ( $2 == 0 ) rssi= sig+95;
                RSSIPER= rssi * 100 / 30;
 #               RSSIPER= 12 * 100 / 30;
                if (RSSIPER < 0) RSSIPER=0;
                if (RSSIPER > 100) RSSIPER=100;
                if(RSSIPER <=10){                                 
                    backgcolor="#FF0000";
                    midbackgcolor="#FF0000";
                    per=0.5;
                }else if (RSSIPER>10 && RSSIPER < 35){           
                    backgcolor="#FFFF00";
                    midbackgcolor="#FF0000";
                    per=0.5*10/RSSIPER;
                    ffper2=70-RSSIPER;
                    ffper1=350/RSSIPER;
                }else{                                                       
                    backgcolor="#00FF00";
                    midbackgcolor="#FFFF00";
                    per=0.1*100/RSSIPER;
                    ffper2=235-RSSIPER;
                    ffper1=1000/RSSIPER;
                }                             
		print "		<td width=\"8%\">"rssi-95"</td>"
	}
	$1 == "tx" && $2 == "bitrate:" {
                printf(" <td width=\"8%%\">%d</td>",txccq);
                printf(" <td width=\"8%%\">%d</td>",rxccq);
		print "		<td width=\"10%\">" $3" "$4"</td>"
	}
	$1 == "rx" && $2 == "bitrate:" {
		print "		<td>" $3" "$4"</td>"

                printf("<td><div class=\"progressbar\" id=\"RSSI\" style=\"width:200px\">");
                printf("<span class=\"progress\" style=\"background-color:%s;background: -webkit-gradient(linear, left top, right top, from(#FF0000), to(%s), color-stop(%s, %s)); background: -moz-linear-gradient(left,#FF0000 0%%,%s %s%%,%s %s%%);filter: progid:DXImageTransform.Microsoft.gradient(GradientType=1,startColorstr=%s, endColorstr=%s);width:%d%%\">%d%%</span>",backgcolor,backgcolor,per,midbackgcolor,midbackgcolor,ffper1,backgcolor,ffper2,midbackgcolor,backgcolor,RSSIPER,RSSIPER);
              	printf("</div></td>");
                printf("</td>");

		print "	</tr>"
		rulecntr++
	}
	END {
	}'
}

displaywiface() {
	local wifpar="$1"
	local cfgsec="$3"
	local fconfig mac_addr wconfig wlan_ssid wlan_mode wlan_freq wlan_txpwr 
        local wlanid_caption="@TR<<SSID>>"
	local wlan_secmode
	local on_phy_num
	local iw
        local rx_pkts rx_bytes rx_errs rx_dropped rx_overrun rx_frame
        local tx_pkts tx_bytes tx_errs tx_dropped tx_overrun tx_carrier
	if [ -n "$wifpar" ]; then
		local wnum="$2"
		wnum="${wnum:-0}"
		wconfig=$(iwconfig "$wifpar" 2>/dev/null)
		fconfig=$(ifconfig "$wifpar" 2>/dev/null)
		iw=$(iw dev "$wifpar" station dump 2>/dev/null)
		[ -n "$wconfig" ] && {
			wlan_ssid=$(echo "$wconfig" | grep "ESSID:" | grep -v off | cut -d'"' -f 2 | cut -d'"' -f 1)
                        #wlan_mode=$(echo "$wconfig" | grep "Mode:" | cut -d':' -f 2 | cut -d' ' -f 1)
                        eval "wlan_mode=\"\$CONFIG_${cfgsec}_mode\""
                        eval "wlan_ifname=\"\$CONFIG_${cfgsec}_ifname\""

                        rssi=$(echo "$iw"|grep "signal:"|awk '{print $2}')
                        [ empty $rssi ] && rssi_percent=0  || {
                           signal=$(expr $rssi + 110)
                           rssi_percent=$(expr $signal "*" 100 / 29 )
                           if [ "$rssi_percent" -gt "100" ]; then
                              rssi_percent=100
                           elif [ "$rssi_percent" -lt "0" ]; then
                              rssi_percent=0
                           fi
                        } 
                         
                        #if [ "$wlan_mode" = "Auto" ]; then
                        if [ "$wlan_mode" = "mesh" ]; then
                              wlanid_caption="@TR<<Mesh ID>>"
                        fi

                        [ "$PRODUCTNAME" = "IPnDDL" ] && wlanid_caption="@TR<<Network ID>>"

			if empty "$wlan_ssid" ; then
                           if [ "$wlan_mode" = "mesh" ]; then
			      eval "cfg_ssid=\"\$CONFIG_${cfgsec}_mesh_id\""
			      wlan_ssid="$cfg_ssid"
                           else
			      eval "cfg_ssid=\"\$CONFIG_${cfgsec}_ssid\""
			      wlan_ssid="$cfg_ssid"
                           fi
			fi
			
			case "$wlan_mode" in
   			   ap)
                                if [ "$PRODUCT" = "IPnDDL" ];then
                                    wlan_mode="Master"
                                else
       			            wlan_mode="Access Point"
                                fi
                                ;;
   			   sta)
                                if [ "$PRODUCT" = "IPnDDL" ];then
                                    wlan_mode="Slave"
                                else
                                    wlan_mode="Station"
                                fi
			      ;;
   			   mesh)
                                UB_CONFIG=$(grep params /proc/mtd | cut -d: -f1)
                                Testlab=$(grep "TESTLAB" /dev/$UB_CONFIG | cut -d= -f2)
                
                                if [ "$Testlab" = "1" ]; then 
                                    wlan_mode="Test Mode"
                                else
   			            wlan_mode="Mesh"
                                fi
			      ;;
   			   repeater)
   			      wlan_mode="Repeater"
			      ;;
   			   *)
   			      wlan_mode="Unknow"
			      ;;
			esac
			wlan_freq=$(echo "$wconfig" | grep "Frequency:" | cut -d':' -f 3 | cut -d' ' -f 1)
			wlan_freq="${wlan_freq:-N/A}"
			mac_addr=$(echo "$fconfig" | sed '/HWaddr /!d; s/^.*HWaddr //; s/[[:space:]]//')
			wlan_txpwr=$(echo "$wconfig" | sed '/Tx-Power=/!d; s/^.*Tx-Power=//; s/[[:space:]].*$//')
			wlan_txpwr="${wlan_txpwr:-0}"

                        rx_bytes=$(cat /proc/net/dev |grep "$wifpar" |grep -v mon |awk -F ':' '{print $2}' | awk '{rx_b+=$1 } END {print rx_b}')
                        rx_pkts=$(cat /proc/net/dev |grep "$wifpar" |grep -v mon |awk -F ':' '{print $2}' | awk '{rx_p+=$2 } END {print rx_p}')
       			#rx_pkts=$(echo "$fconfig" | grep "RX packets" | awk '{print $2}' | awk -F ":" '{print $2}')
       			#rx_bytes=$(echo "$fconfig" | grep "RX bytes" | awk '{print $2}' | awk -F ":" '{print $2}')
			rx_errs=$(echo "$fconfig" | grep "RX packets" | awk '{print $3}' | awk -F ":" '{print $2}')
			rx_dropped=$(echo "$fconfig" | grep "RX packets" | awk '{print $4}' | awk -F ":" '{print $2}')
			rx_overrun=$(echo "$fconfig" | grep "RX packets" | awk '{print $5}' | awk -F ":" '{print $2}')
			rx_frame=$(echo "$fconfig" | grep "RX packets" | awk '{print $6}' | awk -F ":" '{print $2}')
			#tx_pkts=$(echo "$fconfig" | grep "TX packets" | awk '{print $2}' | awk -F ":" '{print $2}')
       			#tx_bytes=$(echo "$fconfig" | grep "TX bytes" | awk '{print $6}' | awk -F ":" '{print $2}')
                        tx_bytes=$(cat /proc/net/dev |grep "$wifpar" |grep -v mon |awk -F ':' '{print $2}' |awk '{tx_b+=$9 } END {print tx_b}')
                        tx_pkts=$(cat /proc/net/dev |grep "$wifpar" |grep -v mon |awk -F ':' '{print $2}' |awk '{tx_p+=$10 } END {print tx_p}')
			tx_errs=$(echo "$fconfig" | grep "TX packets" | awk '{print $3}' | awk -F ":" '{print $2}')
			tx_dropped=$(echo "$fconfig" | grep "TX packets" | awk '{print $4}' | awk -F ":" '{print $2}')
			tx_overrun=$(echo "$fconfig" | grep "TX packets" | awk '{print $5}' | awk -F ":" '{print $2}')
			tx_carrier=$(echo "$fconfig" | grep "TX packets" | awk '{print $6}' | awk -F ":" '{print $2}')

                        if [ "$rx_bytes" -lt "1000" ]; then
                            rx_bytes=$(echo "$rx_bytes")B
                        elif [ "$rx_bytes" -ge "1000" -a "$rx_bytes" -lt "1000000"  ]; then
                            rx_bytes=$(echo "$rx_bytes" | awk '{printf("%s",$1/1000)}')KB
                        elif [ "$rx_bytes" -ge "1000000" -a "$rx_bytes" -lt "1000000000" ];then
                            rx_bytes=$(echo "$rx_bytes" |awk '{printf("%s",$1/1000000)}')MB
                        else
                            rx_bytes=$(echo "$rx_bytes" |awk '{printf("%s",$1/1000000000)}')GB
                        fi

                        if [ "$tx_bytes" -lt "1000" ]; then
                            tx_bytes=$(echo "$tx_bytes")B
                        elif [ "$tx_bytes" -ge "1000" -a "$tx_bytes" -lt "1000000"  ]; then
                            tx_bytes=$(echo "$tx_bytes" | awk '{printf("%s",$1/1000)}')KB
                        elif [ "$tx_bytes" -ge "1000000" -a "$tx_bytes" -lt "1000000000" ];then
                            tx_bytes=$(echo "$tx_bytes" |awk '{printf("%s",$1/1000000)}')MB
                        else
                            tx_bytes=$(echo "$tx_bytes" |awk '{printf("%s",$1/1000000000)}')GB
                        fi

			eval "wlan_secmode=\"\$CONFIG_${cfgsec}_encryption\"" 

                        if [ "$PRODUCT" = "IPnDDL" ];then
        			case "$wlan_secmode" in
           			   none)
           			      wlan_secmode="Disabled"
        			      ;;
        			   *)
           			      wlan_secmode="Enabled"
        			      ;;
        			esac
                        else
        			case "$wlan_secmode" in
   			   none)
   			      wlan_secmode="None"
			      ;;
			   wep)
   			      wlan_secmode="WEP"
			      ;;
			   psk)
   			      wlan_secmode="WPA(PSK)"
			      ;;
			   psk2)
   			      wlan_secmode="WPA2(PSK)"
			      ;;
			   psk-mixe*|psk+psk2)
			      wlan_secmode="WPA+WPA2(PSK)"
			      ;;
			   wpa)
   			      wlan_secmode="WPA(RADIUS)"
			      ;;
			   wpa2)
           		      wlan_secmode="WPA2(RADIUS)"
        		      ;;
                           wpa+wpa2)
           		      wlan_secmode="WPA+WPA2(RADIUS)"
        	              ;;
			   *)
   			      wlan_secmode="Unknown"
			      ;;
			esac
                        fi

			#if [ "$wnum" = "0" ]; then 
			#  wnum=""
			#  on_phy_num=""
			#else
			   eval "wdevice=\"\$CONFIG_${cfgsec}_device\""
			   on_phy_num=" on $wdevice"
			#fi
                            radionum=$(echo $wdevice|cut -c 6)
                            vendor=$(cat /sys/class/ieee80211/phy${radionum}/device/subsystem_device|cut -c3-4) 
                            if [ "$vendor" = "20" ];then #Compex Card
                                freqband=$(cat /sys/class/ieee80211/phy${radionum}/device/subsystem_device|cut -c5-6)
                                if [ "$freqband" = "51" ];then 
                                    fb="2.4G Mode"
                                elif [ "$freqband" = "62" ];then
                                    fb="5.8G Mode"
                                elif [ "$freqband" = "91" ];then  #11ng
                                    fb="2.4G Mode"
                                elif [ "$freqband" = "96" ];then
				    #determine if supported dual-band 
				    iw phy |grep "MHz"|grep -q "* 24"
				    equal "$?" 0 && {
                                	fb="Dual-Band Mode"     #11abgn
				    }||{
                                	fb="5.8G Mode"          #11na
				    }
                                fi
                            else #Ubiquiti Card
                                freqband=$(cat /sys/class/ieee80211/phy${radionum}/device/subsystem_device|cut -c6)
                                if [ "$freqband" = "2" ];then
                                    fb="2.4G Mode"
                                elif [ "$freqband" = "4" ];then
                                    fb="4.9G Mode"
                                elif [ "$freqband" = "5" ];then
                                    fb="5.8G Mode"
                                elif [ "$PRODUCT" = "IPnVT" ]; then
                                    fb="2.4G Mode"
                                elif [ "$freqband" = "1" ];then
                                    fb="2.4G Mode"
                                else
                                    fb="Unknown Mode"
                                fi
                            fi
                            radionum=$(( $radionum + 1 ))
cat <<EOF
<div class="settings">
<table style="width: 95%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3><strong>Radio $radionum Status</strong></h3></th>
</table>

<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
        <th><h3>General Status</h3></th>
</tr>
EOF
if [ "$HARDWARE" = "v1.0.0" ] ;then
cat <<EOF
<tr>
	<td width="20%">MAC Address</td>
	<td width="20%">Mode</td>
	<td width="20%">$wlanid_caption</td>
	<td width="20%">Radio Frequency</td>
	<td>Security mode</td>
</tr>
<tr class="odd">
	<td width="20%">$mac_addr</td>
	<td width="20%">$wlan_mode</td>
	<td width="20%">$wlan_ssid</td>
	<td width="20%">$wlan_freq</td>
	<td>$wlan_secmode</th>
</tr>
</table>

EOF

else
cat <<EOF
<tr>
	<td width="16%">MAC Address</td>
	<td width="16%">Mode</td>
	<td width="16%">$wlanid_caption</td>
	<td width="16%">Frequency Band</td>
	<td width="16%">Radio Frequency</td>
	<td>Security mode</td>
</tr>
<tr class="odd">
	<td width="16%">$mac_addr</td>
	<td width="16%">$wlan_mode</td>
	<td width="16%">$wlan_ssid</td>
	<td width="16%">$fb</td>
	<td width="16%">$wlan_freq</td>
	<td>$wlan_secmode</th>
</tr>
</table>

EOF
fi
cat <<EOF

<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">

<tr>
        <th><h3><strong>Traffic Status</strong></h3></th>
</tr>

<tr>
	<td width="25%">Receive bytes</td>
	<td width="25%">Receive packets</td>
	<td width="25%">Transmit bytes</td>
	<td>Transmit packets</td>
</tr>

<tr class="odd">
	<td width="25%">$rx_bytes</th>
	<td width="25%">$rx_pkts</th>
	<td width="25%">$tx_bytes</th>
	<td>$tx_pkts</th>
</tr>
</table>


EOF
devidx=$wnum
[ "$wnum" = "" ] && wnum=0
netdevs=$(ls /sys/class/ieee80211/phy${wnum}/device/net)
haveheader=0
for dev in $netdevs; do
   # Only show up the current Virtual interface's or repeater dump information
   if [ "$dev" = "$wlan_ifname" ] || [ "$wlan_mode" = "Repeater" ]; then
        ap_iw=$(iw dev "$dev" station dump 2>/dev/null)
   else
        ap_iw=""
   fi

   [ "$ap_iw" != "" ] && {
      if [ "$PRODUCT" = "IPnDDL" ]; then
            if [ "$haveheader" = "0" ]; then
cat <<EOF
<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
        <th><h3>Connection Status</h3></th>
</tr>

<tr>
        <td width="20%">@TR<<status_wlan_MAC#MAC Address>></td>
        <td width="20%">@TR<<status_wlan_Noise#Noise Floor (dBm)>></td>
        <td width="20%">@TR<<status_wlan_SNR#SNR (dB)>></td>
        <td width="20%">@TR<<status_wlan_RSSI#RSSI (dBm)>></td>
        <td>@TR<<status_wlan_Prg#Signal Level>></td>
</tr>
EOF
                haveheader=1
            fi
            parse_target_IPnVT "${ap_iw}"

      else
            if [ "$haveheader" = "0" ]; then
cat <<EOF
<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
        <th><h3>Connection Status</h3></th>
</tr>

<tr>
        <td width="16%">@TR<<status_wlan_MAC#MAC Address>></td>
        <td width="10%">@TR<<status_wlan_Noise#Noise Floor (dBm)>></td>
        <td width="6%">@TR<<status_wlan_SNR#SNR (dB)>></td>
        <td width="8%">@TR<<status_wlan_RSSI#RSSI (dBm)>></td>
        <td width="8%">@TR<<status_wlan_TXCCQ#TX CCQ (%)>></td>
        <td width="8%">@TR<<status_wlan_RXCCQ#RX CCQ (%)>></td>
        <td width="10%">@TR<<status_wlan_RRate#TX Rate>></td>
        <td>@TR<<status_wlan_TRate#RX Rate>></td>
        <td>@TR<<status_wlan_Prg#Signal Level>></td>
</tr>
EOF
            haveheader=1
        fi
        parse_target "${ap_iw}"
        fi
    }
done

if [ "$haveheader" = "1" ]; then
cat <<EOF
</table>
EOF
fi
cat <<EOF
</div>

EOF
    }
	fi
}

config_load wireless
cntr=0
for cfgsec in $CONFIG_SECTIONS; do
	eval "cfgtype=\$CONFIG_${cfgsec}_TYPE"
	[ "$cfgtype" = "wifi-iface" ] && {
		eval "wdevice=\"\$CONFIG_${cfgsec}_device\""
		eval "manuf=\"\$CONFIG_${wdevice}_type\""
                wlancount=$(echo $wdevice |cut -c 6)            
		case "$manuf" in
			atheros)
				ath_cnt=$(( $ath_cnt + 1 ))
				cur_iface=$(printf "ath%d" "$(( $ath_cnt - 1))")
			;;
			mac80211)
				#wlan_cnt=$(( $wlan_cnt + 1 ))
				#cur_iface=$(printf "wlan%d" "$(( $wlan_cnt - 1))")
                                cur_iface="wlan$wlancount" 
			;;
			*)
				eval "wdcnt=\$${wdevice}_cnt"
				wdcnt=$(( $wdcnt + 1 ))
				eval "${wdevice}_cnt=$wdcnt"
				if [ "$wdcnt" -gt 1 ]; then
					cur_iface=$(printf "$wdevice.%d" "$(( $wdcnt - 1))")
				else
					cur_iface="$wdevice"
				fi
			;;
		esac
		eval "cfgnet=\"\$CONFIG_${cfgsec}_network\""
		eval "isbridge=\"\$${cfgnet}_bridgen\""
		if [ "$isbridge" != "1" ]; then
			eval "${cfgnet}_ifacen=\"${cur_iface}\""
		fi
                
		displaywiface $cur_iface $wlancount $cfgsec
		cntr=$(( $cntr +1 ))
	}
done

echo "<div class=\"settings\" style=\"float: right;\">"
echo "<form name=\"refresh\" method=\"post\" action=\"$SCRIPT_NAME\">"

[ "$FORM_interval" -gt 0 ] >/dev/null 2>&1 && {
	cat <<EOF
<input  type="submit" value=" @TR<<system_info_Stop_Refreshing#Stop Refreshing>> " name="refreshstop" />
@TR<<system_info_Interval#Interval>>: $FORM_interval(@TR<<system_info_in_seconds#s>>)
EOF
} || {
	cat <<EOF
<input  type="submit" value=" @TR<<system_info_Auto_Refresh#Auto Refresh>> " name="refreshstart" />
@TR<<system_info_Interval#Interval>>:

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
(@TR<<system_info_in_seconds#s>>)
EOF
}
echo "</form></div>"

footer_nosubmit ?>
<!--
##WEBIF:name:Wireless:150:Status
-->
