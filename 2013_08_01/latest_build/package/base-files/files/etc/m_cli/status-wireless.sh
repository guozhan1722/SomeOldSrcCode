#!/bin/sh
. /usr/lib/webif/webif.sh

config_cb() {
	cfg_type="$1"
	cfg_name="$2"
}

parse_target() {

   equal "$1" "" && is_empty="1"

   echo "$1" | awk -v "is_empty=$is_empty" -v "heading=$2" '
   BEGIN {
      rulecntr=0
		odd=1

      if (is_empty == "1") {
         print "None"
      } else {
#	 print "      <td width=\"20%\">@TR<<status_wlan_MAC#MAC Address>></td>"
#        print "      <td width=\"10%\">@TR<<status_wlan_Signal#RSSI>></td>"
#        print "      <td width=\"10%\">@TR<<status_wlan_Rate#Rate>></td>"
      }

   }

   function blankline()
   {
      print " "
   }

   /^(#.*)?$/ {next}
   $1 == "Station" {
      if (odd == 1) {
         odd--
      } else {
         odd++
      }

      print "status_wlan_MAC : " $2
   }

   $1 == "signal:" {
      print "status_wlan_Signal: " $2" "$3
   }

   $1 == "tx" && $2 == "bitrate:" {
      print "status_wlan_Rate : " $3" "$4
      print " "
   rulecntr++
   }

   END {
      blankline()
   }'
}

displaywiface() {
	local wifpar="$1"
	local cfgsec="$3"
	local fconfig mac_addr wconfig wlan_ssid wlan_mode wlan_freq wlan_txpwr 
	local wlan_secmode
	local on_phy_num
	local iw
	if [ -n "$wifpar" ]; then
		local wnum="$2"
		wnum="${wnum:-0}"
		wconfig=$(iwconfig "$wifpar" 2>/dev/null)
		fconfig=$(ifconfig "$wifpar" 2>/dev/null)
		iw=$(iw dev "$wifpar" station dump 2>/dev/null)
		[ -n "$wconfig" ] && {
			wlan_ssid=$(echo "$wconfig" | grep "ESSID:" | cut -d'"' -f 2 | cut -d'"' -f 1)
			if empty "$wlan_ssid" ; then
			   eval "cfg_ssid=\"\$CONFIG_${cfgsec}_ssid\""
			   wlan_ssid="$cfg_ssid"
			fi

			wlan_mode=$(echo "$wconfig" | grep "Mode:" | cut -d':' -f 2 | cut -d' ' -f 1)
			case "$wlan_mode" in
   			   Master)
   			      wlan_mode="Access Point"
			      ;;
   			   Managed)
   			      wlan_mode="Station"
			      ;;
   			   Auto)
   			      wlan_mode="Mesh Point"
			      ;;
   			   *)
   			      wlan_mode="Unknow"
			      ;;
			esac
			wlan_freq=$(echo "$wconfig" | grep "Frequency:" | cut -d':' -f 3 | cut -d' ' -f 1)
			wlan_freq="${wlan_freq:-0}"
			mac_addr=$(echo "$fconfig" | sed '/HWaddr /!d; s/^.*HWaddr //; s/[[:space:]]//')
			wlan_txpwr=$(echo "$wconfig" | sed '/Tx-Power=/!d; s/^.*Tx-Power=//; s/[[:space:]].*$//')
			wlan_txpwr="${wlan_txpwr:-0}"
			eval "wlan_secmode=\"\$CONFIG_${cfgsec}_encryption\"" 
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

			if [ "$wnum" = "0" ]; then 
			   wnum=""
			   on_phy_num=""
			else
			   eval "wdevice=\"\$CONFIG_${cfgsec}_device\""
			   on_phy_num=" on $wdevice"
			fi

                        echo ""
                        echo -e "WLAN \t$wnum \t$on_phy_num"
                        echo -e "\tMAC Addr      : \t$mac_addr"
                        echo -e "\tMode          : \t$wlan_mode"
                        echo -e "\tESSID         : \t$wlan_ssid"
                        echo -e "\tSecurity mode : \t$wlan_secmode"
                        echo ""
                        echo -e "Connection"
                        parse_target "$iw"

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
		case "$manuf" in
			atheros)
				ath_cnt=$(( $ath_cnt + 1 ))
				cur_iface=$(printf "ath%d" "$(( $ath_cnt - 1))")
			;;
			mac80211)
				wlan_cnt=$(( $wlan_cnt + 1 ))
				cur_iface=$(printf "wlan%d" "$(( $wlan_cnt - 1))") 
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

		displaywiface $cur_iface $cntr $cfgsec
		cntr=$(( $cntr +1 ))
		frm_wifaces="$frm_wifaces $cur_iface"
	}
done


