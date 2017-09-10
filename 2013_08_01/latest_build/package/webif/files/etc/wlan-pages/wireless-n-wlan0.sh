#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
. /etc/version
###################################################################
# Wireless configuration
#
# Description:
#	Wireless configuration.
#
# Author(s) [in order of work date]:
#       Original webif authors.
#	Jeremy Collake <jeremy.collake@gmail.com>
#	Travis Kemen	<kemen04@gmail.com>
# Major revisions:
#
# UCI variables referenced:
#
# Configuration files referenced:
#   wireless
#
radioifname="wlan0"
radioindex="1"
radiophy="phy0"
radiodevice="radio0"
radioheader="Radio1"

#determine supported protocol
mode_a=""
mode_b=""
mode_g=""
mode_n=""
supported_protocol=""
iw phy |grep "MHz"|grep -q "* 24"
equal "$?" 0 && {
	mode_b="b"
	mode_g="g"
}
iw phy|grep "MHz" |grep -q "* 5"
equal "$?" 0 && {
	mode_a="a"
}
iw phy |grep -q "MCS rate"
equal "$?" 0 && {
	mode_n="n"
}
supported_protocol=11${mode_a}${mode_b}${mode_g}${mode_n}

#IEEE 802.11n Annex J.
NG_CHANNELS_HT20="1 2 3 4 5 6 7 8 9 10 11"
NG_CHANNELS_HT40MINUS="5 6 7 8 9 10 11"
NG_CHANNELS_HT40PLUS="1 2 3 4 5 6 7"

NA_CHANNELS_HT20="149 153 157 161 165"
#NA_CHANNELS_HT40MINUS="40 44 48 153 157 161 165"
#NA_CHANNELS_HT40PLUS="36 40 44 149 153 157 161"
NA_CHANNELS_HT40MINUS="153 161"
NA_CHANNELS_HT40PLUS="149 157"

if [ -n "$FORM_generate_wireless" ]; then
	wifi detect > /etc/config/wireless
	unset FORM_submit
fi
if [ ! -s /etc/config/wireless ]; then
	header "Wireless" "Wlan0" "@TR<<Wireless Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"
	echo "@TR<<Generate Wireless Config Waring#No wireless configuration detected. Please make sure you have the correct wireless driver installed for your device.>>"
	display_form <<EOF
onchange|modechange
submit|generate_wireless|@TR<<Generate Wireless Config>>
EOF
	footer
	break
fi

[ "$PRODUCT" = "VIP421EXP49G" -o "$PRODUCT" = "VIP4214900X" -o "$PRODUCT" = "VIP4214900XEXP" -o "$PRODUCT" = "VIP4215800X" -o "$PRODUCT" = "VIP4215800XEXP" -o "$PRODUCT" = "VIP4212400X" -o "$PRODUCT" = "VIP4212400XEXP" ] && { 
    cardtype=$(cat /lib/mdf/cardtype)
}

generate_channels() {
	iwlist $radioifname channel 2>&- |grep -q "GHz"
	if [ "$?" != "0" ]; then
		is_package_installed kmod-madwifi
		if [ "$?" = "0" ]; then
			wlanconfig ath create wlandev wifi0 wlanmode ap 2>/dev/null >/dev/null
			cleanup=1
		fi
	fi
	BGCHANNELS="$(iwlist $radioifname channel 2>&- |grep -v "no frequency information." |grep -v "[Ff]requenc" |grep -v "Current" |grep "2.[0-9]" |cut -d' ' -f12|sort |uniq)"
	ACHANNELS="$(iwlist $radioifname channel 2>&- |grep -v "no frequency information." |grep -v "[Ff]requenc" |grep -v "Current" |grep "5.[0-9]" |cut -d' ' -f12|sort |uniq)"
        
	NG_CHANNELS="$(iw phy |grep "MHz"|grep -v "disabled"|grep -v "radar detection"|grep "* 24"|cut -d' ' -f4|cut -d'[' -f2|cut -d']' -f1|uniq)"
        NA_CHANNELS="$(iw phy |grep "MHz"|grep -v "disabled"|grep -v "radar detection"|grep "* 5"|cut -d' ' -f4|cut -d'[' -f2|cut -d']' -f1|uniq)"
	if ! empty "$ACHANNELS" ; then
    	    #ACHANNELS="36 40 44 48 149 153 157 161 165"
            ACHANNELS="149 153 157 161 165"
	fi
	echo "BGCHANNELS=\"${BGCHANNELS}\"" > /usr/lib/webif/channels.lst$radioindex
	echo "ACHANNELS=\"${ACHANNELS}\"" >> /usr/lib/webif/channels.lst$radioindex
	echo "NG_CHANNELS=\"${NG_CHANNELS}\"" >> /usr/lib/webif/channels.lst$radioindex
	echo "NA_CHANNELS=\"${NA_CHANNELS}\"" >> /usr/lib/webif/channels.lst$radioindex
	if [ "$cleanup" = "1" ]; then
		wifi 2>/dev/null >/dev/null
	fi
}

[ "$PRODUCT" = "VIP421EXP49G" -o "$PRODUCT" = "VIP4214900X" -o "$PRODUCT" = "VIP4214900XEXP" -o "$PRODUCT" = "VIP4215800X" -o "$PRODUCT" = "VIP4215800XEXP" -o "$PRODUCT" = "VIP4212400X" -o "$PRODUCT" = "VIP4212400XEXP" ] || {
    if [ ! -f /usr/lib/webif/channels.lst$radioindex ]; then
        	generate_channels
    fi

    [ -f /usr/lib/webif/channels.lst$radioindex ] && . /usr/lib/webif/channels.lst$radioindex
    if [ -z "$BGCHANNELS" -a -z "$ACHANNELS" ]; then
	generate_channels
    fi
}

dmesg_txt="$(dmesg)"
mesh_count=0
ap_count=0
sta_count=0
validate_wireless() {
	case "$mesh_count:$sta_count:$ap_count" in
		0:0:?)
			if [ "$ap_count" -gt "4" ]; then
				append validate_error "string|<h3>@TR<<Error: Only 4 virtual adapters are allowed in ap mode.>></h3><br />"
			fi
			;;
		0:?:?)
			if [ "$sta_count" -gt "1" ]; then
				append validate_error "string|<h3>@TR<<Error: Only 1 adaptor is allowed in client mode.>></h3><br />"
			fi
			if [ "$1"="broadcom" ]; then
				if [ "$ap_count" -gt "3" ]; then
					append validate_error "string|<h3>@TR<<Error: Only 3 virtual adapters are allowed in ap mode with a adapter in client mode.>></h3><br />"
				fi
			elif [ "$1"="atheros" ]; then
				if [ "$ap_count" -gt "4" ]; then
					append validate_error "string|<h3>@TR<<Error: Only 4 virtual adapters are allowed in ap mode.>></h3><br />"
				fi	
			fi
			;;
	esac
	# Validate multi-SSID configuration
	#if [ "$vcfgcnt" -gt "1" -a "$vcfgcnt" -ne "$ap_count" ]; then
	#	append validate_error "string|<h3>@TR<<Error: Multi-SSID feature is only allowed in ap mode.>></h3><br />"
	#fi

	#reset variables
	mesh_count=0
	ap_count=0
	sta_count=0
}

###################################################################
# Add Virtual Interface
if ! empty "$FORM_add_vcfg"; then

	uci_add "wireless" "wifi-iface" ""; wireless_cfg="$CONFIG_SECTION"
	uci_set "wireless" "$wireless_cfg" "device" "$FORM_add_vcfg"
	uci_set "wireless" "$wireless_cfg" "mode" "ap"
        s1=`cat /proc/sys/kernel/random/uuid`
	uci_set "wireless" "$wireless_cfg" "ssid" "Microhard-${s1:0:2}"
	uci_set "wireless" "$wireless_cfg" "hidden" "0"
	uci_set "wireless" "$wireless_cfg" "encryption" "none"
	FORM_add_vcfg=""
fi

###################################################################
# Remove Virtual Interface
if ! empty "$FORM_remove_vcfg"; then
	uci_remove "wireless" "$FORM_remove_vcfg"
fi

###################################################################
# Parse Settings, this function is called when doing a config_load
config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
	        wifi-device)
	                append DEVICES "$cfg_name"
	        ;;
	        wifi-iface)
	                append vface "$cfg_name" "$N"
	        ;;
	        interface)
		        append network_devices "$cfg_name"
	        ;;
	esac
}
uci_load network
NETWORK_DEVICES="$network_devices"
uci_load webif
uci_load wireless

#FIXME: uci_load bug
#uci_load will pass the same config twice when there is a section to be added by using uci_add before a uci_commit happens
#we will use uniq so we don't try to parse the same config section twice.
vface=$(echo "$vface" |uniq)

vcfg_number=$(echo "$DEVICES $N $vface" |wc -l)
let "vcfg_number+=1"

#####################################################################
#setup network device form for vfaces
#
for iface in $NETWORK_DEVICES; do
   if [ "$iface" != "loopback" ]; then
        if [ "$iface" = "lan" ]; then
            ifacename="LAN"
        elif [ "$iface" = "wan" ]; then
            ifacename="WAN"
        elif [ "$iface" = "wan2" ]; then
            ifacename="4G"
	else
	    ifacename=$iface
        fi
        
	network_options="$network_options 
			option|$iface|$ifacename"
   fi
done

#####################################################################
# install wpa packages if needed
#

if ! empty "$FORM_install_nas"; then
	echo "Installing NAS package ...<pre>"
	install_package "nas"
	echo "</pre>"
fi
if ! empty "$FORM_install_hostapd_mini"; then
	echo "Installing HostAPD mini package ...<pre>"
	install_package "hostapd-mini"
	echo "</pre>"
fi
if ! empty "$FORM_install_hostapd"; then
	hostapd_mini_installed="0"
	is_package_installed hostapd-mini
	equal "$?" "0" && opkg remove "hostapd-mini"
	echo "Installing HostAPD package ...<pre>"
	install_package "hostapd"
	echo "</pre>"
fi
if ! empty "$FORM_install_wpa_supplicant"; then
	echo "Installing wpa-supplicant package ...<pre>"
	install_package "wpa-supplicant"
	echo "</pre>"
fi

nas_installed="0"
is_package_installed nas
equal "$?" "0" && nas_installed="1"

hostapd_installed="0"
is_package_installed hostapd
equal "$?" "0" && hostapd_installed="1"

hostapd_mini_installed="0"
is_package_installed hostapd-mini
equal "$?" "0" && hostapd_mini_installed="1"

wpa_supplicant_installed="0"
is_package_installed wpa-supplicant
equal "$?" "0" && wpa_supplicant_installed="1"

