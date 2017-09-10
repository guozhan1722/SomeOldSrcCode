#!/bin/sh
# DEBUG="echo"
. /etc/version

append DRIVERS "mac80211"

mac80211_hostapd_setup_base() {
	local phy="$1"
	local ifname="$2"

	cfgfile="/var/run/hostapd-$phy.conf"
	macfile="/var/run/hostapd-$phy.maclist"
	[ -e "$macfile" ] && rm -f "$macfile"

	config_get device "$vif" device
	config_get country "$device" country
	config_get hwmode "$device" hwmode
	config_get channel "$device" channel
	config_get_bool noscan "$device" noscan
	[ -n "$channel" -a -z "$hwmode" ] && wifi_fixup_hwmode "$device"
	[ "$channel" = auto ] && channel=
	[ -n "$hwmode" ] && {
		config_get hwmode_11n "$device" hwmode_11n
		[ -n "$hwmode_11n" ] && {
			hwmode="$hwmode_11n"
			append base_cfg "ieee80211n=1" "$N"
			config_get htmode "$device" htmode
			config_get ht_capab_list "$device" ht_capab
			case "$htmode" in
				HT20|HT40+|HT40-) ht_capab="[$htmode]";;
				*)ht_capab=;;
			esac
			for cap in $ht_capab_list; do
				ht_capab="$ht_capab[$cap]"
			done
			[ -n "$ht_capab" ] && append base_cfg "ht_capab=$ht_capab" "$N"
		}
	}

	config_get macfilter "$vif" macfilter
	config_get maclist "$vif" maclist
	[ -n "$maclist" ] && {
        	case "$macfilter" in
	            allow)
		        append base_cfg "macaddr_acl=1" "$N"
			append base_cfg "accept_mac_file=$macfile" "$N"
			;;
		    deny)
			append base_cfg "macaddr_acl=0" "$N"
			append base_cfg "deny_mac_file=$macfile" "$N"
			;;
	        esac
		for mac in $maclist; do
			echo "$mac" >> $macfile
		done
	}

        if [ "$PRODUCT" = "IPnDDL" ];then
            ap_max_inactivity=10
        else
            ap_max_inactivity=180
        fi

	cat >> "$cfgfile" <<EOF
ctrl_interface=/var/run/hostapd-$phy
driver=nl80211
wmm_ac_bk_cwmin=4
wmm_ac_bk_cwmax=10
wmm_ac_bk_aifs=7
wmm_ac_bk_txop_limit=0
wmm_ac_bk_acm=0
wmm_ac_be_aifs=3
wmm_ac_be_cwmin=4
wmm_ac_be_cwmax=10
wmm_ac_be_txop_limit=0
wmm_ac_be_acm=0
wmm_ac_vi_aifs=2
wmm_ac_vi_cwmin=3
wmm_ac_vi_cwmax=4
wmm_ac_vi_txop_limit=94
wmm_ac_vi_acm=0
wmm_ac_vo_aifs=2
wmm_ac_vo_cwmin=2
wmm_ac_vo_cwmax=3
wmm_ac_vo_txop_limit=47
wmm_ac_vo_acm=0
tx_queue_data3_aifs=7
tx_queue_data3_cwmin=15
tx_queue_data3_cwmax=1023
tx_queue_data3_burst=0
tx_queue_data2_aifs=3
tx_queue_data2_cwmin=15
tx_queue_data2_cwmax=63
tx_queue_data2_burst=0
tx_queue_data1_aifs=1
tx_queue_data1_cwmin=7
tx_queue_data1_cwmax=15
tx_queue_data1_burst=3.0
tx_queue_data0_aifs=1
tx_queue_data0_cwmin=3
tx_queue_data0_cwmax=7
tx_queue_data0_burst=1.5
${ap_max_inactivity:+ap_max_inactivity=$ap_max_inactivity}
${hwmode:+hw_mode=$hwmode}
${channel:+channel=$channel}
${country:+country_code=$country}
${noscan:+noscan=$noscan}
$base_cfg

EOF
}

