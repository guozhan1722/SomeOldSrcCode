#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
. /etc/version

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
		print "		<td width=\"10%\">"sig-noi"</td>"
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
		print "		<td width=\"10%\">"rssi-95"</td>"
	}
	$1 == "tx" && $2 == "bitrate:" {
                printf(" <td width=\"10%%\">%d</td>",txccq);
                printf(" <td width=\"10%%\">%d</td>",rxccq);
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

                        [ "$PRODUCTNAME" = "IPnV" ] && wlanid_caption="@TR<<Network ID>>"

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
                                if [ "$PRODUCT" = "IPnVT" ];then
                                    wlan_mode="Master"
                                else
       			            wlan_mode="Access Point"
                                fi
                                ;;
   			   sta)
                                if [ "$PRODUCT" = "IPnVT" ];then
                                    wlan_mode="Slave"
                                else
                                    wlan_mode="Station"
                                fi
			      ;;
   			   mesh)
                                    wlan_mode="Mesh"
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

       			rx_pkts=$(echo "$fconfig" | grep "RX packets" | awk '{print $2}' | awk -F ":" '{print $2}')
       			rx_bytes=$(echo "$fconfig" | grep "RX bytes" | awk '{print $2}' | awk -F ":" '{print $2}')
			rx_errs=$(echo "$fconfig" | grep "RX packets" | awk '{print $3}' | awk -F ":" '{print $2}')
			rx_dropped=$(echo "$fconfig" | grep "RX packets" | awk '{print $4}' | awk -F ":" '{print $2}')
			rx_overrun=$(echo "$fconfig" | grep "RX packets" | awk '{print $5}' | awk -F ":" '{print $2}')
			rx_frame=$(echo "$fconfig" | grep "RX packets" | awk '{print $6}' | awk -F ":" '{print $2}')
			tx_pkts=$(echo "$fconfig" | grep "TX packets" | awk '{print $2}' | awk -F ":" '{print $2}')
       			tx_bytes=$(echo "$fconfig" | grep "TX bytes" | awk '{print $6}' | awk -F ":" '{print $2}')
			tx_errs=$(echo "$fconfig" | grep "TX packets" | awk '{print $3}' | awk -F ":" '{print $2}')
			tx_dropped=$(echo "$fconfig" | grep "TX packets" | awk '{print $4}' | awk -F ":" '{print $2}')
			tx_overrun=$(echo "$fconfig" | grep "TX packets" | awk '{print $5}' | awk -F ":" '{print $2}')
			tx_carrier=$(echo "$fconfig" | grep "TX packets" | awk '{print $6}' | awk -F ":" '{print $2}')

			eval "wlan_secmode=\"\$CONFIG_${cfgsec}_encryption\"" 

                        if [ "$PRODUCTNAME" = "IPnV" ] ;then
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
        			   wpa2|wpa+wpa2)
           			      wlan_secmode="WPA2(RADIUS)"
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
                            freqband=$(cat /sys/class/ieee80211/phy${radionum}/device/subsystem_device|cut -c6)
                            if [ "$freqband" = "2" ];then
                                fb="2.4G Mode"
                            elif [ "$freqband" = "4" ];then
                                fb="4.9G Mode"
                            elif [ "$freqband" = "5" ];then
                                fb="5.8G Mode"
                            else
                                fb="Unknown Mode"
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
if [ "$PRODUCT" = "IPnVT" ] ;then
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
	<td width="10%">MAC Address</td>
	<td width="10%">Mode</td>
	<td width="10%">$wlanid_caption</td>
	<td width="10%">Frequency Band</td>
	<td width="10%">Radio Frequency</td>
	<td>Security mode</td>
</tr>

<tr class="odd">
	<td width="10%">$mac_addr</td>
	<td width="10%">$wlan_mode</td>
	<td width="10%">$wlan_ssid</td>
	<td width="10%">$fb</td>
	<td width="10%">$wlan_freq</td>
	<td>$wlan_secmode</th>