#####################################################################
# This is looped for every physical wireless card (wifi-device)
#
DEVICES="$radiodevice"
device="$radiodevice"
	if empty "$FORM_submit"; then
                [ "$PRODUCT" = "VIP421EXP49G" -o "$PRODUCT" = "VIP4214900X" -o "$PRODUCT" = "VIP4214900XEXP" -o "$PRODUCT" = "VIP4215800X" -o "$PRODUCT" = "VIP4215800XEXP" -o "$PRODUCT" = "VIP4212400X" -o "$PRODUCT" = "VIP4212400XEXP" ] && {
                    config_get FORM_apx_mode $device hwmode
    	            config_get FORM_ex49gmode $device ex49gmode
                    if [ "$FORM_ex49gmode" = "x" ] ;then
                        FORM_ap_mode="11ax"
                    else
                        config_get FORM_ap_mode $device hwmode
                    fi
               	} || {
		    config_get FORM_ap_mode $device hwmode
                } 
		config_get iftype "$device" type
                config_get FORM_htmode $device htmode
                config_get FORM_ht_capab $device ht_capab
                config_get FORM_txaggr $device txaggr
                config_get FORM_sgi $device sgi
		#config_get country $device country
		config_get FORM_channel $device channel
		config_get FORM_maxassoc $device maxassoc
		config_get FORM_bwmode $device bwmode
		config_get FORM_distance $device distance
		config_get FORM_txantenna $device txantenna 0
	        config_get FORM_rxantenna $device rxantenna 0
	        config_get_bool FORM_diversity $device diversity 1
	        config_get_bool FORM_disabled $device disabled 0
		config_get FORM_antenna $device antenna
		config_get FORM_frag $device frag
       		config_get FORM_rts $device rts
		config_get FORM_fragoff $device fragoff
       		config_get FORM_rtsoff $device rtsoff
		[ -z $FORM_antenna ] && FORM_antenna=auto
                [ -z $FORM_rtsoff ] && FORM_rtsoff=1
                [ -z $FORM_fragoff ] && FORM_fragoff=1
	else
		config_get iftype "$device" type
		#eval FORM_country="\$FORM_country_$device"
		eval FORM_ap_mode="\$FORM_ap_mode_$device"
                [ "$PRODUCT" = "VIP421EXP49G" -o "$PRODUCT" = "VIP4214900X" -o "$PRODUCT" = "VIP4214900XEXP" -o "$PRODUCT" = "VIP4215800X" -o "$PRODUCT" = "VIP4215800XEXP" -o "$PRODUCT" = "VIP4212400X" -o "$PRODUCT" = "VIP4212400XEXP" ] && {
                    #`cat /lib/mdf/mdfcfg |grep -v "exmode=" > /lib/mdf/mdfcfg_tmp` 
                    if [ "$FORM_ap_mode" = "11ax" ];then 
                	eval FORM_bwmode="\$FORM_xbwmode_$device"
                        FORM_ex49gmode="x"
                        if [ "$FORM_bwmode" = "20mHz" ];then
        		    eval FORM_channel="\$FORM_xachannelnormal_$device"
                        elif [ "$FORM_bwmode" = "10mHz" ];then
        		    eval FORM_channel="\$FORM_xachannelhalf_$device"
                        elif [ "$FORM_bwmode" = "5mHz" ];then
        		    eval FORM_channel="\$FORM_xachannelquat_$device"
                        elif [ "$FORM_bwmode" = "40mHz" ];then
        		    eval FORM_channel="\$FORM_xachanneltubo_$device"
                        fi
                        echo "exmode=$cardtype" > /var/run/mdfcfg_mode
                    else
                        if [ "$cardtype" = "49G" ]; then
                           	eval FORM_bwmode="\$FORM_abwmode_$device"
                        else
                        	eval FORM_bwmode="\$FORM_xbwmode_$device"
                        fi
                        FORM_ex49gmode=""
		        eval FORM_channel="\$FORM_achannelnormal_$device"
                        echo "exmode=${cardtype}ST" > /var/run/mdfcfg_mode
                    fi
                    FORM_apx_mode="11a"
                } || {
                    eval FORM_bwmode="\$FORM_bwmode_$device"
                    if [ "$FORM_ap_mode" = "11na" -o "$FORM_ap_mode" = "11ng" ]; then
		         eval FORM_channel="\$FORM_channel_$device"
		    elif [ "$FORM_ap_mode" = "11a" ]; then
		         eval FORM_channel="\$FORM_achannel_$device"
		    else
                        if [ "$FORM_bwmode" = "40mHz" ];then
		            eval FORM_channel="\$FORM_tbgchannel_$device"
                        else
		            eval FORM_channel="\$FORM_bgchannel_$device"
                        fi    
	            fi
                    eval FORM_channel="\$FORM_channel_$device"
                }
		eval FORM_maxassoc="\$FORM_maxassoc_$device"
		eval FORM_distance="\$FORM_distance_$device"
		eval FORM_diversity="\$FORM_diversity_$device"
		eval FORM_txantenna="\$FORM_txantenna_$device"
		eval FORM_rxantenna="\$FORM_rxantenna_$device"
		eval FORM_disabled="\$FORM_disabled_$device"
		eval FORM_antenna="\$FORM_antenna_$device"
		eval FORM_frag="\$FORM_frag_$device"
		eval FORM_rts="\$FORM_rts_$device"
                eval FORM_fragoff="\$FORM_fragoff_${device}"
                eval FORM_rtsoff="\$FORM_rtsoff_${device}"
                eval FORM_htmode="\$FORM_htmode_${device}"
                eval FORM_txaggr="\$FORM_txaggr_${device}"
                eval FORM_sgi="\$FORM_sgi_${device}"

		empty "$FORM_generate_wep_128" && empty "$FORM_generate_wep_40" &&
		{
                        append validate_forms "int|FORM_frag_${device}|@TR<<Fragmentation Threshold>>|min=256 max=2346 |$FORM_frag" "$N"
                        append validate_forms "int|FORM_rts_${device}|@TR<<RTS Threshold>>|min=256 max=2346 |$FORM_rts" "$N"
                        append validate_forms "int|FORM_distance_${device}|@TR<<Distance>>|min=1 |$FORM_distance" "$N"
			SAVED=1
validate <<EOF
int|FORM_maxassoc_${device}|@TR<<Max Associated Clients>>||$FORM_maxassoc
EOF

			if [ "$?" = 0 ]; then
				#uci_set "wireless" "$device" "country" "$FORM_country"

                                [ "$PRODUCT" = "VIP421EXP49G" -o "$PRODUCT" = "VIP4214900X" -o "$PRODUCT" = "VIP4214900XEXP" -o "$PRODUCT" = "VIP4215800X" -o "$PRODUCT" = "VIP4215800XEXP" -o "$PRODUCT" = "VIP4212400X" -o "$PRODUCT" = "VIP4212400XEXP" ] && {
                                    uci_set "wireless" "$device" "hwmode" "$FORM_apx_mode"
	                            uci_set "wireless" "$device" "ex49gmode" "$FORM_ex49gmode" 
                                } || {
                                    uci_set "wireless" "$device" "hwmode" "$FORM_ap_mode"
                                }
				[ -n "$FORM_channel" ] && uci_set "wireless" "$device" "channel" "$FORM_channel"
				uci_set "wireless" "$device" "maxassoc" "$FORM_maxassoc"
				uci_set "wireless" "$device" "bwmode" "$FORM_bwmode"
				uci_set "wireless" "$device" "distance" "$FORM_distance"
				uci_set "wireless" "$device" "diversity" "$FORM_diversity"
				uci_set "wireless" "$device" "txantenna" "$FORM_txantenna"
				uci_set "wireless" "$device" "rxantenna" "$FORM_rxantenna"
				uci_set "wireless" "$device" "disabled" "$FORM_disabled"
				uci_set "wireless" "$device" "antenna" "$FORM_antenna"
				uci_set "wireless" "$device" "frag" "$FORM_frag"
				uci_set "wireless" "$device" "rts" "$FORM_rts"
                                uci_set "wireless" "$device" "fragoff" "$FORM_fragoff"
				uci_set "wireless" "$device" "rtsoff" "$FORM_rtsoff"
                                uci_set "wireless" "$device" "htmode" "$FORM_htmode"
                                uci_set "wireless" "$device" "txaggr" "$FORM_txaggr"
                                uci_set "wireless" "$device" "sgi" "$FORM_sgi"
                                if [ "$FORM_htmode" = "HT20" ] ; then 
                                    uci_set "wireless" "$device" "ht_capab" "TX-STBC RX-STBC1 DSSS_CCK-40"
                                else
                                    uci_set "wireless" "$device" "ht_capab" "SHORT-GI-40 TX-STBC RX-STBC1 DSSS_CCK-40"
                                fi
			fi
		}
	fi

	append forms_1 "start_form|Radio$radioindex Phy Configuration" "$N"
	#if [ "$iftype" = "broadcom" ]; then
	#	[ "$(uname -r | cut -d'.' -f2)" != "4" ] && uci_set "wireless" "$device" "type" "mac80211"
	#	append forms "helpitem|Broadcom Wireless Configuration" "$N"
	#	append forms "helptext|Helptext Broadcom Wireless Configuration#The router can be configured to handle multiple virtual interfaces which can be set to different modes and encryptions. Limitations are 1x sta, 0-3x ap or 1-4x ap or 1x  "$N"
	#elif [ "$iftype" = "atheros" ]; then
	#	ccSelect="0"
	#	[ -e /proc/sys/dev/$device/countrycode ] && ccSelect="$(cat /proc/sys/dev/$device/countrycode)"
	#	mode_country="field|@TR<<Country Code>>
	#	select|country_$device|$FORM_country
	#	option|0|@TR<<Default (or unset)>>$(equal "$ccSelect" '0' && echo ' ** CURRENT')
	#	option|840|@TR<<United States>>$(equal "$ccSelect" '840' && echo ' ** CURRENT')
	#	option|826|@TR<<UK and US 5.18-5.70GHz>>$(equal "$ccSelect" '826' && echo ' ** CURRENT')"
	#	append forms "$mode_country" "$N"
	#	append forms "helpitem|Atheros Wireless Configuration" "$N"
	#	append forms "helptext|Helptext Atheros Wireless Configuration#The router can be configured to handle multiple virtual interfaces which can be set to different modes and encryptions. Limitations are 1x sta, 0-4x ap or 1-4x ap or 1x mesh " "$N"
	#fi

	mode_disabled="field|@TR<<Radio>>
			radio|disabled_$device|$FORM_disabled|0|@TR<<On>>
			radio|disabled_$device|$FORM_disabled|1|@TR<<Off>>"
	append forms_1 "$mode_disabled" "$N"

	if [ "$iftype" = "mac80211" ]; then
        	mode_fields="field|@TR<<Mode>>
			select|ap_mode_$device|$FORM_ap_mode"

                [ "$PRODUCT" = "VIP421EXP49G" -o "$PRODUCT" = "VIP4214900X" -o "$PRODUCT" = "VIP4214900XEXP" -o "$PRODUCT" = "VIP4215800X" -o "$PRODUCT" = "VIP4215800XEXP" -o "$PRODUCT" = "VIP4212400X" -o "$PRODUCT" = "VIP4212400XEXP" ] && {
                    if [ "$cardtype" = "49G" ]; then
                        atype="4.9G Standard"
                        xatype="4.9G Ext-Standard"
                    else
                        atype="802.11A"
                        xatype="802.11AX"
                    fi
		    mode_fields="$mode_fields
			    option|11a|$atype
			    option|11ax|$xatype"
                } || {
                   if [ "$supported_protocol" = "11abgn" ]; then
			mode_fields="$mode_fields
                                option|11b|@TR<<802.11B ONLY>>
				option|11g|@TR<<802.11BG>>
				option|11ng|@TR<<802.11NG - High Throughput on 2.4GHz>>
                                option|11a|@TR<<802.11A ONLY>>
				option|11na|@TR<<802.11NA - High Throughput on 5GHz>>"
                   elif [ "$supported_protocol" = "11an" ]; then
       			mode_fields="$mode_fields
                               	option|11a|@TR<<802.11A ONLY>>
       				option|11na|@TR<<802.11NA - High Throughput on 5GHz>>"
                   elif [ "$supported_protocol" = "11bgn" ]; then
       			mode_fields="$mode_fields
                        	option|11b|@TR<<802.11B ONLY>>
       				option|11g|@TR<<802.11BG>>
       				option|11ng|@TR<<802.11NG - High Throughput on 2.4GHz>>"
                   elif [ "$supported_protocol" = "11abg" ]; then
       			mode_fields="$mode_fields
                        	option|11b|@TR<<802.11B ONLY>>
       				option|11g|@TR<<802.11BG>>
       				option|11a|@TR<<802.11A>>"
                    elif [ "$FORM_ap_mode" = "11a" ]; then
			mode_fields="$mode_fields
    				option|11a|@TR<<802.11A>>"
                    else
			mode_fields="$mode_fields
    				option|11b|@TR<<802.11B ONLY>>
    				option|11g|@TR<<802.11BG>>"
                    fi

                }
		append forms_2 "$mode_fields" "$N"
                
                ######### 802.11n #############################################
                htmode_fields="field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<High Throughput Mode>>|htmode_fields_${device}|hidden
			select|htmode_$device|$FORM_htmode
    			option|HT20|@TR<<HT20       >>
    			option|HT40-|@TR<<HT40 -     >>
    			option|HT40+|@TR<<HT40+     >>"
		append forms_2 "$htmode_fields" "$N"

		advanced_fields="field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<Advanced Capabilities>>|advanced_fields_${device}|hidden
                	checkbox|show_advanced_${device}|$FORM_show_advanced|1
                	string|Show"
        	append forms "$advanced_fields" "$N"
			
		txaggr="field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<- MPDU Aggregation>>|txaggr_$device|hidden
			radio|txaggr_$device|$FORM_txaggr|1|@TR<<Enable>>
			radio|txaggr_$device|$FORM_txaggr|0|@TR<<Disable>>"
		append forms "$txaggr" "$N"

		sgi="field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<- Short GI>>|sgi_$device|hidden
			radio|sgi_$device|$FORM_sgi|1|@TR<<Enable>>
			radio|sgi_$device|$FORM_sgi|0|@TR<<Disable>>"
		append forms "$sgi" "$N"

		config_get FORM_ht_capab $device ht_capab
		ht_capab="field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<- HT Capabilities Info>>|ht_capab_$device|hidden
                caption|$FORM_ht_capab"                
		#text|ht_capab1_${device}|$FORM_ht_capab"

		append forms "$ht_capab" "$N"

		FORM_max_amsdu_len="$(iw phy |grep "Max AMSDU length"|uniq |cut -d' ' -f4)"
                #FORM_max_amsdu_len=3839
		max_amsdu_len="field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<- Maximum AMSDU (byte)>>|max_amsdu_len_fields_${device}|hidden
			caption|$FORM_max_amsdu_len"
			#text|max_amsdu_len_${device}|$FORM_max_amsdu_len"
		append forms "$max_amsdu_len" "$N"

                FORM_max_ampdu_len="$(iw phy |grep "Maximum RX AMPDU length"|uniq |cut -d' ' -f5)"
		#FORM_max_ampdu_len=65535
		max_ampdu_len="field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<- Maximum AMPDU (byte)>>|max_ampdu_len_fields_${device}|hidden
			caption|$FORM_max_ampdu_len"
			#text|max_ampdu_len_${device}|$FORM_max_ampdu_len"
		append forms "$max_ampdu_len" "$N"
                ######### 802.11n #############################################


                [ "$PRODUCT" = "VIP421EXP49G" -o "$PRODUCT" = "VIP4214900X" -o "$PRODUCT" = "VIP4214900XEXP" -o "$PRODUCT" = "VIP4215800X" -o "$PRODUCT" = "VIP4215800XEXP" -o "$PRODUCT" = "VIP4212400X" -o "$PRODUCT" = "VIP4212400XEXP" ] && {
                    if [ "$cardtype" = "49G" ]; then
                        xchan_list="/lib/mdf/xchannellist.49g"    
                        chan_list="/lib/mdf/channellist.49g"   
                        bwmode="field|@TR<<Channel BandWidth>>|xbwmode_field_${device}|hidden 
                            select|xbwmode_${device}|$FORM_bwmode
                            option|20mHz|@TR<<20MHz Normal Rate>>
                            option|5mHz|@TR<< 5MHz Quarter Rate>>
                            option|10mHz|@TR<<10MHz Half Rate>>
                            option|40mHz|@TR<<40MHz Turbo Rate>>"
                        append forms "$bwmode" "$N"
    
                        abwmode="field|@TR<<Channel BandWidth>>|abwmode_field_${device}|hidden 
                            select|abwmode_${device}|$FORM_bwmode
                            option|20mHz|@TR<<Auto>>"
                        append forms "$abwmode" "$N"

                    else
                        bwmode="field|@TR<<Channel BandWidth>> 
                            select|xbwmode_${device}|$FORM_bwmode
                            option|20mHz|@TR<<20MHz Normal Rate>>
                            option|5mHz|@TR<< 5MHz Quarter Rate>>
                            option|10mHz|@TR<<10MHz Half Rate>>
                            option|40mHz|@TR<<40MHz Turbo Rate>>"
                        append forms "$bwmode" "$N"
                        chan_list="/lib/mdf/channellist.58g"    
                        xchan_list="/lib/mdf/xchannellist.58g"    
                    fi

                    dev_minfreq=$(cat /lib/mdf/mdfcfg |grep ":${cardtype}=" |awk -F "=" '{print $2}'|awk -F "," '{printf $1}')
                    dev_maxfreq=$(cat /lib/mdf/mdfcfg |grep ":${cardtype}=" |awk -F "=" '{print $2}'|awk -F "," '{printf $2}')
                    std_dev_minfreq=$(cat /lib/mdf/mdfcfg |grep ":${cardtype}ST=" |awk -F "=" '{print $2}'|awk -F "," '{printf $1}')    
                    std_dev_maxfreq=$(cat /lib/mdf/mdfcfg |grep ":${cardtype}ST=" |awk -F "=" '{print $2}'|awk -F "," '{printf $2}')    

                    nor_std_minfreq=$(expr $std_dev_minfreq + 10000 )
                    nor_std_maxfreq=$(expr $std_dev_maxfreq - 10000 )
                    nor_minfreq=$(expr $dev_minfreq + 10000 )
                    nor_maxfreq=$(expr $dev_maxfreq - 10000 )
                    hal_minfreq=$(expr $dev_minfreq + 5000 )
                    hal_maxfreq=$(expr $dev_maxfreq - 5000 )
                    qua_minfreq=$(expr $dev_minfreq + 2500 )
                    qua_maxfreq=$(expr $dev_maxfreq - 2500 )
                    tur_minfreq=$(expr $dev_minfreq + 20000 )
                    tur_maxfreq=$(expr $dev_maxfreq - 20000 )

                    A_CHANNELS_NORMAL="field|@TR<<Normal Channel - Freq >>|achannelnormalform_$device|hidden
                                        select|achannelnormal_$device|$FORM_channel
                                        option|0|@TR<<Auto>>"

                    XA_CHANNELS_NORMAL="field|@TR<<Normal Channel - Freq >>|xachannelnormalform_$device|hidden
                                    select|xachannelnormal_$device|$FORM_channel
                                    option|0|@TR<<Auto>>"
                    XA_CHANNELS_HALF="field|@TR<<Half Channel - Freq >>|xachannelhalfform_$device|hidden
                                      select|xachannelhalf_$device|$FORM_channel
                                      option|0|@TR<<Auto>>"
                    XA_CHANNELS_QUAT="field|@TR<<Quarter Channel - Freq >>|xachannelquatform_$device|hidden
                                  select|xachannelquat_$device|$FORM_channel
                                  option|0|@TR<<Auto>>"
                    XA_CHANNELS_TUBO="field|@TR<<TURBO Channel - Freq >>|xachanneltuboform_$device|hidden
                                select|xachanneltubo_$device|$FORM_channel
                                option|0|@TR<<Auto>>"


                    cnt_chan=1
                    last_chan=$(cat "$chan_list"|tail -n 1 | sed '//s/=/-/g')
                    ch=$(cat "$chan_list" | head -n $cnt_chan |tail -n 1 | sed '//s/=/-/g')
                    while [ "$ch" != "$last_chan" ]; do
                        freq=$(echo "$ch" |cut -d "-" -f2 )
                        [ "$freq" -ge "$nor_std_minfreq" ] && [ "$freq" -le "$nor_std_maxfreq" ]  && {
                            chnum=$(echo "$ch" |cut -d "-" -f1  )
                            freq_d=$(echo $freq|awk '{print $1/1000000}')
                            A_CHANNELS_NORMAL="$A_CHANNELS_NORMAL
                                                option|$chnum|${chnum} - ${freq_d} GHz"
                        }
                        cnt_chan=$(expr $cnt_chan + 1 )
                        ch=$(cat "$chan_list" | head -n $cnt_chan |tail -n 1 | sed '//s/=/-/g')
                    done

                    cnt_chan=1
                    last_chan=$(cat "$xchan_list"|tail -n 1 |sed '//s/=/-/g')
                    ch=$(cat "$xchan_list" | head -n $cnt_chan |tail -n 1 |sed '//s/=/-/g' )
                    while [ "$ch" != "$last_chan" ]; do
                        chnum=$(echo "$ch" |cut -d "-" -f1  )
                        freq=$(echo "$ch" |cut -d "-" -f2 )
                        freq_d=$(echo $freq|awk '{print $1/1000000}')
                        if [ "$cardtype" = "49G" ]; then
                            chnum_d=$(echo $chnum|awk '{print $1*5}')
                        else
                            chnum_d=$chnum
                        fi

                        [ "$freq" -ge "$nor_minfreq" ] && [ "$freq" -le "$nor_maxfreq" ]  && {
                            XA_CHANNELS_NORMAL="$XA_CHANNELS_NORMAL
                                                option|$chnum|${chnum_d} - ${freq_d} GHz"
                        }

                        [ "$freq" -ge "$hal_minfreq" ] && [ "$freq" -le "$hal_maxfreq" ]  && {
                            XA_CHANNELS_HALF="$XA_CHANNELS_HALF
                                                option|$chnum|${chnum_d} - ${freq_d} GHz"
                        }

                        [ "$freq" -ge "$qua_minfreq" ] && [ "$freq" -le "$qua_maxfreq" ]  && {
                            XA_CHANNELS_QUAT="$XA_CHANNELS_QUAT
                                                option|$chnum|${chnum_d} - ${freq_d} GHz"
                        }

                        [ "$freq" -ge "$tur_minfreq" ] && [ "$freq" -le "$tur_maxfreq" ]  && {
                            XA_CHANNELS_TUBO="$XA_CHANNELS_TUBO
                                                option|$chnum|${chnum_d} - ${freq_d} GHz"
                        }

                        cnt_chan=$(expr $cnt_chan + 1 )
                        ch=$(cat "$xchan_list" | head -n $cnt_chan |tail -n 1 |sed '//s/=/-/g' )
                    done

               ##     append forms "$A_CHANNELS_NORMAL" "$N"
               ##     append forms "$XA_CHANNELS_NORMAL" "$N"
               ##     append forms "$XA_CHANNELS_HALF" "$N"
               ##     append forms "$XA_CHANNELS_QUAT" "$N"
               ##     append forms "$XA_CHANNELS_TUBO" "$N"

                } || {
                    if [ "$FORM_ap_mode" = "11na" -o "$FORM_ap_mode" = "11ng" ]; then
                        bwmode="field|@TR<<Channel BandWidth>>|bwmodebox_${device}
                            select|bwmode_${device}|$FORM_bwmode
                            option|20mHz|@TR<<20MHz Normal Rate>>"
                            #option|5mHz|@TR<< 5MHz Quarter Rate>>
                            #option|10mHz|@TR<<10MHz Half Rate>>
                            #option|40mHz|@TR<<40MHz Turbo Rate>>"
                        append forms "$bwmode" "$N"

                        CHANNELS="field|@TR<<Channel-Frequency>>|channelform_$device
             			select|channel_$device|$FORM_channel
             			option|0|@TR<<Auto>>"

                        if [ "$FORM_ap_mode" = "11ng" ]; then
                              case "$FORM_htmode" in
                                      HT20)	HT_CHANNELS=$NG_CHANNELS_HT20		;;
                                      HT40-) 	HT_CHANNELS=$NG_CHANNELS_HT40MINUS	;;
                                      HT40+)	HT_CHANNELS=$NG_CHANNELS_HT40PLUS	;;
                              esac                     
                            
                              for ch in $HT_CHANNELS; do
                                       eval freq=$(echo $ch|awk '{print (2407+$1*5)/1000}')
                              	        CHANNELS="$CHANNELS
                                                option|$ch|$ch - $freq GHz"
                               done
                              append forms "$CHANNELS" "$N"

                        elif [ "$FORM_ap_mode" = "11na" ]; then
                              case $FORM_htmode in
                                      HT20)	HT_CHANNELS=$NA_CHANNELS_HT20		;;
                                      HT40-)	HT_CHANNELS=$NA_CHANNELS_HT40MINUS	;;
                                      HT40+)	HT_CHANNELS=$NA_CHANNELS_HT40PLUS	;;
                              esac    
                            
                              for ch in $HT_CHANNELS; do
                                       eval freq=$(echo $ch|awk '{print (5000+$1*5)/1000}')
                              	        CHANNELS="$CHANNELS
                                                option|$ch|$ch - $freq GHz"
                               done
                              append forms "$CHANNELS" "$N"
                        fi
                        
                    elif [ "$FORM_ap_mode" = "11a" ]; then
                        bwmode="field|@TR<<Channel BandWidth>>|bwmodebox_${device}
                            select|bwmode_${device}|$FORM_bwmode
                            option|20mHz|@TR<<20MHz Normal Rate>>"
                            #option|5mHz|@TR<< 5MHz Quarter Rate>>
                            #option|10mHz|@TR<<10MHz Half Rate>>
                            #option|40mHz|@TR<<40MHz Turbo Rate>>"
                        append forms "$bwmode" "$N"

         	##	A_CHANNELS="field|@TR<<Channel-Freq>>|achannelform_$device
             	##		select|achannel_$device|$FORM_channel
             	##		option|0|@TR<<Auto>>"
                ##
             	##	for ch in $ACHANNELS; do
                ##                 eval freq=$(echo $ch|awk '{print (5000+$1*5)/1000}')
         	##		A_CHANNELS="$A_CHANNELS
                ##                          option|$ch|$ch - $freq GHz"
         	##	done
         	##	append forms "$A_CHANNELS" "$N"

                        CHANNELS="field|@TR<<Channel-Frequency>>|channelform_$device
             			select|channel_$device|$FORM_channel
             			option|0|@TR<<Auto>>"
             
             		for ch in $ACHANNELS; do
                                 eval freq=$(echo $ch|awk '{print (5000+$1*5)/1000}')
         			CHANNELS="$CHANNELS
                                          option|$ch|$ch - $freq GHz"
         		done
         		append forms "$CHANNELS" "$N"
                    else
                        bwmode="field|@TR<<Channel BandWidth>>|bwmodebox_${device}
                            select|bwmode_${device}|$FORM_bwmode
                            option|20mHz|@TR<<20MHz Normal Rate>>"
                            #option|5mHz|@TR<< 5MHz Quarter Rate>>
                            #option|10mHz|@TR<<10MHz Half Rate>>"
                        append forms "$bwmode" "$N"

         	  ##	BG_CHANNELS="field|@TR<<Channel-Freq>>|bgchannelform_$device|hidden
         	  ##		select|bgchannel_$device|$FORM_channel
         	  ##		option|0|@TR<<Auto>>"
         	  ##		
         	  ##	for ch in $BGCHANNELS; do
         	  ##		eval freq=$(echo $ch|awk '{print (2407+$1*5)/1000}')
         	  ##		BG_CHANNELS="$BG_CHANNELS
         	  ##			option|$ch|$ch - $freq GHz"
         	  ##	done
                  ##	append forms "$BG_CHANNELS" "$N"

                        CHANNELS="field|@TR<<Channel-Frequency>>|channelform_$device
         			select|channel_$device|$FORM_channel
         			option|0|@TR<<Auto>>"
         			
         		for ch in $BGCHANNELS; do
         			eval freq=$(echo $ch|awk '{print (2407+$1*5)/1000}')
         			CHANNELS="$CHANNELS
         				option|$ch|$ch - $freq GHz"
         		done
                  	append forms "$CHANNELS" "$N"

                  ##      std24minfreq_tub="2421"
                  ##      std24maxfreq_tub="2453"
         	  ##	TBG_CHANNELS="field|@TR<<Channel-Freq>>|tbgchannelform_$device|hidden
         	  ##		select|tbgchannel_$device|$FORM_channel
         	  ##		option|0|@TR<<Auto>>"
         	  ##		
         	  ##	for ch in $BGCHANNELS; do
         	  ##		eval efreq=$(echo $ch|awk '{print (2407+$1*5)}')
                  ##            [ "$efreq" -ge "$std24minfreq_tub" ] && [ "$efreq" -le "$std24maxfreq_tub" ]  && {
         	  ##		    eval freq=$(echo $ch|awk '{print (2407+$1*5)/1000}')
         	  ##		    TBG_CHANNELS="$TBG_CHANNELS
         	  ##			    option|$ch|$ch - $freq GHz"
                  ##              }
         	  ##	done
                  ##	#append forms "$TBG_CHANNELS" "$N"

                    fi
            }
	else
		BG_CHANNELS="field|@TR<<Channel>>|bgchannelform_$device
			select|bgchannel_$device|$FORM_channel
			option|0|@TR<<Auto>>"
		for ch in $BGCHANNELS; do
			BG_CHANNELS="$BG_CHANNELS
				option|$ch"
		done
         ##	append forms "$BG_CHANNELS" "$N"
	fi




	if [ "$iftype" = "atheros" ]; then
		if [ "$_device" != "NanoStation2" -a "$_device" != "NanoStation5" ]; then
			mode_diversity="field|@TR<<Diversity>>
					radio|diversity_$device|$FORM_diversity|1|@TR<<On>>
					radio|diversity_$device|$FORM_diversity|0|@TR<<Off>>"      	
			append forms "$mode_diversity" "$N"
			#append forms "helpitem|Diversity" "$N"
			#append forms "helptext|Helptext Diversity#Used on systems with multiple antennas to help improve reception. Disable if you only have one antenna." "$N"
			#append forms "helplink|http://madwifi-project.org/wiki/UserDocs/AntennaDiversity" "$N"

			form_txant="field|@TR<<TX Antenna>>
				radio|txantenna_$device|$FORM_txantenna|0|@TR<<Auto>>
				radio|txantenna_$device|$FORM_txantenna|1|@TR<<Antenna 1>>
				radio|txantenna_$device|$FORM_txantenna|2|@TR<<Antenna 2>>"
			append forms "$form_txant" "$N"

			form_rxant="field|@TR<<RX Antenna>>
				radio|rxantenna_$device|$FORM_rxantenna|0|@TR<<Auto>>
				radio|rxantenna_$device|$FORM_rxantenna|1|@TR<<Antenna 1>>
				radio|rxantenna_$device|$FORM_rxantenna|2|@TR<<Antenna 2>>"
			append forms "$form_rxant" "$N"
		else
			form_antenna_selection="field|@TR<<Antenna>>
					select|antenna_$device|$FORM_antenna
					option|auto|@TR<<Internal Auto>>
					option|horizontal|@TR<<Internal Horizontal>>
					option|vertical|@TR<<Internal Vertical>>
					option|external|@TR<<External>>"
			append forms "$form_antenna_selection" "$N"
		fi
	fi

	#Currently broadcom only.
	if [ "$iftype" = "broadcom" ]; then
	maxassoc="field|@TR<<Max Associated Clients (Default 128)>>
		text|maxassoc_${device}|$FORM_maxassoc"
	append forms "$maxassoc" "$N"
	fi