mac80211_hostapd_setup_bss() {
	local phy="$1"
	local vif="$2"

	hostapd_cfg=
	cfgfile="/var/run/hostapd-$phy.conf"
	config_get ifname "$vif" ifname

	if [ -f "$cfgfile" ]; then
		append hostapd_cfg "bss=$ifname" "$N"
	else
		mac80211_hostapd_setup_base "$phy" "$ifname"
		append hostapd_cfg "interface=$ifname" "$N"
	fi

	local net_cfg bridge
	net_cfg="$(find_net_config "$vif")"
	[ -z "$net_cfg" ] || bridge="$(bridge_interface "$net_cfg")"
	config_set "$vif" bridge "$bridge"

	hostapd_set_bss_options hostapd_cfg "$vif"

	config_get_bool wds "$vif" wds 0
	[ "$wds" -gt 0 ] && append hostapd_cfg "wds_sta=1" "$N"

	local macaddr hidden maxassoc wmm
	config_get macaddr "$vif" macaddr
	config_get maxassoc "$vif" maxassoc
	config_get_bool hidden "$vif" hidden 0
	config_get_bool wmm "$vif" wmm 1
	cat >> /var/run/hostapd-$phy.conf <<EOF
$hostapd_cfg
wmm_enabled=$wmm
bssid=$macaddr
ignore_broadcast_ssid=$hidden
${maxassoc:+max_num_sta=$maxassoc}
EOF
}

mac80211_start_vif() {
	local vif="$1"
	local ifname="$2"

	local net_cfg
	local bonding_cfg
	net_cfg="$(find_net_config "$vif")"
	config_get_bool bonding_cfg "$vif" bonding 0
	
	config_get mode "$vif" mode 
        # bonding only work in mesh mode
        [ "$mode" = "mesh" ] || bonding_cfg=0
	[ -z "$net_cfg" ] || start_net "$ifname" "$net_cfg" "" "$bonding_cfg"

	set_wifi_up "$vif" "$ifname"
}

find_mac80211_phy() {
	local device="$1"

	local macaddr="$(config_get "$device" macaddr | tr 'A-Z' 'a-z')"
	config_get phy "$device" phy
	[ -z "$phy" -a -n "$macaddr" ] && {
		for phy in $(ls /sys/class/ieee80211 2>/dev/null); do
			[ "$macaddr" = "$(cat /sys/class/ieee80211/${phy}/macaddress)" ] || continue
			config_set "$device" phy "$phy"
			break
		done
		config_get phy "$device" phy
	}
	[ -n "$phy" -a -d "/sys/class/ieee80211/$phy" ] || {
		echo "PHY for wifi device $1 not found"
		return 1
	}
	[ -z "$macaddr" ] && {
		config_set "$device" macaddr "$(cat /sys/class/ieee80211/${phy}/macaddress)"
	}
	return 0
}

scan_mac80211() {
	local device="$1"
	local adhoc sta ap monitor mesh repeater

	config_get vifs "$device" vifs
	for vif in $vifs; do
		config_get mode "$vif" mode
		case "$mode" in
			adhoc|sta|ap|monitor|mesh|repeater)
				append $mode "$vif"
			;;
			*) echo "$device($vif): Invalid mode, ignored."; continue;;
		esac
	done

	config_set "$device" vifs "${ap:+$ap }${adhoc:+$adhoc }${sta:+$sta }${monitor:+$monitor }${mesh:+$mesh}${repeater:+$repeater}" 
}

list_phy_interfaces() {
	local phy="$1"
	if [ -d "/sys/class/ieee80211/${phy}/device/net" ]; then
		ls "/sys/class/ieee80211/${phy}/device/net" 2>/dev/null;
	else
		ls "/sys/class/ieee80211/${phy}/device" 2>/dev/null | grep net: | sed -e 's,net:,,g'
	fi
}