</tr>
</table>

EOF
fi
cat <<EOF
<br/>
EOF
devidx=$wnum
[ "$wnum" = "" ] && wnum=0
netdevs=$(ls /sys/class/ieee80211/phy${wnum}/device/net)
haveheader=0
for dev in $netdevs; do
    ap_iw=$(iw dev "$dev" station dump 2>/dev/null)
    [ "$ap_iw" != "" ] && {
        if [ "$PRODUCT" = "IPnVT" ]; then
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
        <td width="10%">@TR<<status_wlan_MAC#MAC Address>></td>
        <td width="10%">@TR<<status_wlan_Noise#Noise Floor (dBm)>></td>
        <td width="10%">@TR<<status_wlan_SNR#SNR (dB)>></td>
        <td width="10%">@TR<<status_wlan_RSSI#RSSI (dBm)>></td>
        <td width="10%">@TR<<status_wlan_TXCCQ#TX CCQ (%)>></td>
        <td width="10%">@TR<<status_wlan_RXCCQ#RX CCQ (%)>></td>
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
<br/>

EOF
    }
	fi
}

if [ "$HARDWARE" = "5.0.0" ]; then
	productname="IPMESH"
elif [ "$HARDWARE" = "3.0.0" ]; then
	productname="SVIP"
elif [ "$HARDWARE" = "v1.0.0" ]; then
	productname="NANOPCI"
        if [ "$PRODUCT" = "IPnVT" ]; then
            productname="IPnVT"
        fi
else
	productname="VIP"
fi
[ "$PRODUCT" = "VIP421EXP49G" ] && PRODUCT="VIP421NOST"

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

#header "Info" "System" "<img src=\"/images/blkbox.jpg\" alt=\"@TR<<System Information>>\"/>@TR<<System Information>>"
header "System" "Info" "@TR<<System Information>>"

[ -z "$_device" ] && _device="unidentified"
_kversion=$(uname -srv 2>/dev/null)
_mac0=$(/sbin/ifconfig eth0 2>/dev/null | grep HWaddr | cut -b39-)
_mac1=$(/sbin/ifconfig eth1 2>/dev/null | grep HWaddr | cut -b39-)
netmask=$(/sbin/ifconfig br-lan 2>/dev/null | grep Mask | awk -F'[: ]+' '{print $8}')
dns=
for ns in $(grep nameserver /tmp/resolv.conf.auto | awk '{print $2}'); do
        dns="${dns:+$dns <br/> }$ns"
done
[ -z "$dns" ] && dns="None"

br_gw="$(route -n | grep '^0.0.0.0' | awk '{print $2}')"

_wlan0_mac=$(/sbin/ifconfig wlan0 2>/dev/null | grep HWaddr | cut -b39-)
_wlan1_mac=$(/sbin/ifconfig wlan1 2>/dev/null | grep HWaddr | cut -b39-)
board_type=$(cat /proc/cpuinfo 2>/dev/null | sed 2,20d | cut -c16-)
[ -z "$board_type" ] && board_type=$(uname -m 2>/dev/null)
user_string="$REMOTE_USER"
[ -z "$user_string" ] && user_string="not logged in"
machinfo=$(uname -a 2>/dev/null)

package_filename="webif_latest.ipk"
if $(echo "$_firmware_version" | grep -q "r[[:digit:]]*"); then
	svn_path="trunk"
else
	svn_path="tags/kamikaze_$_firmware_version"
fi
version_url=$(sed '/^src[[:space:]]*X-Wrt[[:space:]]*/!d; s/^src[[:space:]]*X-Wrt[[:space:]]*//g; s/\/packages.*$/\//g' /etc/opkg.conf 2>/dev/null)
revision_text=" $SOFTWARE build $BUILD"
this_revision="$_webif_rev"
version_file=".version"
upgrade_button=""
_hostname=$(cat /proc/sys/kernel/hostname)