#	append forms "helpitem|Carrier BandWidth" "$N"
#	append forms "helptext|Helptext BandWidth#This is the 
#spectrum bandwidth occuppied. 5mHz:Quarter Channel; 10mHz:Half Channel; 
#20mHz:Full Channel; 40mHz:Turbo Channel" "$N"
	
	#if [ "$iftype" != "mac80211" ]; then
	distance="field|@TR<<Wireless Distance>>
		text|distance_${device}|$FORM_distance
                string|(m)"

	append forms "$distance" "$N"

        rts="field|@TR<<RTS Thr (256~2346)>>
                text|rts_${device}|$FORM_rts
                checkbox|rtsoff_${device}|$FORM_rtsoff|1
                string|OFF"
        append forms "$rts" "$N"

        frag="field|@TR<<Fragment Thr (256~2346)>>
                text|frag_${device}|$FORM_frag
                checkbox|fragoff_${device}|$FORM_fragoff|1
                string|OFF"
        append forms "$frag" "$N"

	#append forms "helpitem|Wireless Distance" "$N"
	#append forms "helptext|Helptext Wireless Distance#This is the distance of your longest link." "$N"
	#fi
	# mac80211 does not support multiple virtual interface
	add_vcfg="field|
		string|<a href=\"$SCRIPT_NAME?add_vcfg=$device&amp;add_vcfg_number=$vcfg_number\">@TR<<Add Virtual Interface>></a>"
	append forms "$add_vcfg" "$N"
	append forms "end_form" "$N"

	#####################################################################
	# This is looped for every virtual wireless interface (wifi-iface)
	#
        vcfgcnt=0
	for vcfg in $vface; do
		config_get FORM_device $vcfg device
		if [ "$FORM_device" = "$device" ]; then
			if empty "$FORM_submit"; then
				config_get FORM_network $vcfg network
				config_get FORM_mode $vcfg mode
				config_get FORM_ssid $vcfg ssid
				config_get FORM_mesh_id $vcfg mesh_id
				config_get FORM_bssid $vcfg bssid
				config_get FORM_encryption $vcfg encryption
				config_get FORM_key $vcfg key
                                config_get FORM_identity $vcfg identity
                                config_get FORM_password $vcfg password
				case "$FORM_key" in
					1|2|3|4) FORM_wep_key="$FORM_key"
						FORM_key="";;
				esac
				config_get FORM_key1 $vcfg key1
				config_get FORM_key2 $vcfg key2
				config_get FORM_key3 $vcfg key3
				config_get FORM_key4 $vcfg key4
				config_get FORM_server $vcfg server
				config_get FORM_radius_port $vcfg port
				config_get FORM_rate $vcfg rate
				config_get FORM_txpower $vcfg txpower
	        		config_get_bool FORM_hidden $vcfg hidden 0
	        		config_get_bool FORM_isolate $vcfg isolate 0
	        		#config_get_bool FORM_bgscan $vcfg bgscan 0
	        		config_get_bool FORM_wds $vcfg wds 0
				#config_get_bool FORM_doth "$vcfg" 80211h
				#config_get_bool FORM_compression "$vcfg" compression
				#config_get_bool FORM_bursting "$vcfg" bursting
				#config_get_bool FORM_fframes "$vcfg" ff
				#config_get_bool FORM_wmm "$vcfg" wmm
				#config_get_bool FORM_xr "$vcfg" xr
				#config_get_bool FORM_ar "$vcfg" ar
				config_get_bool FORM_turbo "$vcfg" turbo
				config_get_bool FORM_bonding "$vcfg" bonding 0
				config_get FORM_macpolicy "$vcfg" macfilter
				config_get FORM_maclist "$vcfg" maclist
                                config_get FORM_wep_passphrase "$vcfg" wep_passphrase
				eval FORM_maclistremove="\$FORM_${vcfgcnt}_maclistremove"
				if [ "$FORM_maclistremove" != "" ]; then
					list_remove FORM_maclist "$FORM_maclistremove"
					uci_set "wireless" "$vcfg" "maclist" "$FORM_maclist"
				fi
			else
				eval FORM_key="\$FORM_radius_key_$vcfgcnt"
				eval FORM_radius_ipaddr="\$FORM_radius_ipaddr_$vcfgcnt"
                                eval FORM_password="\$FORM_radius_password_$vcfgcnt"
                                eval FORM_identity="\$FORM_radius_identity_$vcfgcnt"
				
				eval FORM_encryption="\$FORM_encryption_$vcfgcnt"
				case "$FORM_encryption" in
					psk|psk2|psk-mixe*|psk+psk2) eval FORM_key="\$FORM_wpa_psk_$vcfgcnt";;
					wpa|wpa2|wpa+wpa2) eval FORM_key="\$FORM_radius_key_$vcfgcnt";;
				esac
				eval FORM_mode="\$FORM_mode_$vcfgcnt"
                                # Defaulted to AP mode in multi-SSID scenario
                                empty "$FORM_mode" && FORM_mode="ap"
				eval FORM_server="\$FORM_server_$vcfgcnt"
				eval FORM_radius_port="\$FORM_radius_port_$vcfgcnt"
				eval FORM_hidden="\$FORM_broadcast_$vcfgcnt"
				eval FORM_isolate="\$FORM_isolate_$vcfgcnt"
				eval FORM_wep_key="\$FORM_wep_key_$vcfgcnt"
				eval FORM_key1="\$FORM_key1_$vcfgcnt"
				eval FORM_key2="\$FORM_key2_$vcfgcnt"
				eval FORM_key3="\$FORM_key3_$vcfgcnt"
				eval FORM_key4="\$FORM_key4_$vcfgcnt"
				eval FORM_broadcast="\$FORM_broadcast_$vcfgcnt"
				eval FORM_ssid="\$FORM_ssid_$vcfgcnt"
				eval FORM_mesh_id="\$FORM_mesh_id_$vcfgcnt"
				eval FORM_wds="\$FORM_wds_$vcfgcnt"
				eval FORM_bonding="\$FORM_bonding_$vcfgcnt"
				eval FORM_bssid="\$FORM_bssid_$vcfgcnt"
				eval FORM_network="\$FORM_network_$vcfgcnt"
                                [ "$PRODUCT" = "VIP421EXP49G" -o "$PRODUCT" = "VIP4214900X" -o "$PRODUCT" = "VIP4214900XEXP" -o "$PRODUCT" = "VIP4215800X" -o "$PRODUCT" = "VIP4215800XEXP" -o "$PRODUCT" = "VIP4212400X" -o "$PRODUCT" = "VIP4212400XEXP" ] && {
                                    if [ "$FORM_ap_mode" = "11ax" ];then 
                                        if [ "$FORM_bwmode" = "20mHz" ];then
                                            eval FORM_txpower="\$FORM_normal_txpower_$vcfgcnt"
                                        elif [ "$FORM_bwmode" = "10mHz" ];then
                                            eval FORM_txpower="\$FORM_half_txpower_$vcfgcnt"
                                        elif [ "$FORM_bwmode" = "5mHz" ];then
                                            eval FORM_txpower="\$FORM_quat_txpower_$vcfgcnt"
                                        elif [ "$FORM_bwmode" = "40mHz" ];then
                                            eval FORM_txpower="\$FORM_tubo_txpower_$vcfgcnt"
                                        fi
                                    else
                                        eval FORM_txpower="\$FORM_normal_txpower_$vcfgcnt"
                                    fi

				} || {
                                    eval FORM_txpower="\$FORM_txpower_$vcfgcnt"
                                }
                                if [ "$FORM_ap_mode" = "11b" ];then 
                                    eval FORM_rate="\$FORM_brate_$vcfgcnt"
                                else
                                    eval FORM_rate="\$FORM_rate_$vcfgcnt"
                                fi
                                eval FORM_rate="\$FORM_abgnrate_$vcfgcnt"
                                
				#eval FORM_bgscan="\$FORM_bgscan_$vcfg"
				#eval FORM_doth="\$FORM_doth_$vcfg"
				#eval FORM_compression="\$FORM_compression_$vcfg"
				#eval FORM_bursting="\$FORM_bursting_$vcfg"
				#eval FORM_fframes="\$FORM_fframes_$vcfg"
				#eval FORM_wmm="\$FORM_wmm_$vcfg"
				#eval FORM_xr="\$FORM_xr_$vcfg"
				#eval FORM_ar="\$FORM_ar_$vcfg"
				eval FORM_turbo="\$FORM_turbo_$vcfgcnt"
				eval FORM_macpolicy="\$FORM_macpolicy_$vcfgcnt"
				eval FORM_maclistadd="\$FORM_${vcfgcnt}_maclistadd"
                                eval FORM_wep_passphrase="\$FORM_wep_passphrase_$vcfgcnt"
				config_get FORM_maclist "$vcfg" maclist
				[ $FORM_maclistadd != "" ] && FORM_maclist="$FORM_maclist $FORM_maclistadd"
				empty "$FORM_generate_wep_128" && empty "$FORM_generate_wep_40" &&
				{
					case "$FORM_encryption" in
						psk|psk2|psk+psk2|*mixed*) append validate_forms "wpapsk|FORM_wpa_psk_$vcfgcnt|@TR<<WPA PSK#WPA Pre-Shared Key (min needs 8 characters) >>|min=8 max=63 required|$FORM_key" "$N";;
						wpa|wpa2|wpa+wpa2)
                                                        if [ "$FORM_mode" = "ap" ];then  
                                                        append validate_forms "string|FORM_radius_key_$vcfgcnt|@TR<<RADIUS Server Key>>|min=4 max=63 required|$FORM_key" "$N"
							append validate_forms "ip|FORM_server_$vcfgcnt|@TR<<RADIUS IP Address>>|required|$FORM_server" "$N"
							append validate_forms "port|FORM_radius_port_$vcfgcnt|@TR<<RADIUS Port>>|required|$FORM_radius_port" "$N"
                                                        fi
                                                        ;;
						wep)
							append validate_forms "int|FORM_wep_key_$vcfgcnt|@TR<<Selected WEP Key>>|min=1 max=4|$FORM_wep_key" "$N"
							append validate_forms "wep|FORM_key1_$vcfgcnt|@TR<<WEP Key>> 1||$FORM_key1" "$N"
							append validate_forms "wep|FORM_key2_$vcfgcnt|@TR<<WEP Key>> 2||$FORM_key2" "$N"
							append validate_forms "wep|FORM_key3_$vcfgcnt|@TR<<WEP Key>> 3||$FORM_key3" "$N"
							append validate_forms "wep|FORM_key4_$vcfgcnt|@TR<<WEP Key>> 4||$FORM_key4" "$N";;
					esac
					case "$FORM_mode" in
						wds)
							append validate_forms "string|FORM_ssid_$vcfgcnt|@TR<<ESSID>>||$FORM_ssid" "$N"
							append validate_forms "mac|FORM_bssid_$vcfgcnt|@TR<<BSSID>>||$FORM_bssid" "$N";;
						mesh)
							append validate_forms "string|FORM_mesh_id_$vcfgcnt|@TR<<MESH ID>>|required|$FORM_mesh_id" "$N";; 
						*)
							append validate_forms "string|FORM_ssid_$vcfgcnt|@TR<<ESSID>>|required|$FORM_ssid" "$N";;
					esac
					SAVED=1