disable_mac80211() (
	local device="$1"
	
	find_mac80211_phy "$device" || return 0
	config_get phy "$device" phy
	
        config_get vifs "$device" vifs 
	
	set_wifi_down "$device"
	# kill all running hostapd and wpa_supplicant processes that
	# are running on atheros/mac80211 vifs
	for pid in `pidof hostapd`; do
		grep -E "$phy" /proc/$pid/cmdline >/dev/null && \
			kill $pid
	done

	include /lib/network
	
	for vif in $vifs; do
	    config_get ifname "$vif" ifname
	    unbridge "$ifname"
	    #force removing the bonding interface
	    config_get_bool disabled "$device" disabled 0
	    [ "$disabled" = 0 ] && unbridge bond0 2>/dev/null >/dev/null   
	done
	
	for wdev in $(list_phy_interfaces "$phy"); do
		[ -f "/var/run/$wdev.pid" ] && kill $(cat /var/run/$wdev.pid) >&/dev/null 2>&1
		for pid in `pidof wpa_supplicant`; do
			grep "$wdev" /proc/$pid/cmdline >/dev/null && \
				kill $pid
		done
		ifconfig "$wdev" down 2>/dev/null
		#this is wrong $dev is not defined so I remove it
		#unbridge "$dev"
		iw dev "$wdev" del
	done

        if [ "$HARDWARE" = "5.0.0" -o "$HARDWARE" = "3.0.0" ]; then
                     echo "0" > /sys/class/leds/ath5k-"$phy"\:\:tx/brightness
                     echo "0" > /sys/class/leds/"$phy"_rssi1/brightness
                     echo "0" > /sys/class/leds/"$phy"_rssi2/brightness
                     echo "0" > /sys/class/leds/"$phy"_rssi3/brightness
                     pid=$(ps|grep ledcon.sh |grep $phy |awk '{print $1}')
                     kill $pid
        elif [ "$HARDWARE" = "v1.0.0" ]; then
                     echo "0" > /sys/class/leds/ledrx/brightness
                     echo "0" > /sys/class/leds/ledtx/brightness                     
                     killall mpci-led
                     mpci-led -s 0
        elif [ "$HARDWARE" = "v2.0.0" ]; then
                     logger "<mac80211.sh/disable_mac80211> v2.0.0 " 
                     #echo "1" > /sys/class/leds/RSSIMIN/brightness
                     #echo "1" > /sys/class/leds/RSSIMID/brightness
                     #echo "1" > /sys/class/leds/RSSIMAX/brightness
                     ##pid=$(ps|grep ledcon.sh |grep $phy |awk '{print $1}')
                     ##kill $pid
                     #killall -9 ledcon.sh                                        
        else
                     echo "0" > /sys/class/leds/ath5k-"$phy"\:\:tx/brightness
                     echo "0" > /sys/class/leds/"$phy"_rssi1/brightness
                     echo "0" > /sys/class/leds/"$phy"_rssi2/brightness
                     echo "0" > /sys/class/leds/"$phy"_rssi3/brightness
                     echo "0" > /sys/class/leds/"$phy"_rssi4/brightness
                     echo "0" > /sys/class/leds/"$phy"_rssi5/brightness
                     echo "0" > /sys/class/leds/"$phy"_rssi6/brightness
                     pid=$(ps|grep ledcon.sh |grep $phy |awk '{print $1}')
                     kill $pid
        fi

	return 0
)
get_freq() {
	local phy="$1"
	local chan="$2"
        #[ "$(echo $chan|cut -c 1)" = "0" ] && chan=$(echo $chan|cut -c 2)
        chan=$(echo $chan|awk '{print $1+0}')
	iw "$phy" info | grep -E -m1 "(\* ${chan:-....} MHz${chan:+|\\[$chan\\]})" | grep MHz | awk '{print $2}'
}

get_max_txpower() {
	local phy="$1"
	local chan="$2"
        #[ "$(echo $chan|cut -c 1)" = "0" ] && chan=$(echo $chan|cut -c 2)
        chan=$(echo $chan|awk '{print $1+0}')
	iw "$phy" info | grep -E -m1 "(\* ${chan:-....} MHz${chan:+|\\[$chan\\]})" | grep MHz | awk -F '(' '{print $2}'| awk '{print $1+0}'
}