NTPC=`which ntpclient`
NTP_RUNNING=`ps  | grep $NTPC | grep -v grep`
Built_date=$(cat /etc/banner|grep "Built time"|awk '{print $3}')
Built_time=$(cat /etc/banner|grep "Built time"|awk '{print $4}')
_date="`date +%F`"
_time="`date +%T`"

_uptime="$(uptime)"
_loadavg="${_uptime#*load average: }"
_uptime="${_uptime#*up }"
_uptime="${_uptime%%, load *}"

#[ -z "$NTP_RUNNING" ] && { 
#   Date="@TR<<Built Date>>"
#   Time="@TR<<Built Time>>"
#   _date="$Built_date"
#   _time="$Built_time"
#} || {
#   Date="@TR<<System Date>>"
#   Time="@TR<<System Time>>"
#   _date="`date +%F`"
#   _time="`date +%T`"
#} 

br_ip="$(ifconfig br-lan | grep inet | awk -F'[: ]+' '{print $4}')"

config_get_bool show_banner general show_banner 0
[ 1 -eq "$show_banner" ] && {
	echo "<pre>"
	cat /etc/banner 2>/dev/null
	echo "</pre><br />"
}
if [ "$PRODUCT" = "IPnVT" ]; then
cat <<EOF
<div class="settings">
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3><strong>System Information</strong></h3></th>
<th><h3><strong>Version Information</strong></h3></th></tr>
</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%>Host Name</td>
	<td width=28%>$_hostname</td>
	<td width=22%>Product Name</td>
	<td>$productname</td>
</tr>
</table>
EOF
_firmware_name="IPnVT 1.00"
else
cat <<EOF
<div class="settings">
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3><strong>System Information</strong></h3></th>
<th><h3><strong>Version Information</strong></h3></th></tr>
</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%>Host Name</td>
	<td width=28%>$_hostname</td>
	<td width=22%>Product Name</td>
	<td>$productname-$PRODUCT</td>
</tr>
</table>
EOF
fi
[ "$PRODUCT" = "VIP421ENCOM" ] || {
cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%>User Name</td>
	<td width=28%>$user_string</td>
	<td width=22%>Firmware Version</td>
	<td>$_firmware_name</td>
</tr>
</table>
EOF
}

cat<<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%>System date</td>
	<td width=28%>$_date</td>
	<td width=22%>Hardware Type</td>
	<td>$HARDWARE</td>
</tr>
</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%>System time</td>
	<td width=28%>$_time</td>
	<td width=22%>Build Version</td>
	<td>$revision_text</td>
</tr>
</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%>System uptime</td>
	<td width=28%>$_uptime</td>
	<td width=22%>Built date</td>
	<td>$Built_date</td>
</tr>
</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%>LAN IP Address</td>
	<td width=28%>$br_ip</td>
	<td width=22%>Built time</td>
	<td>$Built_time</td>
</tr>
</table>

EOF
cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%>Ethernet0 MAC Address</td>
	<td>$_mac0</td>
</tr>
</table>
EOF

empty $_mac1 || { 
cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%>Ethernet1 MAC Address</td>
	<td>$_mac1</td>

</tr>
</table>
EOF
}

cat <<EOF
<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%>Net Mask</td>
	<td>$netmask</td>
</tr>
</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%>DNS Server(s)</td>
	<td>$dns</td>
</tr>
</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=22%>Gateway</td>
	<td>$br_gw</td>
</tr>
</table>
</div>
<br/>
EOF

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

[ "$PRODUCT" = "VIP421ENCOM" ] || {
display_form <<EOF
start_form|@TR<<Copyright>>
field|@TR<<Copyright>>
string|$VENDOR
end_form|
EOF
}

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
echo "</form></div><br/>"

footer

?>
<!--
##WEBIF:name:System:001:Info
-->