validate <<EOF
$validate_forms
mac|FORM_maclistadd|@TR<<Mac List>>||$FORM_maclistadd
EOF
					if [ "$?" = 0 ]; then
						uci_set "wireless" "$vcfg" "network" "$FORM_network"
						case "$FORM_mode" in
      						   mesh)
						     # uci_remove "wireless" "$vcfg" "ssid"
						      uci_set "wireless" "$vcfg" "mesh_id" "$FORM_mesh_id" 
						      ;;
						   *)
						      uci_set "wireless" "$vcfg" "ssid" "$FORM_ssid"
						   #   uci_remove "wireless" "$vcfg" "mesh_id"
                                                      ;;
						esac
						FORM_bssid="`echo "$FORM_bssid" |tr "[A-Z]" "[a-z]"`" 
						#uci_set "wireless" "$vcfg" "bssid" "$FORM_bssid"
						uci_set "wireless" "$vcfg" "mode" "$FORM_mode"
						uci_set "wireless" "$vcfg" "encryption" "$FORM_encryption"
						uci_set "wireless" "$vcfg" "server" "$FORM_server"
						uci_set "wireless" "$vcfg" "port" "$FORM_radius_port"
						uci_set "wireless" "$vcfg" "hidden" "$FORM_hidden"
						uci_set "wireless" "$vcfg" "isolate" "$FORM_isolate"
						uci_set "wireless" "$vcfg" "txpower" "$FORM_txpower"
						#if [ "$iftype" = "mac80211" ]; then 
						#   uci_set "wireless" "$vcfg" "txpower" "17" 
						#fi
                                                uci_set "wireless" "$vcfg" "rate" "$FORM_rate" 
                                                #uci_set "wireless" "$vcfg" "bgscan" "$FORM_bgscan"
						uci_set "wireless" "$vcfg" "wds" "$FORM_wds"

						case "$FORM_encryption" in
							wep) uci_set "wireless" "$vcfg" "key" "$FORM_wep_key";;
							psk|psk2|psk+psk2|*mixed*) uci_set "wireless" "$vcfg" "key" "$FORM_key";;
							wpa|wpa2|wpa+wpa2) uci_set "wireless" "$vcfg" "key" "$FORM_key";;
						esac
                                                uci_set "wireless" "$vcfg" "identity" "$FORM_identity";
                                                uci_set "wireless" "$vcfg" "password" "$FORM_password";
						uci_set "wireless" "$vcfg" "key1" "$FORM_key1"
						uci_set "wireless" "$vcfg" "key2" "$FORM_key2"
						uci_set "wireless" "$vcfg" "key3" "$FORM_key3"
						uci_set "wireless" "$vcfg" "key4" "$FORM_key4"
						#uci_set "wireless" "$vcfg" "80211h" "$FORM_doth"
						#uci_set "wireless" "$vcfg" "compression" "$FORM_compression"
						#uci_set "wireless" "$vcfg" "bursting" "$FORM_bursting"
						#uci_set "wireless" "$vcfg" "ff" "$FORM_fframes"
						#uci_set "wireless" "$vcfg" "wmm" "$FORM_wmm"
						#uci_set "wireless" "$vcfg" "xr" "$FORM_xr"
						#uci_set "wireless" "$vcfg" "ar" "$FORM_ar"
						uci_set "wireless" "$vcfg" "bonding" "$FORM_bonding"
						uci_set "wireless" "$vcfg" "turbo" "$FORM_turbo"
						uci_set "wireless" "$vcfg" "macfilter" "$FORM_macpolicy"
						uci_set "wireless" "$vcfg" "maclist" "$FORM_maclist"
                                                uci_set "wireless" "$vcfg" "wep_passphrase" "$FORM_wep_passphrase"
					fi
				}
			fi

			case "$FORM_mode" in
				ap) let "ap_count+=1";;
				sta) let "sta_count+=1";;
				mesh) let "mesh_count+=1";;
			esac

			append forms "start_form|Radio$radioindex Virtual Interface" "$N"
                        let "count+=1"
			network="field|@TR<<Network>>|network_field_$vcfgcnt
	        	        select|network_$vcfgcnt|$FORM_network
	        	        $network_options"
			append forms "$network" "$N"

			if [ "$iftype" != "mac80211" ]; then
			option_wds="option|wds|@TR<<WDS>>"
				#append forms "helpitem|WDS Connections" "$N"
				#append forms "helptext|Helptext WDS Connections#Enter the MAC address of the router on your network that should be wirelessly connected to. The other router must also support wds and have the mac address of this router entered." "$N"
			fi

			if [ "$vcfgcnt" -lt "1" ]; then
				mode_fields="field|@TR<<WLAN Mode#Mode>>|mode_field_$vcfgcnt
				select|mode_$vcfgcnt|$FORM_mode
				option|ap|@TR<<Access Point>>
				$option_wds
				option|sta|@TR<<Client>>
				option|repeater|@TR<<Repeater>>
				option|mesh|@TR<<Mesh Point>>"
			else
				mode_fields="field|@TR<<WLAN Mode#Mode>>|mode_field_$vcfgcnt|hidden
				select|mode_$vcfgcnt|$FORM_mode
				option|ap|@TR<<Access Point>>
				$option_wds
				option|sta|@TR<<Client>>
				option|repeater|@TR<<Repeater>>
				option|mesh|@TR<<Mesh Point>>"
			fi	
			append forms "$mode_fields" "$N"

                        [ "$PRODUCT" = "VIP421EXP49G" -o "$PRODUCT" = "VIP4214900X" -o "$PRODUCT" = "VIP4214900XEXP" -o "$PRODUCT" = "VIP4215800X" -o "$PRODUCT" = "VIP4215800XEXP" -o "$PRODUCT" = "VIP4212400X" -o "$PRODUCT" = "VIP4212400XEXP" ] && {
   			    rate_list="6M 9M 12M 18M 24M 36M 48M 54M"
			} || {
                            if [ "$FORM_ap_mode" = "11a" ]; then
   			        rate_list="6M 9M 12M 18M 24M 36M 48M 54M"
                            else
                                rate_list="1M 2M 5.5M 11M 6M 9M 12M 18M 24M 36M 48M 54M"
			        brate_list="1M 2M 5.5M 11M" 
			    fi
                        }

			rate_field="field|@TR<<TX Rate>>|rate_field_$vcfgcnt|hidden
			select|rate_$vcfgcnt|$FORM_rate
			option|auto|@TR<<Auto>>"
			for rate in $rate_list; do
			rate_field="$rate_field
                        option|$rate|@TR<<$rate>>"
			done

			#append forms "$rate_field" "$N"

			brate_field="field|@TR<<TX Rate>>|brate_field_$vcfgcnt|hidden
			select|brate_$vcfgcnt|$FORM_rate
			option|auto|@TR<<Auto>>"
			for rate in $brate_list; do
			    brate_field="$brate_field
                            option|$rate|@TR<<$rate>>"
			done
                     
                        if [ "$FORM_ap_mode" = "11a" ]; then
                            abgnrate_list="6M 9M 12M 18M 24M 36M 48M 54M"
                        elif [ "$FORM_ap_mode" = "11b" ]; then
                            abgnrate_list="1M 2M 5.5M 11M"
                        elif [ "$FORM_ap_mode" = "11g" ]; then
                            abgnrate_list="1M 2M 5.5M 11M 6M 9M 12M 18M 24M 36M 48M 54M"
                        elif [ "$FORM_ap_mode" = "11ng" -o "$FORM_ap_mode" = "11na" ]; then
                            # MCS index here is only from 0 - 15, furture can extend by standard 802.11n-2009
			    #if [ "$FORM_htmode" = 'HT20' ]; then 
                            	abgnrate_list="0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15"
                            #elif [ "$FORM_htmode" = "HT40+" -o "$FORM_htmode" = "HT40-" ]; then
                            #	abgnrate_list="0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87"
			    #fi
                        fi

                        abgnrate_field="field|@TR<<TX bitrate>>|abgnrate_field_$vcfgcnt
			select|abgnrate_$vcfgcnt|$FORM_rate
			option|auto|@TR<<Auto>>"
			for rate in $abgnrate_list; do
                            if [ "$FORM_ap_mode" = "11ng" -o "$FORM_ap_mode" = "11na" ]; then
				#if [ $rate -lt 16 ]; then
					rate_option="mcs-$rate"
				#fi 
				#if [ $rate -gt 47 ] && [ $rate -lt 64 ]; then
				#	rate_40=$(expr ${rate} - 48)
				#	rate_option="mcs-$rate_40 40Mhz"
				#fi 
				#if [ $rate -gt 71 ] && [ $rate -lt 88 ]; then
				#	rate_40_short_GI=$(expr ${rate} - 72)
				#	rate_option="mcs-$rate_40_short_GI 40Mhz Short GI"
				#fi 
			    	abgnrate_field="$abgnrate_field
                            	option|$rate|@TR<<$rate_option>>"
			    else #show legacy bitrate
			    	abgnrate_field="$abgnrate_field
                            	option|$rate|@TR<<$rate>>"
			    fi
			done

			append forms "$abgnrate_field" "$N"

                        [ "$PRODUCT" = "VIP421EXP49G" -o "$PRODUCT" = "VIP4214900X" -o "$PRODUCT" = "VIP4214900XEXP" -o "$PRODUCT" = "VIP4215800X" -o "$PRODUCT" = "VIP4215800XEXP" -o "$PRODUCT" = "VIP4212400X" -o "$PRODUCT" = "VIP4212400XEXP" ] && {
                            txpowers='11 12 13 14 15 16 17 18 19 20 21 22 23'
                            quat_txpower_field="field|@TR<<Tx Power>>|quat_txpower_$vcfgcnt|hidden
                                select|quat_txpower_$vcfgcnt|$FORM_txpower"
                            for txpower in $txpowers; do
                                quat_txpower_field="$quat_txpower_field
                                option|$txpower|$txpower dbm"
                            done
                       	    append forms "$quat_txpower_field" "$N"

                            txpowers='11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29'
                            half_txpower_field="field|@TR<<Tx Power>>|half_txpower_$vcfgcnt|hidden
                                select|half_txpower_$vcfgcnt|$FORM_txpower"
                            for txpower in $txpowers; do
                                half_txpower_field="$half_txpower_field
                                option|$txpower|$txpower dbm"
                            done
                            append forms "$half_txpower_field" "$N"

                            txpowers='11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30'
                            normal_txpower_field="field|@TR<<Tx Power>>|normal_txpower_$vcfgcnt|hidden
                                select|normal_txpower_$vcfgcnt|$FORM_txpower"
                            for txpower in $txpowers; do
                                normal_txpower_field="$normal_txpower_field
                                option|$txpower|$txpower dbm"
                            done
                       	    append forms "$normal_txpower_field" "$N"

                            txpowers='11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30'
                            tubo_txpower_field="field|@TR<<Tx Power>>|tubo_txpower_$vcfgcnt|hidden
                                select|tubo_txpower_$vcfgcnt|$FORM_txpower"
                            for txpower in $txpowers; do
                                tubo_txpower_field="$tubo_txpower_field
                                option|$txpower|$txpower dbm"
                            done
                       	    append forms "$tubo_txpower_field" "$N"

                        } || {
                            txpowers='11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30'
                            txpower_field="field|@TR<<Tx Power>>
                            select|txpower_$vcfgcnt|$FORM_txpower"
                            for txpower in $txpowers; do
                            txpower_field="$txpower_field
                            option|$txpower|$txpower dbm"
                            done
                       	    append forms "$txpower_field" "$N"
                        }

			wds="field|@TR<<WDS>>|wds_form_$vcfgcnt|hidden
				radio|wds_$vcfgcnt|$FORM_wds|1|@TR<<On>>
				radio|wds_$vcfgcnt|$FORM_wds|0|@TR<<Off>>"
			append forms "$wds" "$N"

			hidden="field|@TR<<ESSID Broadcast>>|broadcast_form_$vcfgcnt|hidden
				radio|broadcast_$vcfgcnt|$FORM_hidden|0|@TR<<On>>
				radio|broadcast_$vcfgcnt|$FORM_hidden|1|@TR<<Off>>"
			append forms "$hidden" "$N"

			#bgscan_field="field|@TR<<Background Client Scanning>>|bgscan_form_$vcfg|hidden
			#		radio|bgscan_$vcfg|$FORM_bgscan|1|@TR<<On>>
			#		radio|bgscan_$vcfg|$FORM_bgscan|0|@TR<<Off>>"
			#append forms "$bgscan_field" "$N"
			#append forms "helpitem|Background Client Scanning" "$N"

			isolate_field="field|@TR<<AP Isolation>>|isolate_form_$vcfgcnt|hidden
					radio|isolate_$vcfgcnt|$FORM_isolate|1|@TR<<On>>
					radio|isolate_$vcfgcnt|$FORM_isolate|0|@TR<<Off>>"
			append forms "$isolate_field" "$N"

		        bonding="field|@TR<<Interface Bonding>>|bonding_form_$vcfgcnt|hidden
                	radio|bonding_$vcfgcnt|$FORM_bonding|1|@TR<<On>>
                    	radio|bonding_$vcfgcnt|$FORM_bonding|0|@TR<<Off>>"
            		append forms "$bonding" "$N"

			if [ "$iftype" = "atheros" ]; then
				#append forms "helptext|Helptext Backround Client Scanning#Enables or disables the ablility of a virtual interface to scan for other access points while in client mode. Disabling this allows for higher throughput but keeps your card from roaming to other access points with a higher signal strength." "$N"
				#append forms "helplink|http://madwifi-project.org/wiki/UserDocs/PerformanceTuning" "$N"
				#doth="field|@TR<<DFS/TPC>>
				#	radio|doth_$vcfg|$FORM_doth|1|@TR<<On>>
				#	radio|doth_$vcfg|$FORM_doth|0|@TR<<Off>>"
				#append forms "$doth" "$N"

				#compression="field|@TR<<Compression>>
				#	radio|compression_$vcfg|$FORM_compression|1|@TR<<On>>
				#	radio|compression_$vcfg|$FORM_compression|0|@TR<<Off>>"
				#append forms "$comp" "$N"

				#bursting="field|@TR<<Bursting>>
				#	radio|bursting_$vcfg|$FORM_bursting|1|@TR<<On>>
				#	radio|bursting_$vcfg|$FORM_bursting|0|@TR<<Off>>"
				#append forms "$bursting" "$N"

				#fframes="field|@TR<<Fast Frames>>
				#	radio|fframes_$vcfg|$FORM_fframes|1|@TR<<On>>
				#	radio|fframes_$vcfg|$FORM_fframes|0|@TR<<Off>>"
				#append forms "$ff" "$N"

				#wmm="field|@TR<<WMM>>
				#	radio|wmm_$vcfg|$FORM_wmm|1|@TR<<On>>
				#	radio|wmm_$vcfg|$FORM_wmm|0|@TR<<Off>>"
				#append forms "$wmm" "$N"
				if [ "$_device" != "NanoStation2" -a "$_device" != "NanoStation5" ]; then
				#	xr="field|@TR<<XR>>
				#		radio|xr_$vcfg|$FORM_xr|1|@TR<<On>>
				#		radio|xr_$vcfg|$FORM_xr|0|@TR<<Off>>"
				#	append forms "$xr" "$N"

				#	ar="field|@TR<<AR>>
				#		radio|ar_$vcfg|$FORM_ar|1|@TR<<On>>
				#		radio|ar_$vcfg|$FORM_ar|0|@TR<<Off>>"
				#	append forms "$ar" "$N"

					turbo="field|@TR<<Turbo>>
						radio|turbo_$vcfgcnt|$FORM_turbo|1|@TR<<On>>
						radio|turbo_$vcfgcnt|$FORM_turbo|0|@TR<<Off>>"
					append forms "$turbo" "$N"
				fi

				rate="field|@TR<<TX Rate>>
					select|rate_$vcfgcnt|$FORM_rate
					option|auto|@TR<<Auto>>
					option|1M|@TR<<1M>>
					option|2M|@TR<<2M>>
					option|5.5M|@TR<<5.5M>>
					option|11M|@TR<<11M>>
					option|6M|@TR<<6M>>
					option|12M|@TR<<12M>>
					option|18M|@TR<<18M>>
					option|24M|@TR<<24M>>
					option|36M|@TR<<36M>>
					option|54M|@TR<<54M>>"
				append forms "$rate" "$N"

				#eval txpowers="\$CONFIG_wireless_${device}_txpower"
				#[ -z "$txpowers" ] && {
				#	txpower=""
				#	for athname in $(ls /proc/sys/net/ 2>/dev/null | grep "^ath"); do
				#		[ "$(cat /proc/sys/net/${athname}/\%parent)" = "$device" ] && {
				#			for power in $(iwlist $athname txpower 2>&1 | sed '/dBm/!d /Current/d; s/^[[:space:]]*//;' | cut -d ' ' -f 1); do
				#				txpower="$txpower $power"
				#				FORM_txpower="$power"
				#			done
				#			break
				#		}
				#	done
				#	[ "$txpower" = "" ] && {
				#		athname=$(wlanconfig ath create wlandev $device wlanmode ap)
				#		for power in $(iwlist ath0 txpower 2>&1 | sed '/dBm/!d /Current/d; s/^[[:space:]]*//;' | cut -d ' ' -f 1); do
				#			txpower="$txpower $power"
				#			FORM_txpower="$power"
				#		done
				#		wlanconfig "$athname" destroy
				#	}
				#	[ "$txpower" != "" ] && {
				#		txpowers="$txpower"
				#       	config_set wireless "${device}_txpower" "$txpower"
				#		uci_set webif wireless "${device}_txpower" "$txpower"
				#	}
				#}
				#if [ "$txpowers" = "" ]; then
				#	txpowers='1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16'
				#	FORM_txpower="16"
				#fi
				#txpower_field="field|@TR<<Tx Power>>
				#		select|txpower_$vcfg|$FORM_txpower"
				#for txpower in $txpowers; do
				#	txpower_field="$txpower_field
				#			option|$txpower|$txpower dbm"
				#done
				#ppend forms "$txpower_field" "$N"
			fi

                        PRON_R=$(echo $PRODUCTNAME |grep IPnV)
                        [ -z $PRON_R ] && {
        			ssid="field|@TR<<SSID>>|ssid_form_$vcfgcnt
        				text|ssid_$vcfgcnt|$FORM_ssid"
        			append forms "$ssid" "$N"
                        }||{
        			ssid="field|@TR<<Network ID>>|ssid_form_$vcfgcnt
        				text|ssid_$vcfgcnt|$FORM_ssid"
        			append forms "$ssid" "$N"
                        }


			mesh_id="field|@TR<<MESH ID>>|mesh_id_form_$vcfgcnt
				text|mesh_id_$vcfgcnt|$FORM_mesh_id"
			append forms "$mesh_id" "$N"

			bssid="field|@TR<<BSSID>>|bssid_form_$vcfgcnt|hidden
				text|bssid_$vcfgcnt|$FORM_bssid"
			append forms "$bssid" "$N"

			###################################################################
			# Generate 4 40-bit WEP keys or 1 128-bit WEP Key
			#eval FORM_wep_passphrase="\$FORM_wep_passphrase_$vcfg"
			[ "$FORM_wep_passphrase" = "" ] && FORM_wep_passphrase="$(dd if=/dev/urandom count=200 bs=1 2>/dev/null|tr "\n" " "|sed 's/[^a-zA-Z0-9]//g')"
			eval FORM_generate_wep_128="\$FORM_generate_wep_128_$vcfgcnt"
			eval FORM_generate_wep_40="\$FORM_generate_wep_40_$vcfgcnt"
			! empty "$FORM_generate_wep_128" &&
			{
				FORM_wep_key="1"
				FORM_key1=""
				FORM_key2=""
				FORM_key3=""
				FORM_key4=""
				# generate a single 128(104)bit key
				if empty "$FORM_wep_passphrase"; then
					echo "<div class=warning>$EMPTY_passphrase_error</div>"
				else
					textkeys=$(wepkeygen -s "$FORM_wep_passphrase"  |
					 awk 'BEGIN { count=0 };
						{ total[count]=$1, count+=1; }
						END { print total[0] ":" total[1] ":" total[2] ":" total[3]}')
					FORM_key1=$(echo "$textkeys" | cut -d ':' -f 0-13 | sed s/':'//g)
					FORM_key2=$(echo "$textkeys" | cut -d ':' -f 0-13 | sed s/':'//g)
					FORM_key3=$(echo "$textkeys" | cut -d ':' -f 0-13 | sed s/':'//g)
					FORM_key4=$(echo "$textkeys" | cut -d ':' -f 0-13 | sed s/':'//g)
					FORM_encryption="wep"
				fi
			}

			! empty "$FORM_generate_wep_40" &&
			{
				FORM_wep_key="1"
				FORM_key1=""
				FORM_key2=""
				FORM_key3=""
				FORM_key4=""
				# generate a single 128(104)bit key
				if empty "$FORM_wep_passphrase"; then
					echo "<div class=warning>$EMPTY_passphrase_error</div>"
				else
					textkeys=$(wepkeygen "$FORM_wep_passphrase" | sed s/':'//g)
					keycount=1
					for curkey in $textkeys; do
					case $keycount in
						1) FORM_key1=$curkey;;
						2) FORM_key2=$curkey;;
						3) FORM_key3=$curkey;;
						4) FORM_key4=$curkey
							break;;
					esac
					let "keycount+=1"
					done
					FORM_encryption="wep"
				fi
			
			}
			
			if [ "$iftype" = "broadcom" ]; then
				psk_option="option|psk+psk2|WPA+WPA2 (@TR<<PSK>>)"
				wpa_option="option|wpa+wpa2|WPA+WPA2 (@TR<<RADIUS>>)"
			else
				psk_option="option|psk-mixed/tkip+aes|WPA+WPA2 (@TR<<PSK>>)"
                                wpa_option="option|wpa|WPA Enterprise (@TR<<RADIUS>>)
                                            option|wpa2|WPA2 Enterprise (@TR<<RADIUS>>)
                                            option|wpa+wpa2|WPA+WPA2 Enterprise (@TR<<RADIUS>>)" 
#                                wpa_option="option|wpa+tkip|WPA Enterprise (@TR<<RADIUS>>)
#                                            option|wpa2+aes|WPA2 Enterprise (@TR<<RADIUS>>)
#                                            option|mixed-wpa+tkip+aes|WPA+WPA2 Enterprise (@TR<<RADIUS>>)" 
			fi

			[ "$EXPORT" = "1" ] && { 
                            encryption_forms="field|@TR<<Encryption Type>>
				select|encryption_$vcfgcnt|$FORM_encryption
				option|none|@TR<<Disabled>>
				option|wep|WEP"
			    append forms "$encryption_forms" "$N"
                        } || {
                            encryption_forms="field|@TR<<Encryption Type>>
				select|encryption_$vcfgcnt|$FORM_encryption
				option|none|@TR<<Disabled>>
				option|wep|WEP
				option|psk|WPA (@TR<<PSK>>)
				option|psk2|WPA2 (@TR<<PSK>>)
				$psk_option
				$wpa_option"
			    append forms "$encryption_forms" "$N"
                        }

			[ "$EXPORT" = "1" ] && {
			    wep="field|@TR<<Passphrase>>|wep_keyphrase_$vcfgcnt|hidden
				text|wep_passphrase_$vcfgcnt|$FORM_wep_passphrase
				string|<br />
				field|&nbsp;|wep_generate_keys_$vcfgcnt|hidden
				submit|generate_wep_40_$vcfgcnt|@TR<<Generate 40bit Keys>>
				string|<br />
				field|@TR<<WEP Key 1>>|wep_key_1_$vcfgcnt|hidden
				radio|wep_key_$vcfgcnt|$FORM_wep_key|1
				text|key1_$vcfgcnt|$FORM_key1|<br />
				field|@TR<<WEP Key 2>>|wep_key_2_$vcfgcnt|hidden
				radio|wep_key_$vcfgcnt|$FORM_wep_key|2
				text|key2_$vcfgcnt|$FORM_key2|<br />
				field|@TR<<WEP Key 3>>|wep_key_3_$vcfgcnt|hidden
				radio|wep_key_$vcfgcnt|$FORM_wep_key|3
				text|key3_$vcfgcnt|$FORM_key3|<br />
				field|@TR<<WEP Key 4>>|wep_key_4_$vcfgcnt|hidden
				radio|wep_key_$vcfgcnt|$FORM_wep_key|4
				text|key4_$vcfgcnt|$FORM_key4|<br />"
			    append forms "$wep" "$N"
                        
                        } || { 
			    wep="field|@TR<<Passphrase>>|wep_keyphrase_$vcfgcnt|hidden
				text|wep_passphrase_$vcfgcnt|$FORM_wep_passphrase
				string|<br />
				field|&nbsp;|wep_generate_keys_$vcfgcnt|hidden
				submit|generate_wep_40_$vcfgcnt|@TR<<Generate 40bit Keys>>
				string|<br />
				submit|generate_wep_128_$vcfgcnt|@TR<<Generate 104bit Key>>
				string|<br />
				field|@TR<<WEP Key 1>>|wep_key_1_$vcfgcnt|hidden
				radio|wep_key_$vcfgcnt|$FORM_wep_key|1
				text|key1_$vcfgcnt|$FORM_key1|<br />
				field|@TR<<WEP Key 2>>|wep_key_2_$vcfgcnt|hidden
				radio|wep_key_$vcfgcnt|$FORM_wep_key|2
				text|key2_$vcfgcnt|$FORM_key2|<br />
				field|@TR<<WEP Key 3>>|wep_key_3_$vcfgcnt|hidden
				radio|wep_key_$vcfgcnt|$FORM_wep_key|3
				text|key3_$vcfgcnt|$FORM_key3|<br />
				field|@TR<<WEP Key 4>>|wep_key_4_$vcfgcnt|hidden
				radio|wep_key_$vcfgcnt|$FORM_wep_key|4
				text|key4_$vcfgcnt|$FORM_key4|<br />"
			    append forms "$wep" "$N"
                       }

			wpa="field|WPA @TR<<PSK>>|wpapsk_$vcfgcnt|hidden
				password|wpa_psk_$vcfgcnt|$FORM_key
                                field|@TR<<Show password>>|passwd_status_$vcfgcnt|hidden
			        checkbox|show_passwd_$vcfgcnt|$FORM_show_passwd|1
				field|@TR<<RADIUS IP Address>>|radius_ip_$vcfgcnt|hidden
				text|server_$vcfgcnt|$FORM_server
				field|@TR<<RADIUS Port>>|radius_port_form_$vcfgcnt|hidden
				text|radius_port_$vcfgcnt|$FORM_radius_port
				field|@TR<<RADIUS Server Key>>|radiuskey_$vcfgcnt|hidden
				text|radius_key_$vcfgcnt|$FORM_key

                                field|@TR<<RADIUS Identity>>|radiusidentity_$vcfgcnt|hidden
				text|radius_identity_$vcfgcnt|$FORM_identity
                                field|@TR<<RADIUS Password>>|radiuspassword_$vcfgcnt|hidden
				password|radius_password_$vcfgcnt|$FORM_password
                                field|@TR<<Show Radius Password>>|radiuspasswd_status_$vcfgcnt|hidden
			        checkbox|show_radiuspasswd_$vcfgcnt|$FORM_show_radiuspasswd|1"


			append forms "$wpa" "$N"
			
			if [ "$iftype" = "broadcom" ]; then
				install_nas_button="field|@TR<<NAS Package>>|install_nas_$vcfgcnt|hidden"
				if ! equal "$nas_installed" "1"; then
					install_nas_button="$install_nas_button
						string|<div class=\"warning\">WPA and WPA2 will not work until you install the NAS package. </div>
						submit|install_nas| Install NAS Package |"
				else
					install_nas_button="$install_nas_button
					string|@TR<<Installed>>."
				fi
				append forms "$install_nas_button" "$N"
			else
				install_hostapd_mini_button="field|@TR<<HostAPD Package>>|install_hostapd_mini_$vcfgcnt|hidden"
				if [ "$hostapd_installed" = "1" -o "$hostapd_mini_installed" = "1" ]; then
					install_hostapd_mini_button="$install_hostapd_mini_button
						string|@TR<<Installed>>."
				else
					install_hostapd_mini_button="$install_hostapd_mini_button
						string|<div class=\"warning\">PSK and PSK2 will not work until you install the HostAPD or HostAPD Mini package. (HostAPD Mini only does PSK and PSK2) </div>
						submit|install_hostapd_mini| Install HostAPD-Mini Package |
						submit|install_hostapd| Install HostAPD Package |"
				fi
				install_hostapd_button="field|@TR<<HostAPD Package>>|install_hostapd_$vcfgcnt|hidden"
				if [ "$hostapd_installed" != "1" ]; then
					install_hostapd_button="$install_hostapd_button
						string|<div class=\"warning\">WPA and WPA2 will not work until you install the HostAPD package. </div>
						submit|install_hostapd| Install HostAPD Package |"
				else
					install_hostapd_button="$install_hostapd_button
						string|@TR<<Installed>>."
				fi

				install_wpa_supplicant_button="field|@TR<<wpa-supplicant Package>>|install_wpa_supplicant_$vcfgcnt|hidden"
				if ! equal "$wpa_supplicant_installed" "1"; then
					install_wpa_supplicant_button="$install_wpa_supplicant_button
						string|<div class=\"warning\">WPA and WPA2 will not work until you install the wpa-supplicant package. </div>
						submit|install_wpa_supplicant| Install wpa-supplicant Package |"
				else
					install_wpa_supplicant_button="$install_wpa_supplicant_button
						string|@TR<<Installed>>."
				fi
				append forms "$install_hostapd_button" "$N"
				append forms "$install_hostapd_mini_button" "$N"
				append forms "$install_wpa_supplicant_button" "$N"
			fi
			
			#append forms "helpitem|Encryption Type" "$N"
			#append forms "helptext|HelpText Encryption Type#WPA (RADIUS) is only supported in Access Point mode. WPA (PSK) does not work in Ad-Hoc mode." "$N"

				macpolicy="field|@TR<<MAC Filter>>|macpolicy_form_$vcfgcnt|hidden
					select|macpolicy_$vcfgcnt|$FORM_macpolicy
					option|none|@TR<<Disabled>>
					option|allow|@TR<<Allow>>
					option|deny|@TR<<Deny>>"
				append forms "$macpolicy" "$N"

				maclist="end_form
					start_form|@TR<<MAC List>>|maclist_form_$vcfgcnt|hidden
					listedit|${vcfgcnt}_maclist|$SCRIPT_NAME?|$FORM_maclist
					end_form"
				append forms "$maclist" "$N"

			###################################################################
			# set JavaScript
			javascript_forms="
				v = isset('encryption_$vcfgcnt','wep');
				set_visible('wep_key_1_$vcfgcnt', v);
				set_visible('wep_key_2_$vcfgcnt', v);
				set_visible('wep_key_3_$vcfgcnt', v);
				set_visible('wep_key_4_$vcfgcnt', v);
				set_visible('wep_generate_keys_$vcfgcnt', v);
				set_visible('wep_keyphrase_$vcfgcnt', v);
				set_visible('wep_keys_$vcfgcnt', v);

		                v = isset('mode_$vcfgcnt','mesh');
				set_visible('mesh_id_form_$vcfgcnt', v);
				set_visible('ssid_form_$vcfgcnt', !v);
                                set_visible('bonding_form_$vcfgcnt', v);

                                // Force to AP mode in multi-SSID scenario
                                //alert($vcfgcnt)
                                if ($vcfgcnt>0){
                                    document.getElementById('mode_0').selectedIndex = 0; 
                                    document.getElementById('mode_0').options[0].selected = true; 
                                   //document.getElementById('mode_0').options[0].selected = 'selected';
                                    //document.getElementById('mode_0').selectedIndex = 0; 
                                    document.getElementById('mode_0').disabled = 1;
                                    //alert(document.getElementById('mode_0').selectedIndex);
                                }
                                else{
                                    document.getElementById('mode_0').disabled = 0;
                                }

				//
				// force encryption listbox to no selection if user tries
				// to set WPA (Radius) with anything but AP mode.
				//
				if (!isset('mode_$vcfgcnt','ap') && !isset('mode_$vcfgcnt','sta'))
				{
					if (isset('encryption_$vcfgcnt','wpa') || isset('encryption_$vcfgcnt','wpa2') || isset('encryption_$vcfgcnt','wpa+wpa2'))
				        { 
                                                alert('Radius Encryption not supported on Mesh or Repeater mode !');
						document.getElementById('encryption_$vcfgcnt').value = 'none'; 
					}
					set_visible('isolate_form_$vcfgcnt', 0);
				}
				else{
					set_visible('isolate_form_$vcfgcnt', 1);
				}
				//v = (isset('ap_mode_$device','11b') || isset('ap_mode_$device','11bg') || isset('ap_mode_$device','11g'));
				//set_visible('bgchannelform_$device', v); 

                                if (isset('ap_mode_$device','11ax'))
                                {
                                    v=isset('ap_mode_$device','11ax');
                                    set_visible('rate_field_$vcfgcnt', v);
                                    set_visible('achannelnormalform_$device', !v);
                                    set_visible('xbwmode_field_${device}', v);
                                    set_visible('abwmode_field_${device}', !v);
                                    v = (isset('xbwmode_$device','20mHz') && isset('ap_mode_$device','11ax'));
                                    set_visible('xachannelnormalform_$device', v);
                                    set_visible('normal_txpower_$vcfgcnt', v);    
                                    v = (isset('xbwmode_$device','10mHz') && isset('ap_mode_$device','11ax'));
                                    set_visible('xachannelhalfform_$device', v);
                                    set_visible('half_txpower_$vcfgcnt', v);
                                    v = (isset('xbwmode_$device','5mHz') && isset('ap_mode_$device','11ax'));
                                    set_visible('xachannelquatform_$device', v);
                                    set_visible('quat_txpower_$vcfgcnt', v);
                                    v = (isset('xbwmode_$device','40mHz') && isset('ap_mode_$device','11ax'));
                                    set_visible('xachanneltuboform_$device', v);
                                    set_visible('tubo_txpower_$vcfgcnt', v);
                                }
                                else
                                {
                                    v = isset('ap_mode_$device','11a');
                                    set_visible('rate_field_$vcfgcnt', v);
                                    set_visible('brate_field_$vcfgcnt', !v);
                                    set_visible('achannelnormalform_$device', v);
                                    set_visible('xbwmode_field_${device}', !v);
                                    set_visible('abwmode_field_${device}', v);
                                    set_visible('xachannelnormalform_$device', !v);
                                    set_visible('xachannelhalfform_$device', !v);
                                    set_visible('xachanneltuboform_$device', !v);
                                    set_visible('xachannelquatform_$device', !v);
                                    set_visible('normal_txpower_$vcfgcnt', v);
                                    set_visible('quat_txpower_$vcfgcnt', !v);
                                    set_visible('half_txpower_$vcfgcnt', !v);
                                    set_visible('tubo_txpower_$vcfgcnt', !v);    
                                }

                                if ((isset('ap_mode_$device','11b'))||(isset('ap_mode_$device','11g')))
                                {

                                    v = isset('ap_mode_$device','11b');
                                    set_visible('rate_field_$vcfgcnt', !v);
                                    set_visible('brate_field_$vcfgcnt', v);

                                    v = isset('bwmode_${device}','40mHz');                                
                                    set_visible('bgchannelform_$device', !v);
                                    set_visible('tbgchannelform_$device', v);
                                }
                                
                               //////////// 802.11n ////////////////////////////////////////////////////////////
                               if ((isset('ap_mode_$device','11na'))||(isset('ap_mode_$device','11ng')))
                               {                                 
		                 set_visible('htmode_fields_${device}', 1);
                                 set_visible('advanced_fields_$device', 1);
                               }else{
		                 set_visible('htmode_fields_${device}', 0);
                                 set_visible('advanced_fields_$device', 0);
                                 document.getElementById('show_advanced_${device}_1').checked = false;
		                }
                       
                               if( document.getElementById('show_advanced_${device}_1').checked)
                               {
		                    set_visible('ht_capab_${device}', 1);
		                    set_visible('txaggr_${device}', 1);
		                    set_visible('sgi_${device}', 1);
                                    set_visible('max_amsdu_len_fields_${device}', 1);
		                    set_visible('max_ampdu_len_fields_${device}', 1);		                    
                               }
                               else
                               {
		                    set_visible('ht_capab_${device}', 0);
		                    set_visible('txaggr_${device}', 0);
		                    set_visible('sgi_${device}', 0);
                                    set_visible('max_amsdu_len_fields_${device}', 0);
		                    set_visible('max_ampdu_len_fields_${device}', 0);		                    
                               }

                               //////////////// check txpower ////////////////////////////////////////
                                  var txpowerbox = document.getElementById('txpower_$vcfgcnt'); 
                                 // var txpowerbox = document.getElementById('txpower_0');
                                 // alert('hello'); 
                                 // alert(txpowerbox.name);
                                 // alert(txpowerbox.selectedIndex);
                                 // alert(txpowerbox.options[txpowerbox.selectedIndex].value);
                                 var chnbox = document.getElementById('channel_${device}');
                                 var chn=chnbox.options[chnbox.selectedIndex].value;
                                 //alert('current channel ' + chn);

                                 if( chn >= 1 && chn <= 11 ){
                                        var max_power=27; 
                                 }else if( chn >= 36 && chn <= 48 ){
                                        var max_power=17; 
                                 }else if ( chn >= 52 && chn <= 140 ){
                                        var max_power=20; 
                                 }else if ( chn >= 149 && chn <= 165 ){
                                        var max_power=30;
                                 }
                                 //alert('Channel ' + chn + ', Maximun txpower ' + max_power + ' dBm');                                                                   

                                  if (txpowerbox.options[txpowerbox.selectedIndex].value > max_power){
                                        //alert(txpowerbox.options[txpowerbox.selectedIndex].value + 'dBm. Tx Power Too Big!');
                                        alert('Channel ' + chn + ', Maximum allowed txpower ' + max_power + ' dBm');
                                        txpowerbox.selectedIndex = '6';   // 17dBm                                      
                                  }
                                      
                               //   txpowerbox.options.length = 0;                                 
                               //   var max_power=27;
                               //   var k=0;
                               //   for(k=6;k<=max_power;k++){    
                               //        txpowerbox.options[txpowerbox.options.length] = new Option(k + ' - dBm', k);
                               //   }
                         
                               ///////////////////////////////////////////////////////////////////////

				if (!isset('mode_$vcfgcnt','sta') && ('$iftype'=='mac80211')) 
				{ 
				    if (isset('channel_$device','0')) 
				    { 
				       // alert('hello why?');
				       document.getElementById('channel_$device').selectedIndex = '1'; 

				    }
				    if (isset('bgchannel_$device','0')) 
				    { 
                                       document.getElementById('bgchannel_$device').selectedIndex = '1'; 

				    }
				    if (isset('tbgchannel_$device','0')) 
				    { 
				    
				       document.getElementById('tbgchannel_$device').selectedIndex = '1'; 
				    }
				    if (isset('achannel_$device','0'))
				    { 
					     
				       document.getElementById('achannel_$device').selectedIndex = '1'; 
				    }

				    if (isset('achannelnormal_$device','0'))
				    { 
					     
				       document.getElementById('achannelnormal_$device').selectedIndex = '1'; 
				    }

				    if (isset('xachannelnormal_$device','0'))
				    { 
				       document.getElementById('xachannelnormal_$device').selectedIndex = '1'; 
				    }
				    if (isset('xachannelhalf_$device','0'))
				    { 
				       document.getElementById('xachannelhalf_$device').selectedIndex = '1'; 
				    }
				    if (isset('xachannelquat_$device','0'))
				    { 
				       document.getElementById('xachannelquat_$device').selectedIndex = '1'; 
				    }
				    if (isset('xachanneltubo_$device','0'))
				    { 
				       document.getElementById('xachanneltubo_$device').selectedIndex = '1'; 
				    }

				}

                                if ((isset('mode_$vcfgcnt','sta')) ||(isset('mode_$vcfgcnt','ap')) )
                                {
                                    set_visible('wds_form_$vcfgcnt', 1);
                                }
                                else
                                {
                                    set_visible('wds_form_$vcfgcnt', 0);
                                }
				v = (!isset('mode_$vcfgcnt','wds'));
				set_visible('broadcast_form_$vcfgcnt', v);
				//v = ((isset('mode_$vcfgcnt','wds')) || (isset('mode_$vcfgcnt','adhoc') && ('$iftype'=='mac80211')));
				//set_visible('bssid_form_$vcfgcnt', v);
				//v = (isset('mode_$vcfgcnt','sta'));
				//set_visible('bgscan_form_$vcfgcnt', v);
				//v = (isset('mode_$vcfgcnt','ap'));
				v = (isset('encryption_$vcfgcnt','psk') || isset('encryption_$vcfgcnt','psk2') || isset('encryption_$vcfgcnt','psk+psk2')  || isset('encryption_$vcfgcnt','psk-mixed/tkip+aes') );
				set_visible('wpapsk_$vcfgcnt', v);
                                set_visible('passwd_status_$vcfgcnt', v);
                                if( document.getElementById('show_passwd_${vcfgcnt}_1').checked)
                                {
                                     document.getElementById('wpa_psk_$vcfgcnt').type = 'text';
                                }
                                else
                                {
                                     document.getElementById('wpa_psk_$vcfgcnt').type = 'password';
                                }
                                if( document.getElementById('show_radiuspasswd_${vcfgcnt}_1').checked)
                                {
                                     document.getElementById('radius_password_$vcfgcnt').type = 'text';
                                }
                                else
                                {
                                     document.getElementById('radius_password_$vcfgcnt').type = 'password';
                                }

                                if( document.getElementById('rtsoff_${device}_1').checked)
                                {
                                     document.getElementById('rts_${device}').type = 'hidden';
                                }
                                else
                                {
                                     document.getElementById('rts_${device}').type = 'text';
                                }

                                if( document.getElementById('fragoff_${device}_1').checked)
                                {
                                     document.getElementById('frag_${device}').type = 'hidden';
                                }
                                else
                                {
                                     document.getElementById('frag_${device}').type = 'text';
                                }
				// show/hide bwmode box if it is in 11n
                            	v = ( isset('ap_mode_$device','11ng') || isset('ap_mode_$device','11na') );
                             	set_visible('bwmodebox_${device}', !v);

				v = (('$iftype'=='broadcom') && (isset('encryption_$vcfgcnt','psk') || isset('encryption_$vcfgcnt','psk2') || isset('encryption_$vcfgcnt','psk+psk2') || isset('encryption_$vcfgcnt','wpa') || isset('encryption_$vcfgcnt','wpa2') || isset('encryption_$vcfgcnt','wpa+wpa2')));
				set_visible('install_nas_$vcfgcnt', v);
				v = (('$iftype'!='broadcom') && (!isset('mode_$vcfgcnt','sta')) && (isset('encryption_$vcfgcnt','psk') || isset('encryption_$vcfgcnt','psk2')));
				// do not show install-package field
				set_visible('install_hostapd_mini_$vcfgcnt', 0);
				v = (('$iftype'!='broadcom') && (!isset('mode_$vcfgcnt','sta')) && (isset('encryption_$vcfgcnt','wpa') || isset('encryption_$vcfgcnt','wpa2')));
				// do not show install-package field
				set_visible('install_hostapd_$vcfgcnt', 0);
				v = (('$iftype'!='broadcom') && (isset('mode_$vcfgcnt','sta')) && (isset('encryption_$vcfgcnt','psk') || isset('encryption_$vcfgcnt','psk2') || isset('encryption_$vcfgcnt','wpa') || isset('encryption_$vcfgcnt','wpa2')));
				// do not show install-package field
				set_visible('install_wpa_supplicant_$vcfgcnt', 0);



        			v = (isset('encryption_$vcfgcnt','wpa') || isset('encryption_$vcfgcnt','wpa2') || isset('encryption_$vcfgcnt','wpa+wpa2'));
                                if  (isset('mode_$vcfgcnt','ap') && v) {
        			    set_visible('radiuskey_$vcfgcnt', v);
        			    set_visible('radius_ip_$vcfgcnt', v);
        			    set_visible('radius_port_form_$vcfgcnt', v);
                                    set_visible('radiusidentity_$vcfgcnt', !v);
                                    set_visible('radiuspassword_$vcfgcnt', !v);
                                    set_visible('radiuspasswd_status_$vcfgcnt', !v);
                                } else if (isset('mode_$vcfgcnt','sta') && v) {
                                    set_visible('radiuskey_$vcfgcnt', !v);
        			    set_visible('radius_ip_$vcfgcnt', !v);
        			    set_visible('radius_port_form_$vcfgcnt', !v);
                                    set_visible('radiusidentity_$vcfgcnt', v);
                                    set_visible('radiuspassword_$vcfgcnt', v);
                                    set_visible('radiuspasswd_status_$vcfgcnt', v);
                                } else {
                                    set_visible('radiuskey_$vcfgcnt', 0);
        			    set_visible('radius_ip_$vcfgcnt', 0);
        			    set_visible('radius_port_form_$vcfgcnt', 0);
                                    set_visible('radiusidentity_$vcfgcnt', 0);
                                    set_visible('radiuspassword_$vcfgcnt', 0);
                                    set_visible('radiuspasswd_status_$vcfgcnt', 0);
                                }


				//v = ( isset('mode_$vcfgcnt','ap') || isset('mode_$vcfgcnt','sta') );
				//set_visible('wds_form_$vcfgcnt', v);
                                //v = ( isset('mode_$vcfgcnt','mesh') );
                                //set_visible('bonding_form_$vcfgcnt', v);
				//v = (('$iftype'!='broadcom'));
				//set_visible('macpolicy_form_$vcfgcnt', v);
				//v = (!isset('macpolicy_$vcfgcnt','none'));
				//set_visible('maclist_form_$vcfgcnt', v);"
			append js "$javascript_forms" "$N"

			# mac80211 does not support multiple virtual interface
			if [ "$vcfgcnt" -gt "0" ]; then                        
   		        remove_vcfg="start_form
			    field|
			    string|<a href=\"$SCRIPT_NAME?remove_vcfg=$vcfg\">@TR<<Remove Virtual Interface>></a>"
        		append forms "$remove_vcfg" "$N"
			append forms "end_form" "$N"
                        fi
                        let "vcfgcnt+=1"
		fi
	done

	validate_wireless $iftype

header "Wireless" "$radioheader" "@TR<<Wireless Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"
#####################################################################
# modechange script
#
cat <<EOF
<script type="text/javascript" src="/webif.js"></script>
<script type="text/javascript">
<!--
function modechange()
{       //alert('${FORM_ht_capab}');
        //alert('${FORM_ap_mode}');
	var v;
        $js        
	hide('save');
	show('save');
}
-->
</script>
EOF

cat <<EOF
<script type="text/javascript">
<!--

function set_channel(){
    //alert('hello'); 
    var selbox1 = document.getElementById('ap_mode_${device}');
    var selbox2 = document.getElementById('htmode_${device}');

    var mymode = selbox1.options[selbox1.selectedIndex].value;    
    var myhtmode = selbox2.options[selbox2.selectedIndex].value;
    
    if (myhtmode == " ") {
            myhtmode = "HT20";
    }
    //alert(mymode);
    //alert(myhtmode);

    var selbox = document.getElementById('channel_${device}');    
    selbox.options.length = 0;

    if ((mymode == "11na") || (mymode == "11ng")) {
            document.getElementById('htmode_fields_${device}').style.display = '';
            document.getElementById('advanced_fields_$device').style.display = '';
    }else{            
            document.getElementById('htmode_fields_${device}').style.display = 'none';
            document.getElementById('advanced_fields_$device').style.display = 'none';
            document.getElementById('show_advanced_${device}_1').checked = false;
            document.getElementById('ht_capab_${device}').style.display = 'none';
            document.getElementById('txaggr_${device}').style.display = 'none';
            document.getElementById('sgi_${device}').style.display = 'none';
            document.getElementById('max_amsdu_len_fields_${device}').style.display = 'none';
            document.getElementById('max_ampdu_len_fields_${device}').style.display = 'none';
    }

    ////////// Dynamic options for channel-freq ///////////////////////////////////////////////////
    if (mymode == "11ng") {
                  if (myhtmode == 'HT20'){
                      var abgn_chn_list=new Array("1", "2", "3", "4","5", "6", "7", "8", "9", "10", "11");           
                      var chn_count=11;

                 }
                 if (myhtmode == 'HT40-'){
                      var abgn_chn_list=new Array("5", "6", "7", "8", "9", "10", "11");           
                      var chn_count=7;
                 }
                 if (myhtmode == 'HT40+'){
                      var abgn_chn_list=new Array("1", "2", "3", "4","5", "6", "7");           
                      var chn_count=7;
                 }
              var j=0;
              for(j=0;j<chn_count;j++){   
                      var freq=(2407+abgn_chn_list[j]*5)/1000;
                      var chn_freq=abgn_chn_list[j] + ' - ' + freq + ' GHz';
                      //alert(chn_freq);
                    selbox.options[selbox.options.length] = new Option(chn_freq, abgn_chn_list[j]);
              }
     }
     if (mymode == "11na") {
             if (myhtmode == 'HT20'){
                        var abgn_chn_list=new Array("149", "153", "157", "161", "165");           
                        var chn_count=5;
                 }
              if (myhtmode == 'HT40-'){ 
                        //var abgn_chn_list=new Array("40", "44", "48", "153", "157", "161", "165");           
                        var abgn_chn_list=new Array("153", "161");           
                        var chn_count=2;
                 }
              if (myhtmode == 'HT40+'){                     
                        //var abgn_chn_list=new Array("36", "40", "44", "149", "153", "157", "161");           
                        var abgn_chn_list=new Array("149", "157");           
                        var chn_count=2;                                          
                 }
              var j=0;
              for(j=0;j<chn_count;j++){   
                      var freq=(5000+abgn_chn_list[j]*5)/1000;
                      var chn_freq=abgn_chn_list[j] + ' - ' + freq + ' GHz'; 
                      //alert(chn_freq);
                    selbox.options[selbox.options.length] = new Option(chn_freq, abgn_chn_list[j]);
              }
    }
    if (mymode == "11a") {
              selbox.options[selbox.options.length] = new Option('Auto','36');        
              var abgn_chn_list=new Array("149", "153", "157", "161", "165");           
              var chn_count=5;
              var j=0;
              for(j=0;j<chn_count;j++){   
                      var freq=(5000+abgn_chn_list[j]*5)/1000;
                      var chn_freq=abgn_chn_list[j] + ' - ' + freq + ' GHz';
                      //alert(chn_freq);
                    selbox.options[selbox.options.length] = new Option(chn_freq, abgn_chn_list[j]);
              } 
    }
    if ((mymode == "11b")  || (mymode == "11g")){
              selbox.options[selbox.options.length] = new Option('Auto','1');      
              var abgn_chn_list=new Array("1", "2", "3", "4","5", "6", "7", "8", "9", "10", "11");           
              var chn_count=11;              
              var j=0;
              for(j=0;j<chn_count;j++){   
                      var freq=(2407+abgn_chn_list[j]*5)/1000;
                      var chn_freq=abgn_chn_list[j] + ' - ' + freq + ' GHz';
                      //alert(chn_freq);
                    selbox.options[selbox.options.length] = new Option(chn_freq, abgn_chn_list[j]);
              }  
    }


    //////////////// Dynamic options fot txrate for all SSIDs////////////////////////////////////////
       ////////////////////var txratebox = document.getElementById('abgnrate_${vcfgcnt}'); 
       //var txratebox = document.getElementById('abgnrate_0');
       //alert('hello1'); 
       //alert(txratebox.name);    
       //txratebox.options.length = 0;
       if ( mymode == "11a" ){
           var abgnrate_list=new Array("6M", "9M", "12M", "18M", "24M", "36M", "48M", "54M");           
           var count=8;
       }else if ( mymode == "11b" ){
           var abgnrate_list=new Array("1M", "2M", "5.5M", "11M");           
           var count=4;
       }else if ( mymode == "11g" ){
           var abgnrate_list=new Array("1M", "2M", "5.5M", "11M", "6M", "9M", "12M", "18M", "24M", "36M", "48M", "54M");           
           var count=12;
       }else if ( (mymode == "11ng") || (mymode == "11na") ){ 
/////////////// for now support mcs index 0 - 15, we can extend support for furture.//////////////////////////////          
//        if ( myhtmode == "HT20" ){
           	var abgnrate_list=new Array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15");
           	var count=16;
//	  }else {
//           	var abgnrate_list=new Array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", 
//  	 				"48", "49", "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "60", "61", "62", "63", 
//	 				"72", "73", "74", "75", "76", "77", "78", "79", "80", "81", "82", "83", "84", "85", "86", "87");
//           	var count=48;
//	  } 
       }
       //alert('hello2'); 
       var vifcnt = 0;
       for (vifcnt=0; vifcnt<$vcfgcnt; vifcnt++)
       {
       var txratebox = document.getElementById('abgnrate_'+vifcnt); 
       //var txratebox = document.getElementById('abgnrate_0');
       //alert('hello'); 
       //alert(txratebox.name);    
       txratebox.options.length = 0;
       txratebox.options[txratebox.options.length] = new Option('Auto','auto');
       var i=0;
       for(i=0;i<count;i++){    
            if ( (mymode == "11ng") || (mymode == "11na") ){  
		if (abgnrate_list[i] < 16){
			var rate_option = "mcs-" + abgnrate_list[i];
		} 
//		if (( abgnrate_list[i] > 47 ) && ( abgnrate_list[i] < 64 )){
//			var rate_40 = abgnrate_list[i] - 48;
//			var rate_option = "mcs-" + rate_40 + " 40Mhz";
//		}
//		if (( abgnrate_list[i] > 71 ) && (abgnrate_list[i] < 88 )){
//			var rate_40_short_GI = abgnrate_list[i] - 72;
//			var rate_option = "mcs-" + rate_40_short_GI + " 40Mhz Short GI";
//		} 
		//alert(rate_option);
            	txratebox.options[txratebox.options.length] = new Option( rate_option, abgnrate_list[i]);
	    }else{
		//alert('legacy-rate ' + abgnrate_list[i]);
            	txratebox.options[txratebox.options.length] = new Option(abgnrate_list[i], abgnrate_list[i]);
	    }
       }
       }
    //alert(myhtmode + ' ' + count);
    /////////////////////////////////////////////////////////////////////////////



    //////////// Hide/Show banwidth field /////////////////////////////////////////
    	var bwmodebox = document.getElementById('bwmodebox_${device}');
	//alert('show/hide'+' '+bwmodebox.id);
    	if ( (mymode == "11ng") || (mymode == "11na") ){           
		bwmodebox.style.display = 'none';
	}else{
            	bwmodebox.style.display = '';
	}

} // end of function

-->
</script>
EOF



display_form <<EOF
onchange|modechange
$validate_error
$forms_1
EOF


display_form <<EOF
onchange|set_channel
$validate_error
$forms_2
EOF


display_form <<EOF
onchange|modechange
$validate_error
$forms
EOF


footer ?>
<!--
##WEBIF:name:Wireless:400:Radio1
-->