enable_mac80211() {
	local device="$1"
	config_get channel "$device" channel
	config_get vifs "$device" vifs
	config_get country "$device" country
	config_get distance "$device" distance
	config_get bwmode "$device" bwmode
	config_get txantenna "$device" txantenna all
	config_get rxantenna "$device" rxantenna all
	config_get frag "$device" frag
	config_get rts "$device" rts
	config_get fragoff "$device" fragoff
	config_get rtsoff "$device" rtsoff
	config_get txaggr "$device" txaggr
        config_get sgi "$device" sgi

	find_mac80211_phy "$device" || return 0
	config_get phy "$device" phy
	local i=0
	local macidx=0
	local apidx=0
	fixed=""
	local hostapd_ctrl=""

	[ -n "$country" ] && iw reg set "$country"
	[ "$channel" = "auto" -o "$channel" = "0" ] || {
		fixed=1
	}

	iw phy "$phy" set antenna $txantenna $rxantenna >/dev/null 2>&1

        [ "$fragoff" = "1" ] && {
                iw phy "$phy" set frag off
        } || {
    	    [ -n "$frag" ] && [ "$frag" -lt "2347" ] && {
                iw phy "$phy" set frag "${frag%%.*}"
            }
        }

        [ "$rtsoff" = "1" ] && {
                iw phy "$phy" set rts off
        } || {
    	    [ -n "$rts" ] && [ "$rts" -lt "2347" ] && {
                iw phy "$phy" set rts "${rts%%.*}"
            }
        }

	[ -n "$bwmode" ] && {
    	    case "$bwmode" in
	 	     "5mHz")
      			   testmode "$phy" bwmode 1
		     ;;
	 	     "10mHz")
      			   testmode "$phy" bwmode 2
		     ;;
	 	     "40mHz")
      			   testmode "$phy" bwmode 3
		     ;;
		     *)
      			   testmode "$phy" bwmode 0
		     ;;
	    esac
	}

	export channel fixed
	# convert channel to frequency
	local freq="$(get_freq "$phy" "${fixed:+$channel}")"
        local max_txpower="$(get_max_txpower "$phy" "${fixed:+$channel}")"

	wifi_fixup_hwmode "$device" "g"
	for vif in $vifs; do
		while [ -d "/sys/class/net/wlan$i" ]; do
			i=$(($i + 1))
		done

		config_get ifname "$vif" ifname
		[ -n "$ifname" ] || {
			ifname="wlan$i"
		}
		config_set "$vif" ifname "$ifname"

		config_get mode "$vif" mode
		config_get ssid "$vif" ssid

		# It is far easier to delete and create the desired interface
		case "$mode" in
			adhoc)
				iw phy "$phy" interface add "$ifname" type adhoc
			;;
			ap)
				# Hostapd will handle recreating the interface and
				# it's accompanying monitor
				apidx="$(($apidx + 1))"
				i=$(($i + 1))
				[ "$apidx" -gt 1 ] || iw phy "$phy" interface add "$ifname" type managed
			;;
			mesh)
				config_get mesh_id "$vif" mesh_id
				iw phy "$phy" interface add "$ifname" type mp mesh_id "$mesh_id"
			;;
			monitor)
				iw phy "$phy" interface add "$ifname" type monitor
			;;
			sta)
				local wdsflag
				config_get_bool wds "$vif" wds 0
				[ "$wds" -gt 0 ] && wdsflag="4addr on"
				iw phy "$phy" interface add "$ifname" type managed $wdsflag
				config_get_bool powersave "$vif" powersave 0
				[ "$powersave" -gt 0 ] && powersave="on" || powersave="off"
				iwconfig "$ifname" power "$powersave"
			;;
			repeater)
				iw phy "$phy" interface add "$ifname.rep" type managed 4addr on
                                config_get_bool powersave "$vif" powersave 0
				[ "$powersave" -gt 0 ] && powersave="on" || powersave="off"
				iwconfig "$ifname.rep" power "$powersave" 
				#set the mac addr of 02:xx:xx:xx:xx:xx
                                config_get macaddr "$device" macaddr
		                local mac_1="${macaddr%%:*}"
		                local mac_2="${macaddr#*:}"
                                vif_mac="$( printf %02x $((0x$mac_1 + 2)) ):$mac_2"
                                ifconfig "$ifname.rep" hw ether "$vif_mac"

				# we set the ssid here and stop the ap scan to provent the hardware competetion
				iwconfig "$ifname.rep" essid "$ssid"

				# Hostapd will handle recreating the interface and
				# it's accompanying monitor
				apidx="$(($apidx + 1))"
				i=$(($i + 1))
				[ "$apidx" -gt 1 ] || iw phy "$phy" interface add "$ifname" type managed
			;;

		esac

		# All interfaces must have unique mac addresses
		# which can either be explicitly set in the device
		# section, or automatically generated
		config_get macaddr "$device" macaddr
		local mac_1="${macaddr%%:*}"
		local mac_2="${macaddr#*:}"

		config_get vif_mac "$vif" macaddr
		[ -n "$vif_mac" ] || {
			if [ "$macidx" -gt 0 ]; then
				offset="$(( 2 + $macidx * 4 ))"
			else
				offset="0"
			fi
			vif_mac="$( printf %02x $((0x$mac_1 + $offset)) ):$mac_2"
			macidx="$(($macidx + 1))"
		}
		[ "$mode" = "ap" -o "$mode" = "repeater" ] || ifconfig "$ifname" hw ether "$vif_mac"
		config_set "$vif" macaddr "$vif_mac"

		# !! ap !!
		#
		# ALL ap functionality will be passed to hostapd
		#
		# !! station !!
		#
		# ALL station functionality will be passed to wpa_supplicant
		#
		[ "$mode" = "ap" -o "$mode" = "repeater" ] || {
			# We attempt to set the channel for all interfaces, although
			# mac80211 may not support it or the driver might not yet
			# for ap mode this is handled by hostapd
			[ -n "$fixed" -a -n "$channel" ] && iw dev "$ifname" set channel "$channel"
		}

		config_get vif_txpower "$vif" txpower
		# use vif_txpower (from wifi-iface) to override txpower (from
		# wifi-device) if the latter doesn't exist
		txpower_t="${txpower:-$vif_txpower}"
                logger -t "mac80211 txpower" "user setting is $txpower_t the max can be set is $max_txpower ."
		[ -z "$txpower_t" ] || {
                    if [ "$HARDWARE" = "v1.0.0" ];then
                        txpower=$(expr $txpower_t - 10)
                    else
                        txpower=$txpower_t
                    fi
                    [ -z "$max_txpower" ] || {
                        if [ "$txpower" -gt "$max_txpower" ]; then
                            txpower=$max_txpower
                        fi
                    }
                    iw dev "$ifname" set txpower fixed "${txpower%%.*}00"
                }
		# rate is not yet implemented in iw
		config_get vif_rate "$vif" rate
		rate="${rate:-$vif_rate}"
		#[ -z "$rate" ] || iwconfig "$ifname" rate "${rate%%.*}"
	        #### support for mcs ####
		config_get hwmode "$device" hwmode
		## rate setting
                UB_CONFIG=$(grep params /proc/mtd | cut -d: -f1)
                Testlab=$(grep "TESTLAB" /dev/$UB_CONFIG | cut -d= -f2)

                if [ "$Testlab" = "1" ]; then 
                    if [ "$rate" = "auto" ]; then
                        iw "$ifname" set bitrates
                    else
                        if [ "$hwmode" = "11na" -o "$hwmode" = "a" ];then
                            iw "$ifname" set bitrates legacy-5 "${rate%%M}"
                        else
                            iw "$ifname" set bitrates legacy-2.4 "${rate%%M}"
                        fi
                    fi
                else
                    [ "$hwmode" = "11na" -o "$hwmode" = "11ng" ] || {
        	        [ -z "$rate" ] || {
			    if [ "$rate" = "auto" ]; then
				iw "$ifname" set bitrates
			    else
				iw phy |grep -q "MCS rate"
				if [ "$?" = "0" ]; then
				    [ "$hwmode" = "a" ] && {
					iw "$ifname" set bitrates legacy-5 "${rate%%M}"
				    } || {
					iw "$ifname" set bitrates legacy-2.4 "${rate%%M}"
				    }
				else
				    iwconfig "$ifname" rate "${rate%%.*}"
				fi
			    fi
			}
        	    } && {
                        if [ "$rate" = "auto" ]; then
                            iw "$ifname" set bitrates 
                        else 
                            [ "$hwmode" = "11ng" ] && {
        		        [ -z "$rate" ] || iw "$ifname" set bitrates legacy-2.4 54 mcs-2.4 "${rate%%.*}"
        		    }
                            [ "$hwmode" = "11na" ] && {
        		        [ -z "$rate" ] || iw "$ifname" set bitrates legacy-5 54 mcs-5 "${rate%%.*}"
        		    }
                        fi
                                 
                    }           
                fi
	done

	local start_hostapd=
	rm -f /var/run/hostapd-$phy.conf
	for vif in $vifs; do
		config_get mode "$vif" mode
		[ "$mode" = "ap" -o "$mode" = "repeater" ] || continue
		mac80211_hostapd_setup_bss "$phy" "$vif"
		start_hostapd=1
	done

	[ -n "$start_hostapd" ] && {
		hostapd -P /var/run/wifi-$phy.pid -B /var/run/hostapd-$phy.conf || {
			echo "Failed to start hostapd for $phy"
			return
		}
		sleep 2

		for vif in $vifs; do
			config_get mode "$vif" mode
			config_get ifname "$vif" ifname
			[ "$mode" = "ap" -o "$mode" = "repeater" ] || continue
			hostapd_ctrl="${hostapd_ctrl:-/var/run/hostapd-$phy/$ifname}"
			mac80211_start_vif "$vif" "$ifname"
		done
	}

	for vif in $vifs; do
		config_get mode "$vif" mode
		config_get ifname "$vif" ifname

		if [ ! "$mode" = "ap" ]; then
			ifconfig "$ifname" up
			case "$mode" in
				adhoc)
					config_get bssid "$vif" bssid
					config_get ssid "$vif" ssid
					config_get bintval "$vif" bintval
					config_get basicrates "$vif" basicrates
					config_get encryption "$vif" encryption
					config_get key "$vif" key 1
					config_get mcast_rate "$vif" mcast_rate

					local keyspec=""
					[ "$encryption" = "wep" ] && {
						case "$key" in
							[1234])
								local idx
								for idx in 1 2 3 4; do
									local ikey
									config_get ikey "$vif" "key$idx"

									[ -n "$ikey" ] && {
										ikey="$(($idx - 1)):$(prepare_key_wep "$ikey")"
										[ $idx -eq $key ] && ikey="d:$ikey"
										append keyspec "$ikey"
									}
								done
							;;
							*) append keyspec "d:0:$(prepare_key_wep "$key")" ;;
						esac
					}

					local mcval=""
					[ -n "$mcast_rate" ] && {
						mcval="$(($mcast_rate / 1000))"
						mcsub="$(( ($mcast_rate / 100) % 10 ))"
						[ "$mcsub" -gt 0 ] && mcval="$mcval.$mcsub"
					}

					iw dev "$ifname" ibss join "$ssid" $freq \
						${fixed:+fixed-freq} $bssid \
						${bintval:+beacon-interval $bintval} \
						${basicrates:+basic-rates $basicrates} \
						${mcval:+mcast-rate $mcval} \
						${keyspec:+keys $keyspec}
				;;
				sta)
                                        if [ "$PRODUCT" = "IPnDDL" ];then
        					if eval "type wpa_supplicant_setup_vif" 2>/dev/null >/dev/null; then
        						wpa_supplicant_setup_vif "$vif" nl80211 "${hostapd_ctrl:+-H $hostapd_ctrl}" "$freq" || {
        							echo "enable_mac80211($device): Failed to set up wpa_supplicant for interface $ifname" >&2
        							# make sure this wifi interface won't accidentally stay open without encryption
        							ifconfig "$ifname" down
        							continue
        						}

        					fi
                                        else
        					if eval "type wpa_supplicant_setup_vif" 2>/dev/null >/dev/null; then
        						wpa_supplicant_setup_vif "$vif" nl80211 "${hostapd_ctrl:+-H $hostapd_ctrl}" || {
        							echo "enable_mac80211($device): Failed to set up wpa_supplicant for interface $ifname" >&2
        							# make sure this wifi interface won't accidentally stay open without encryption
        							ifconfig "$ifname" down
        							continue
        						}

        					fi
                                        fi

				;;
				repeater)
					if eval "type wpa_supplicant_setup_vif" 2>/dev/null >/dev/null; then
						wpa_supplicant_setup_vif "$vif" nl80211 "${hostapd_ctrl:+-H $hostapd_ctrl}" "$freq" || {
							echo "enable_mac80211($device): Failed to set up wpa_supplicant for interface $ifname.rep" >&2
							# make sure this wifi interface won't accidentally stay open without encryption
							ifconfig "$ifname.rep" down 
							continue
						}
					fi
				;;
				mesh)
					config_get enc "$vif" encryption                 
                                                                     
					# Examples:                                                  
					# psk-mixed/tkip        => WPA1+2 PSK, TKIP                  
					# wpa-psk2/tkip+aes     => WPA2 PSK, CCMP+TKIP               
					# wpa2/tkip+aes         => WPA2 RADIUS, CCMP+TKIP            
					# ...                
       
					# use crypto/auth settings for building the hostapd config              
					ifconfig "$ifname" down
					iw dev "$ifname" encryption reset
					case "$enc" in                                                          
						*psk*)
							config_get psk "$vif" key                               
							if [ ${#psk} -eq 64 ]; then                             
								eval wpa_psk="$psk"               
							else                                                    
								eval wpa_passphrase="$psk"
							fi  
                                                    
							case "$enc" in 
								*psk2*)
									crypto="ccmp"
								;;
								*aes)      
                                                                        crypto="ccmp"
                                                                ;;      
								*)
									crypto="tkip"
								;;
							esac	
							iw dev "$ifname" encryption alg "$crypto"
							iw dev "$ifname" encryption new_key 1 p:"$wpa_passphrase"
						;;
						*wep*)                                                          
							config_get key "$vif" key 1 
							case "$key" in                                          
								[1234])                                         
									config_get ckey "$vif" "key${key}"
									[ -n "$ckey" ] && {
									    eval wep_key=$(prepare_key_wep "$ckey")
									    eval wep_default_key=$key
									}
								;;                                              
								*)                                              
									eval wep_key=$(prepare_key_wep "$key")
									wep_default_key=1
								;;                                              
							esac                                                    
							crypto=wep
							iw dev "$ifname" encryption alg "$crypto"
							iw dev "$ifname" encryption new_key "$wep_default_key" "$wep_key"
						;;

						*)
							# mesh does not support radius encryption so it goes here
							crypto="none"
							iw dev "$ifname" encryption alg "$crypto"
						;;
					esac
					#restart the interface and commit the encryption setting
					ifconfig "$ifname" up
					iw dev "$ifname" encryption commit
					
				;;
			esac
			mac80211_start_vif "$vif" "$ifname"
			#re-set the channel to make it visible to the user at mesh mode
			[ -n "$fixed" -a -n "$channel" ] && iw dev "$ifname" set channel "$channel"
		fi
	done

	# move the distance parameter here since the interface is up
        [ -n "$distance" ] && iw phy "$phy" set distance "$distance"

        if [ "$HARDWARE" = "5.0.0" -o "$HARDWARE" = "3.0.0" ]; then 
               /etc/ledcon.sh $phy 2>/dev/null &
        elif [ "$HARDWARE" = "v1.0.0" ]; then
               /etc/init.d/ledcon start
        elif [ "$HARDWARE" = "v2.0.0" ]; then
             logger "<mac80211.sh/enable_mac80211> v2.0.0 "
             killall mpci-led #for fix bug after upgrading firmware
             #echo "default-on" > /sys/class/leds/cpustatus/trigger  
             #/etc/ledcon.sh $phy 2>/dev/null &   #we use three rssi leds for celluar not wifi 
        else 
             /etc/ledcon.sh $phy 2>/dev/null &
        fi

	config_get hwmode "$device" hwmode
	[ "$hwmode" = "11na" -o "$hwmode" = "11ng" ] && {
                [ -n "$txaggr" ] && {
                    case "$txaggr" in
 	                0)     testmode "$phy" txaggr 0 ;;	#disable tx aggregation		     
	                *)     testmode "$phy" txaggr 1 ;;
                    esac
                }
                [ -n "$sgi" ] && [ "$htmode" != "HT20" ] && { 
                    case "$sgi" in
 	                0)     testmode "$phy" sgi 0 ;;	#disable short GI		     
	                *)     testmode "$phy" sgi 1 ;;
                    esac
                }
	}
}

delete_all_section() {
      uci delete wireless."$1"
}

check_macaddr() {
	config_get macaddr "$1" macaddr
	[ -n "$macaddr" ] && {
	    [ "$macaddr" = "$(cat /sys/class/ieee80211/${dev}/macaddress)" ] && found=1 
	}
}

check_device() {
	config_get phy "$1" phy
	[ -z "$phy" ] && {
		find_mac80211_phy "$1" >/dev/null || return 0
		config_get phy "$1" phy
	}
	[ "$phy" = "$dev" ] && found=1
}

detect_mac80211() {
	devidx=0
	config_load wireless
	#while :; do
	#	config_get type "radio$devidx" type
	#	[ -n "$type" ] || break
	#	devidx=$(($devidx + 1))
	#done

	#check whether the macaddr is altered
	#if so re-initialize the config file
	for dev in $(ls /sys/class/ieee80211); do
		found=0
		config_foreach check_macaddr wifi-device
		[ "$found" -le 0 ] && {
		     config_foreach delete_all_section
		     uci commit wireless
		}
        done

	config_load wireless

	for dev in $(ls /sys/class/ieee80211); do
		found=0
                devidx=$(echo $dev|cut -c 4)
		config_foreach check_device wifi-device
		[ "$found" -gt 0 ] && continue

		mode_11n=""
		mode_band="g"
		channel="11"
                bw="20mHz"
		ht_cap=0
                ssid="wlan$devidx"
                rate="auto"
                txpower="17"
		for cap in $(iw phy "$dev" info | grep 'Capabilities:' | cut -d: -f2); do
			ht_cap="$(($ht_cap | $cap))"
		done
		ht_capab="";
		[ "$ht_cap" -gt 0 ] && {
			mode_11n="n"
			append ht_capab "	option htmode	HT20" "$N"
                        append ht_capab "	option txaggr	1" "$N"
                        append ht_capab "	option sgi	1" "$N"

			list="	list ht_capab"
			[ "$(($ht_cap & 1))" -eq 1 ] && append ht_capab "$list	LDPC" "$N"
			[ "$(($ht_cap & 16))" -eq 16 ] && append ht_capab "$list	GF" "$N"
			[ "$(($ht_cap & 32))" -eq 32 ] && append ht_capab "$list	SHORT-GI-20" "$N"
			[ "$(($ht_cap & 64))" -eq 64 ] && append ht_capab "$list	SHORT-GI-40" "$N"
			[ "$(($ht_cap & 128))" -eq 128 ] && append ht_capab "$list	TX-STBC" "$N"
			[ "$(($ht_cap & 768))" -eq 256 ] && append ht_capab "$list	RX-STBC1" "$N"
			[ "$(($ht_cap & 768))" -eq 512 ] && append ht_capab "$list	RX-STBC12" "$N"
			[ "$(($ht_cap & 768))" -eq 768 ] && append ht_capab "$list	RX-STBC123" "$N"
			[ "$(($ht_cap & 4096))" -eq 4096 ] && append ht_capab "$list	DSSS_CCK-40" "$N"
		}

		iw phy "$dev" info | grep -q '2412 MHz' || {

                    if [ "$PRODUCT" = "VIP421EXP49G" -o "$PRODUCT" = "VIP4214900X" -o "$PRODUCT" = "VIP4214900XEXP" ];then
                        mode_band="a"
                        channel="20"
                    else
                        mode_band="a"
                        channel="153"
                    fi
               }

                Distance=10000
                if [ "$HARDWARE" = "v1.0.0" ];then
                    Distance=3000
                    ssid="$PRODUCTNAME"
                    if [ "$PRODUCT" = "IPnDDL" ];then
                         bw="10mHz"
                         mode_band="g"
                         channel="5"
                         ssid="IPnDDL1"
                         txpower=20
                         rate="auto"
                    fi
                fi
                if [ "$PRODUCT" = "VIP4G" -o "$PRODUCT" = "VIP4GEXP" ];then
                    ssid="$PRODUCT"
                fi

                UB_CONFIG=$(grep params /proc/mtd | cut -d: -f1)
                Testlab=$(grep "TESTLAB" /dev/$UB_CONFIG | cut -d= -f2)
                if [ "$Testlab" = "1" ]; then
		    tmode="mesh"
		    tmid="testlab"
		else
        	    tmode="ap"
		    tmid="$PTODUCT"
		fi 


cat <<EOF
config wifi-device  radio$devidx
	option type     mac80211
	option channel  ${channel}
	option macaddr	$(cat /sys/class/ieee80211/${dev}/macaddress)
	option hwmode	11${mode_11n}${mode_band}
	option bwmode	${bw}
	option distance ${Distance}
        option frag     2346
	option rts      2346
        option fragoff  1
        option rtsoff   1
        $ht_capab
	# REMOVE THIS LINE TO ENABLE WIFI:
	option disabled 0

config wifi-iface
	option device   radio$devidx
	option ifname   wlan$devidx
	option network  lan
	option bonding  0
	option mode     ${tmode}
        option mesh_id  ${tmid}
	option ssid     ${ssid}
	option encryption none
	option txpower  ${txpower}
	option wds      1
        option rate     ${rate}
EOF
	devidx=$(($devidx + 1))
	done
}

