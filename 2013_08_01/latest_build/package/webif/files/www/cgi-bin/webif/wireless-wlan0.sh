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
generate_channels() {
	iwlist channel 2>&- |grep -q "GHz"
	if [ "$?" != "0" ]; then
		is_package_installed kmod-madwifi
		if [ "$?" = "0" ]; then
			wlanconfig ath create wlandev wifi0 wlanmode ap 2>/dev/null >/dev/null
			cleanup=1
		fi
	fi
	BGCHANNELS="$(iwlist channel 2>&- |grep -v "no frequency information." |grep -v "[Ff]requenc" |grep -v "Current" |grep "2.[0-9]" |cut -d' ' -f12|sort |uniq)"
	ACHANNELS="$(iwlist channel 2>&- |grep -v "no frequency information." |grep -v "[Ff]requenc" |grep -v "Current" |grep "5.[0-9]" |cut -d' ' -f12|sort |uniq)"
	if ! empty "$ACHANNELS" ; then
    	    ACHANNELS="36 40 44 48 149 153 157 161 165"
	fi
	echo "BGCHANNELS=\"${BGCHANNELS}\"" > /usr/lib/webif/channels.lst
	echo "ACHANNELS=\"${ACHANNELS}\"" >> /usr/lib/webif/channels.lst
	if [ "$cleanup" = "1" ]; then
		wifi 2>/dev/null >/dev/null
	fi
}

if [ ! -f /usr/lib/webif/channels.lst ]; then
	generate_channels
fi

[ -f /usr/lib/webif/channels.lst ] && . /usr/lib/webif/channels.lst

[ "$PRODUCT" = "VIP421EXP49G" ] && { 
    ACHANNELS="36 40 44 48 149 153 157 161 165"
    #XACHANNELS="$(iwlist channel 2>&- |grep -v "no frequency information." |grep -v "[Ff]requenc" |grep -v "Current" |grep "4.[0-9]" |awk '{print $2$3$4$5}'|uniq|sed '//s/:/-/g')"
    XACHANNELS="$(cat /lib/mdf/xchannellist.49g |sed '//s/=/-/g')"
} ||{
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
	uci_set "wireless" "$wireless_cfg" "ssid" "OpenWrt$FORM_add_vcfg_number"
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
	network_options="$network_options 
			option|$iface|$iface"
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
DEVICES="radio0"
for device in $DEVICES; do
	if empty "$FORM_submit"; then
                [ "$PRODUCT" = "VIP421EXP49G" ] && {
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
		[ -z $FORM_antenna ] && FORM_antenna=auto
	else
		config_get iftype "$device" type
		#eval FORM_country="\$FORM_country_$device"
		eval FORM_ap_mode="\$FORM_ap_mode_$device"
		eval FORM_bwmode="\$FORM_bwmode_$device"
                [ "$PRODUCT" = "VIP421EXP49G" ] && {
                    `cat /lib/mdf/mdfcfg |grep -v "exmode=" > /lib/mdf/mdfcfg_tmp` 
                    if [ "$FORM_ap_mode" = "11ax" ];then 
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
                        echo "exmode='49G'" >> /lib/mdf/mdfcfg_tmp
                    else
                        FORM_ex49gmode=""
		        eval FORM_channel="\$FORM_achannel_$device"
                        echo "exmode=standard" >> /lib/mdf/mdfcfg_tmp
                    fi
                    `cp /lib/mdf/mdfcfg_tmp /lib/mdf/mdfcfg`
                    `rm /lib/mdf/mdfcfg_tmp -rf`
                    . /lib/mdf/mdfcfg                              
                    mdf -w $exmode >/dev/null 2>&1   
                    FORM_apx_mode="11a"
                } || {
		if [ "$FORM_ap_mode" = "11a" ]; then
		     eval FORM_channel="\$FORM_achannel_$device"
		else
		     eval FORM_channel="\$FORM_bgchannel_$device"
	        fi
                }
		
		eval FORM_maxassoc="\$FORM_maxassoc_$device"
		eval FORM_distance="\$FORM_distance_$device"
		eval FORM_diversity="\$FORM_diversity_$device"
		eval FORM_txantenna="\$FORM_txantenna_$device"
		eval FORM_rxantenna="\$FORM_rxantenna_$device"
		eval FORM_disabled="\$FORM_disabled_$device"
		eval FORM_antenna="\$FORM_antenna_$device"

		empty "$FORM_generate_wep_128" && empty "$FORM_generate_wep_40" &&
		{
			SAVED=1
			validate <<EOF
int|FORM_distance_$vcfg|@TR<<Distance>>||$FORM_distance
int|FORM_maxassoc_$vcfg|@TR<<Max Associated Clients>>||$FORM_maxassoc
EOF
			if [ "$?" = 0 ]; then
				#uci_set "wireless" "$device" "country" "$FORM_country"

                                [ "$PRODUCT" = "VIP421EXP49G" ] && {
                                    uci_set "wireless" "$device" "hwmode" "$FORM_apx_mode"
	                            uci_set "wireless" "$device" "ex49gmode" "$FORM_ex49gmode" 
                                } || {
                                    uci_set "wireless" "$device" "hwmode" "$FORM_ap_mode"
                                }
				uci_set "wireless" "$device" "channel" "$FORM_channel"
				uci_set "wireless" "$device" "maxassoc" "$FORM_maxassoc"
				uci_set "wireless" "$device" "bwmode" "$FORM_bwmode"
				uci_set "wireless" "$device" "distance" "$FORM_distance"
				uci_set "wireless" "$device" "diversity" "$FORM_diversity"
				uci_set "wireless" "$device" "txantenna" "$FORM_txantenna"
				uci_set "wireless" "$device" "rxantenna" "$FORM_rxantenna"
				uci_set "wireless" "$device" "disabled" "$FORM_disabled"
				uci_set "wireless" "$device" "antenna" "$FORM_antenna"
			fi
		}
	fi

	append forms "start_form|$device @TR<< Phy Configuration>>" "$N"
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
	append forms "$mode_disabled" "$N"

	if [ "$iftype" = "atheros" ]; then
        	mode_fields="field|@TR<<Mode>>
			select|ap_mode_$device|$FORM_ap_mode"
		echo "$dmesg_txt" |grep -q "${device}: 11g"
		if [ "$?" = "0" ]; then
			mode_fields="$mode_fields
				option|11bg|@TR<<802.11B/G>>
				option|11g|@TR<<802.11G>>"
		fi
		echo "$dmesg_txt" |grep -q "${device}: 11b"
		if [ "$?" = "0" ]; then
			mode_fields="$mode_fields
				option|11b|@TR<<802.11B>>"
		fi
		echo "$dmesg_txt" |grep -q "${device}: 11a"
		if [ "$?" = "0" ]; then
			mode_fields="$mode_fields
				option|11a|@TR<<802.11A>>"
		fi
		append forms "$mode_fields" "$N"

		BG_CHANNELS="field|@TR<<Channel>>|bgchannelform_$device|hidden
			select|bgchannel_$device|$FORM_channel
			option|0|@TR<<Auto>>"
		for ch in $BGCHANNELS; do
			BG_CHANNELS="$BG_CHANNELS
				option|$ch"
		done

         	append forms "$BG_CHANNELS" "$N"

		A_CHANNELS="field|@TR<<Channel>>|achannelform_$device|hidden
			select|achannel_$device|$FORM_channel"
		for ch in $ACHANNELS; do
			A_CHANNELS="$A_CHANNELS
				option|$ch"
		done
		append forms "$A_CHANNELS" "$N"
	elif [ "$iftype" = "mac80211" ]; then
        	mode_fields="field|@TR<<Mode>>
			select|ap_mode_$device|$FORM_ap_mode"

                [ "$PRODUCT" = "VIP421EXP49G" ] && {
			mode_fields="$mode_fields
				option|11a|@TR<<802.11A>>
				option|11ax|@TR<<802.11AX>>"
                } || {
                    if [ "$FORM_ap_mode" = "11a" ]; then
			mode_fields="$mode_fields
    				option|11a|@TR<<802.11A>>"
                    else
			mode_fields="$mode_fields
    				option|11bg|@TR<<802.11B/G>>
    				option|11g|@TR<<802.11G>>"
                    fi

                }
		append forms "$mode_fields" "$N"

        	bwmode="field|@TR<<Channel BandWidth>>
        		select|bwmode_${device}|$FORM_bwmode
        		option|20mHz|@TR<<20MHz Normal Rate>>
        		option|5mHz|@TR<< 5MHz Quarter Rate>>
        		option|10mHz|@TR<<10MHz Half Rate>>
        		option|40mHz|@TR<<40MHz Turbo Rate>>"
        	append forms "$bwmode" "$N"

                [ "$PRODUCT" = "VIP421EXP49G" ] && {
                    dev_minfreq=$(cat /lib/mdf/mdfcfg |grep ":49G=" |awk -F "=" '{print $2}'|awk -F "," '{printf $1}')
                    dev_maxfreq=$(cat /lib/mdf/mdfcfg |grep ":49G=" |awk -F "=" '{print $2}'|awk -F "," '{printf $2}')

                    A_CHANNELS="field|@TR<<Channel-Freq(MHz)>>|achannelform_$device|hidden
         			select|achannel_$device|$FORM_channel
         			option|0|@TR<<Auto>>"
         
                    for ch in $ACHANNELS; do
                        eval freq=$(echo $ch|awk '{print (5000+$1*5)/1000}')
         		A_CHANNELS="$A_CHANNELS
                            option|$ch|$ch-$freq GHz"
         	    done
         	    append forms "$A_CHANNELS" "$N"

                    XA_CHANNELS_NORMAL="field|@TR<<Normal Channel-Freq(KHz)>>|xachannelnormalform_$device|hidden
                                        select|xachannelnormal_$device|$FORM_channel
                                        option|0|@TR<<Auto>>"
                                                        
                    for ch in $XACHANNELS; do
                        chnum=$(echo "$ch" |cut -d "-" -f1  )
                        freq=$(echo "$ch" |cut -d "-" -f2 )
                        minfreq=$(expr $dev_minfreq + 10000 )
                        maxfreq=$(expr $dev_maxfreq - 10000 )

                        [ "$freq" -ge "$minfreq" ] && [ "$freq" -le "$maxfreq" ]  && {
                            XA_CHANNELS_NORMAL="$XA_CHANNELS_NORMAL
                                                option|$chnum|$ch"
                        }
                    done
                    append forms "$XA_CHANNELS_NORMAL" "$N"
    
                    XA_CHANNELS_HALF="field|@TR<<Half Channel-Freq(KHz)>>|xachannelhalfform_$device|hidden
                                      select|xachannelhalf_$device|$FORM_channel
                                      option|0|@TR<<Auto>>"
             
                    for ch in $XACHANNELS; do
                        chnum=$(echo "$ch" |cut -d "-" -f1  )
                        freq=$(echo "$ch" |cut -d "-" -f2 )
                        minfreq=$(expr $dev_minfreq + 5000 )
                        maxfreq=$(expr $dev_maxfreq - 5000 )

                        [ "$freq" -ge "$minfreq" ] && [ "$freq" -le "$maxfreq" ]  && {
                            XA_CHANNELS_HALF="$XA_CHANNELS_HALF
                                            option|$chnum|$ch"
                        } 
                    done
                    append forms "$XA_CHANNELS_HALF" "$N"
    
                    XA_CHANNELS_QUAT="field|@TR<<Quarter Channel-Freq(KHz)>>|xachannelquatform_$device|hidden
                                      select|xachannelquat_$device|$FORM_channel
                                      option|0|@TR<<Auto>>"
             
                    for ch in $XACHANNELS; do
                        chnum=$(echo "$ch" |cut -d "-" -f1  )
                        freq=$(echo "$ch" |cut -d "-" -f2 )
                        minfreq=$(expr $dev_minfreq + 2500 )
                        maxfreq=$(expr $dev_maxfreq - 2500 )
                        [ "$freq" -ge "$minfreq" ] && [ "$freq" -le "$maxfreq" ]  && {
                            XA_CHANNELS_QUAT="$XA_CHANNELS_QUAT
                                        option|$chnum|$ch"
                        }
                    done
                    append forms "$XA_CHANNELS_QUAT" "$N"
    
                    XA_CHANNELS_TUBO="field|@TR<<TURBO Channel-Freq(KHz)>>|xachanneltuboform_$device|hidden
                                    select|xachanneltubo_$device|$FORM_channel
                                    option|0|@TR<<Auto>>"
             
                    for ch in $XACHANNELS; do
                        chnum=$(echo "$ch" |cut -d "-" -f1  )
                        freq=$(echo "$ch" |cut -d "-" -f2 )
                        minfreq=$(expr $dev_minfreq + 20000 )
                        maxfreq=$(expr $dev_maxfreq - 20000 )

                        [ "$freq" -ge "$minfreq" ] && [ "$freq" -le "$maxfreq" ]  && {
                            XA_CHANNELS_TUBO="$XA_CHANNELS_TUBO
                                        option|$chnum|$ch"
                        }
                        done
                        append forms "$XA_CHANNELS_TUBO" "$N"

                } || {
                    if [ "$FORM_ap_mode" = "11a" ]; then
         		A_CHANNELS="field|@TR<<Channel-Freq>>|achannelform_$device
             			select|achannel_$device|$FORM_channel
             			option|0|@TR<<Auto>>"
             
             		for ch in $ACHANNELS; do
                                 eval freq=$(echo $ch|awk '{print (5000+$1*5)/1000}')
         			A_CHANNELS="$A_CHANNELS
                                          option|$ch|$ch-$freq GHz"
         		done
         		append forms "$A_CHANNELS" "$N"
                    else
         		BG_CHANNELS="field|@TR<<Channel-Freq>>|bgchannelform_$device
         			select|bgchannel_$device|$FORM_channel
         			option|0|@TR<<Auto>>"
         			
         		for ch in $BGCHANNELS; do
         			eval freq=$(echo $ch|awk '{print (2407+$1*5)/1000}')
         			BG_CHANNELS="$BG_CHANNELS
         				option|$ch|$ch-$freq GHz"
         		done
                  	append forms "$BG_CHANNELS" "$N"
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
         	append forms "$BG_CHANNELS" "$N"
	fi

	if [ "$iftype" = "atheros" ]; then
		if [ "$_device" != "NanoStation2" -a "$_device" != "NanoStation5" ]; then
			mode_diversity="field|@TR<<Diversity>>
					radio|diversity_$device|$FORM_diversity|1|@TR<<On>>
					radio|diversity_$device|$FORM_diversity|0|@TR<<Off>>"      	
			append forms "$mode_diversity" "$N"
			append forms "helpitem|Diversity" "$N"
			append forms "helptext|Helptext Diversity#Used on systems with multiple antennas to help improve reception. Disable if you only have one antenna." "$N"
			append forms "helplink|http://madwifi-project.org/wiki/UserDocs/AntennaDiversity" "$N"

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
	distance="field|@TR<<Wireless Distance (In Meters)>>
		text|distance_${device}|$FORM_distance"

	append forms "$distance" "$N"
	append forms "helpitem|Wireless Distance" "$N"
	append forms "helptext|Helptext Wireless Distance#This is the distance of your longest link." "$N"
	#fi
	# mac80211 does not support multiple virtual interface
	#add_vcfg="field|
	#	string|<a href=\"$SCRIPT_NAME?add_vcfg=$device&amp;add_vcfg_number=$vcfg_number\">@TR<<Add Virtual Interface>></a>"
	#append forms "$add_vcfg" "$N"
	append forms "end_form" "$N"

	#####################################################################
	# This is looped for every virtual wireless interface (wifi-iface)
	#
        count=0
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
				config_get FORM_frag $vcfg frag
	        		config_get FORM_rts $vcfg rts
	        		config_get_bool FORM_hidden $vcfg hidden 0
	        		#config_get_bool FORM_isolate $vcfg isolate 0
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
				config_get FORM_macpolicy "$vcfg" macfilter
				config_get FORM_maclist "$vcfg" maclist
                                config_get FORM_wep_passphrase "$vcfg" wep_passphrase
				eval FORM_maclistremove="\$FORM_${vcfg}_maclistremove"
				if [ "$FORM_maclistremove" != "" ]; then
					list_remove FORM_maclist "$FORM_maclistremove"
					uci_set "wireless" "$vcfg" "maclist" "$FORM_maclist"
				fi
			else
				eval FORM_key="\$FORM_radius_key_$vcfg"
				eval FORM_radius_ipaddr="\$FORM_radius_ipaddr_$vcfg"
				
				eval FORM_encryption="\$FORM_encryption_$vcfg"
				case "$FORM_encryption" in
					psk|psk2|psk-mixe*|psk+psk2) eval FORM_key="\$FORM_wpa_psk_$vcfg";;
					wpa|wpa2|wpa+wpa2) eval FORM_key="\$FORM_radius_key_$vcfg";;
				esac
				eval FORM_mode="\$FORM_mode_$vcfg"
				eval FORM_server="\$FORM_server_$vcfg"
				eval FORM_radius_port="\$FORM_radius_port_$vcfg"
				eval FORM_hidden="\$FORM_broadcast_$vcfg"
				#eval FORM_isolate="\$FORM_isolate_$vcfg"
				eval FORM_wep_key="\$FORM_wep_key_$vcfg"
				eval FORM_key1="\$FORM_key1_$vcfg"
				eval FORM_key2="\$FORM_key2_$vcfg"
				eval FORM_key3="\$FORM_key3_$vcfg"
				eval FORM_key4="\$FORM_key4_$vcfg"
				eval FORM_broadcast="\$FORM_broadcast_$vcfg"
				eval FORM_ssid="\$FORM_ssid_$vcfg"
				eval FORM_mesh_id="\$FORM_mesh_id_$vcfg"
				eval FORM_wds="\$FORM_wds_$vcfg"
				eval FORM_bssid="\$FORM_bssid_$vcfg"
				eval FORM_network="\$FORM_network_$vcfg"
				eval FORM_txpower="\$FORM_txpower_$vcfg"
                                eval FORM_rate="\$FORM_rate_$vcfg"
				#eval FORM_bgscan="\$FORM_bgscan_$vcfg"
				eval FORM_frag="\$FORM_frag_$vcfg"
				eval FORM_rts="\$FORM_rts_$vcfg"
				#eval FORM_doth="\$FORM_doth_$vcfg"
				#eval FORM_compression="\$FORM_compression_$vcfg"
				#eval FORM_bursting="\$FORM_bursting_$vcfg"
				#eval FORM_fframes="\$FORM_fframes_$vcfg"
				#eval FORM_wmm="\$FORM_wmm_$vcfg"
				#eval FORM_xr="\$FORM_xr_$vcfg"
				#eval FORM_ar="\$FORM_ar_$vcfg"
				eval FORM_turbo="\$FORM_turbo_$vcfg"
				eval FORM_macpolicy="\$FORM_macpolicy_$vcfg"
				eval FORM_maclistadd="\$FORM_${vcfg}_maclistadd"
                                eval FORM_wep_passphrase="\$FORM_wep_passphrase_$vcfg"
				config_get FORM_maclist "$vcfg" maclist
				[ $FORM_maclistadd != "" ] && FORM_maclist="$FORM_maclist $FORM_maclistadd"
				empty "$FORM_generate_wep_128" && empty "$FORM_generate_wep_40" &&
				{
					case "$FORM_encryption" in
						psk|psk2|psk+psk2|*mixed*) append validate_forms "wpapsk|FORM_wpa_psk_$vcfg|@TR<<WPA PSK#WPA Pre-Shared Key>>|min=8 max=63 required|$FORM_key" "$N";;
						wpa|wpa2|wpa+wpa2) append validate_forms "string|FORM_radius_key_$vcfg|@TR<<RADIUS Server Key>>|min=4 max=63 required|$FORM_key" "$N"
							append validate_forms "ip|FORM_server_$vcfg|@TR<<RADIUS IP Address>>|required|$FORM_server" "$N"
							append validate_forms "port|FORM_radius_port_$vcfg|@TR<<RADIUS Port>>|required|$FORM_radius_port" "$N";;
						wep)
							append validate_forms "int|FORM_wep_key_$vcfg|@TR<<Selected WEP Key>>|min=1 max=4|$FORM_wep_key" "$N"
							append validate_forms "wep|FORM_key1_$vcfg|@TR<<WEP Key>> 1||$FORM_key1" "$N"
							append validate_forms "wep|FORM_key2_$vcfg|@TR<<WEP Key>> 2||$FORM_key2" "$N"
							append validate_forms "wep|FORM_key3_$vcfg|@TR<<WEP Key>> 3||$FORM_key3" "$N"
							append validate_forms "wep|FORM_key4_$vcfg|@TR<<WEP Key>> 4||$FORM_key4" "$N";;
					esac
					case "$FORM_mode" in
						wds)
							append validate_forms "string|FORM_ssid_$vcfg|@TR<<ESSID>>||$FORM_ssid" "$N"
							append validate_forms "mac|FORM_bssid_$vcfg|@TR<<BSSID>>||$FORM_bssid" "$N";;
						mesh)
							append validate_forms "string|FORM_mesh_id_$vcfg|@TR<<MESH ID>>|required|$FORM_mesh_id" "$N";; 
						*)
							append validate_forms "string|FORM_ssid_$vcfg|@TR<<ESSID>>|required|$FORM_ssid" "$N";;
					esac
					SAVED=1
					validate <<EOF
$validate_forms
append validate_forms "int|FORM_frag_$vcfg|@TR<<Fragmentation Threshold>>|min=0 max=2346|$FORM_frag" "$N"
append validate_forms "int|FORM_rts_$vcfg|@TR<<RTS Threshold>>|min=0 max=2347|$FORM_rts" "$N"
append validate_forms "mac|FORM_maclistadd_$vcfg|@TR<<MAC Address>>||$FORM_maclistadd" "$N"
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
						uci_set "wireless" "$vcfg" "bssid" "$FORM_bssid"
						uci_set "wireless" "$vcfg" "mode" "$FORM_mode"
						uci_set "wireless" "$vcfg" "encryption" "$FORM_encryption"
						uci_set "wireless" "$vcfg" "server" "$FORM_server"
						uci_set "wireless" "$vcfg" "port" "$FORM_radius_port"
						uci_set "wireless" "$vcfg" "hidden" "$FORM_hidden"
						#uci_set "wireless" "$vcfg" "isolate" "$FORM_isolate"
						uci_set "wireless" "$vcfg" "txpower" "$FORM_txpower"
						#if [ "$iftype" = "mac80211" ]; then 
						#   uci_set "wireless" "$vcfg" "txpower" "17" 
						#fi
                                                uci_set "wireless" "$vcfg" "rate" "$FORM_rate" 
                                                #uci_set "wireless" "$vcfg" "bgscan" "$FORM_bgscan"
						uci_set "wireless" "$vcfg" "frag" "$FORM_frag"
						uci_set "wireless" "$vcfg" "rts" "$FORM_rts"
						uci_set "wireless" "$vcfg" "wds" "$FORM_wds"

						case "$FORM_encryption" in
							wep) uci_set "wireless" "$vcfg" "key" "$FORM_wep_key";;
							psk|psk2|psk+psk2|*mixed*) uci_set "wireless" "$vcfg" "key" "$FORM_key";;
							wpa|wpa2|wpa+wpa2) uci_set "wireless" "$vcfg" "key" "$FORM_key";;
						esac
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

			append forms "start_form|$FORM_device @TR<< Virtual Interface>> $count" "$N"
                        let "count+=1"
			network="field|@TR<<Network>>
	        	        select|network_$vcfg|$FORM_network
	        	        $network_options"
			append forms "$network" "$N"

			if [ "$iftype" != "mac80211" ]; then
				option_wds="option|wds|@TR<<WDS>>"
				append forms "helpitem|WDS Connections" "$N"
				append forms "helptext|Helptext WDS Connections#Enter the MAC address of the router on your network that should be wirelessly connected to. The other router must also support wds and have the mac address of this router entered." "$N"
			fi

			mode_fields="field|@TR<<WLAN Mode#Mode>>
			select|mode_$vcfg|$FORM_mode
			option|ap|@TR<<Access Point>>
			$option_wds
			option|sta|@TR<<Client>>
			option|mesh|@TR<<Mesh Point>>"
			append forms "$mode_fields" "$N"

                        [ "$PRODUCT" = "VIP421EXP49G" ] && {
   			    rate_list="6M 9M 12M 18M 24M 36M 48M 54M"
			} || {
                            if [ "$FORM_ap_mode" = "11a" ]; then
   			   rate_list="6M 9M 12M 18M 24M 36M 48M 54M"

			else
			   rate_list="1M 2M 5.5M 11M 6M 9M 12M 18M 24M 36M 48M 54M" 
			fi
                        }
			rate_field="field|@TR<<TX Rate>>
			select|rate_$vcfg|$FORM_rate
			option|auto|@TR<<Auto>>"
			for rate in $rate_list; do
			rate_field="$rate_field
                        option|$rate|@TR<<$rate>>"
			done

			append forms "$rate_field" "$N"

                        txpowers='5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30'
			txpower_field="field|@TR<<Tx Power>>
			select|txpower_$vcfg|$FORM_txpower"
			for txpower in $txpowers; do
			txpower_field="$txpower_field
                        option|$txpower|$txpower dbm"
			done
			append forms "$txpower_field" "$N"

			wds="field|@TR<<WDS>>|wds_form_$vcfg|hidden
				radio|wds_$vcfg|$FORM_wds|1|@TR<<On>>
				radio|wds_$vcfg|$FORM_wds|0|@TR<<Off>>"
			append forms "$wds" "$N"

			hidden="field|@TR<<ESSID Broadcast>>|broadcast_form_$vcfg|hidden
				radio|broadcast_$vcfg|$FORM_hidden|0|@TR<<On>>
				radio|broadcast_$vcfg|$FORM_hidden|1|@TR<<Off>>"
			append forms "$hidden" "$N"

			#bgscan_field="field|@TR<<Background Client Scanning>>|bgscan_form_$vcfg|hidden
			#		radio|bgscan_$vcfg|$FORM_bgscan|1|@TR<<On>>
			#		radio|bgscan_$vcfg|$FORM_bgscan|0|@TR<<Off>>"
			#append forms "$bgscan_field" "$N"
			#append forms "helpitem|Background Client Scanning" "$N"

			#isolate_field="field|@TR<<AP Isolation>>|isolate_form_$vcfg|hidden
			#		radio|isolate_$vcfg|$FORM_isolate|1|@TR<<On>>
			#		radio|isolate_$vcfg|$FORM_isolate|0|@TR<<Off>>"
			#append forms "$isolate_field" "$N"

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
						radio|turbo_$vcfg|$FORM_turbo|1|@TR<<On>>
						radio|turbo_$vcfg|$FORM_turbo|0|@TR<<Off>>"
					append forms "$turbo" "$N"
				fi

				rate="field|@TR<<TX Rate>>
					select|rate_$vcfg|$FORM_rate
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
			if [ "$iftype" != "broadcom" ]; then
				rts="field|@TR<<RTS (Default off)>>
					text|rts_$vcfg|$FORM_rts"
				append forms "$rts" "$N"

				frag="field|@TR<<Fragmentation (Default off)>>
					text|frag_$vcfg|$FORM_frag"
				append forms "$frag" "$N"
			fi

                        PRON_R=$(echo $PRODUCTNAME |grep IPnV)
                        [ -z $PRON_R ] && {
        			ssid="field|@TR<<SSID>>|ssid_form_$vcfg
        				text|ssid_$vcfg|$FORM_ssid"
        			append forms "$ssid" "$N"
                        }||{
        			ssid="field|@TR<<Network ID>>|ssid_form_$vcfg
        				text|ssid_$vcfg|$FORM_ssid"
        			append forms "$ssid" "$N"
                        }


			mesh_id="field|@TR<<MESH ID>>|mesh_id_form_$vcfg
				text|mesh_id_$vcfg|$FORM_mesh_id"
			append forms "$mesh_id" "$N"

			bssid="field|@TR<<BSSID>>|bssid_form_$vcfg|hidden
				text|bssid_$vcfg|$FORM_bssid"
			append forms "$bssid" "$N"

			###################################################################
			# Generate 4 40-bit WEP keys or 1 128-bit WEP Key
			#eval FORM_wep_passphrase="\$FORM_wep_passphrase_$vcfg"
			[ "$FORM_wep_passphrase" = "" ] && FORM_wep_passphrase="$(dd if=/dev/urandom count=200 bs=1 2>/dev/null|tr "\n" " "|sed 's/[^a-zA-Z0-9]//g')"
			eval FORM_generate_wep_128="\$FORM_generate_wep_128_$vcfg"
			eval FORM_generate_wep_40="\$FORM_generate_wep_40_$vcfg"
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
					FORM_key2=""
					FORM_key3=""
					FORM_key4=""
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
			fi

			[ "$PRODUCT" = "VIP421EXPBR" ] && { 
                            encryption_forms="field|@TR<<Encryption Type>>
				select|encryption_$vcfg|$FORM_encryption
				option|none|@TR<<Disabled>>
				option|wep|WEP"
			    append forms "$encryption_forms" "$N"
                        } || {
                            encryption_forms="field|@TR<<Encryption Type>>
				select|encryption_$vcfg|$FORM_encryption
				option|none|@TR<<Disabled>>
				option|wep|WEP
				option|psk|WPA (@TR<<PSK>>)
				option|psk2|WPA2 (@TR<<PSK>>)
				$psk_option
				option|wpa|WPA (@TR<<RADIUS>>)
				option|wpa2|WPA2 (@TR<<RADIUS>>)
				$wpa_option"
			    append forms "$encryption_forms" "$N"
                        }

			[ "$PRODUCT" = "VIP421EXPBR" ] && {
			    wep="field|@TR<<Passphrase>>|wep_keyphrase_$vcfg|hidden
				text|wep_passphrase_$vcfg|$FORM_wep_passphrase
				string|<br />
				field|&nbsp;|wep_generate_keys_$vcfg|hidden
				submit|generate_wep_40_$vcfg|@TR<<Generate 40bit Keys>>
				string|<br />
				field|@TR<<WEP Key 1>>|wep_key_1_$vcfg|hidden
				radio|wep_key_$vcfg|$FORM_wep_key|1
				text|key1_$vcfg|$FORM_key1|<br />
				field|@TR<<WEP Key 2>>|wep_key_2_$vcfg|hidden
				radio|wep_key_$vcfg|$FORM_wep_key|2
				text|key2_$vcfg|$FORM_key2|<br />
				field|@TR<<WEP Key 3>>|wep_key_3_$vcfg|hidden
				radio|wep_key_$vcfg|$FORM_wep_key|3
				text|key3_$vcfg|$FORM_key3|<br />
				field|@TR<<WEP Key 4>>|wep_key_4_$vcfg|hidden
				radio|wep_key_$vcfg|$FORM_wep_key|4
				text|key4_$vcfg|$FORM_key4|<br />"
			    append forms "$wep" "$N"
                        
                        } || { 
			    wep="field|@TR<<Passphrase>>|wep_keyphrase_$vcfg|hidden
				text|wep_passphrase_$vcfg|$FORM_wep_passphrase
				string|<br />
				field|&nbsp;|wep_generate_keys_$vcfg|hidden
				submit|generate_wep_40_$vcfg|@TR<<Generate 40bit Keys>>
				submit|generate_wep_128_$vcfg|@TR<<Generate 128bit Key>>
				string|<br />
				field|@TR<<WEP Key 1>>|wep_key_1_$vcfg|hidden
				radio|wep_key_$vcfg|$FORM_wep_key|1
				text|key1_$vcfg|$FORM_key1|<br />
				field|@TR<<WEP Key 2>>|wep_key_2_$vcfg|hidden
				radio|wep_key_$vcfg|$FORM_wep_key|2
				text|key2_$vcfg|$FORM_key2|<br />
				field|@TR<<WEP Key 3>>|wep_key_3_$vcfg|hidden
				radio|wep_key_$vcfg|$FORM_wep_key|3
				text|key3_$vcfg|$FORM_key3|<br />
				field|@TR<<WEP Key 4>>|wep_key_4_$vcfg|hidden
				radio|wep_key_$vcfg|$FORM_wep_key|4
				text|key4_$vcfg|$FORM_key4|<br />"
			    append forms "$wep" "$N"
                       }

			wpa="field|WPA @TR<<PSK>>|wpapsk_$vcfg|hidden
				password|wpa_psk_$vcfg|$FORM_key
                                field|@TR<<Show password>>|passwd_status_$vcfg|hidden
			        checkbox|show_passwd_$vcfg|$FORM_show_passwd|1
				field|@TR<<RADIUS IP Address>>|radius_ip_$vcfg|hidden
				text|server_$vcfg|$FORM_server
				field|@TR<<RADIUS Port>>|radius_port_form_$vcfg|hidden
				text|radius_port_$vcfg|$FORM_radius_port
				field|@TR<<RADIUS Server Key>>|radiuskey_$vcfg|hidden
				text|radius_key_$vcfg|$FORM_key"
			append forms "$wpa" "$N"
			
			if [ "$iftype" = "broadcom" ]; then
				install_nas_button="field|@TR<<NAS Package>>|install_nas_$vcfg|hidden"
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
				install_hostapd_mini_button="field|@TR<<HostAPD Package>>|install_hostapd_mini_$vcfg|hidden"
				if [ "$hostapd_installed" = "1" -o "$hostapd_mini_installed" = "1" ]; then
					install_hostapd_mini_button="$install_hostapd_mini_button
						string|@TR<<Installed>>."
				else
					install_hostapd_mini_button="$install_hostapd_mini_button
						string|<div class=\"warning\">PSK and PSK2 will not work until you install the HostAPD or HostAPD Mini package. (HostAPD Mini only does PSK and PSK2) </div>
						submit|install_hostapd_mini| Install HostAPD-Mini Package |
						submit|install_hostapd| Install HostAPD Package |"
				fi
				install_hostapd_button="field|@TR<<HostAPD Package>>|install_hostapd_$vcfg|hidden"
				if [ "$hostapd_installed" != "1" ]; then
					install_hostapd_button="$install_hostapd_button
						string|<div class=\"warning\">WPA and WPA2 will not work until you install the HostAPD package. </div>
						submit|install_hostapd| Install HostAPD Package |"
				else
					install_hostapd_button="$install_hostapd_button
						string|@TR<<Installed>>."
				fi

				install_wpa_supplicant_button="field|@TR<<wpa-supplicant Package>>|install_wpa_supplicant_$vcfg|hidden"
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
			
			append forms "helpitem|Encryption Type" "$N"
			append forms "helptext|HelpText Encryption Type#WPA (RADIUS) is only supported in Access Point mode. WPA (PSK) does not work in Ad-Hoc mode." "$N"

				macpolicy="field|@TR<<MAC Filter>>|macpolicy_form_$vcfg|hidden
					select|macpolicy_$vcfg|$FORM_macpolicy
					option|none|@TR<<Disabled>>
					option|allow|@TR<<Allow>>
					option|deny|@TR<<Deny>>"
				append forms "$macpolicy" "$N"

				maclist="end_form
					start_form|@TR<<MAC List>>|maclist_form_$vcfg|hidden
					listedit|${vcfg}_maclist|$SCRIPT_NAME?|$FORM_maclist|$FORM_macadd
					end_form"
				append forms "$maclist" "$N"

			###################################################################
			# set JavaScript
			javascript_forms="
				v = isset('encryption_$vcfg','wep');
				set_visible('wep_key_1_$vcfg', v);
				set_visible('wep_key_2_$vcfg', v);
				set_visible('wep_key_3_$vcfg', v);
				set_visible('wep_key_4_$vcfg', v);
				set_visible('wep_generate_keys_$vcfg', v);
				set_visible('wep_keyphrase_$vcfg', v);
				set_visible('wep_keys_$vcfg', v);
				//
				// force encryption listbox to no selection if user tries
				// to set WPA (PSK) with mesh mode.
				//
				if (isset('mode_$vcfg','mesh'))
				{
				        //v = isset('mode_$vcfg','mesh');
					set_visible('mesh_id_form_$vcfg', 1);

					//v = !isset('mode_$vcfg','mesh');
					set_visible('ssid_form_$vcfg', 0);
				}
				else
				{
				        //v = isset('mode_$vcfg','mesh');
					set_visible('mesh_id_form_$vcfg', 0);

					//v = !isset('mode_$vcfg','mesh');
					set_visible('ssid_form_$vcfg', 1);
				}

				//
				//wpa_supplicant does not support psk+psk2
				//
				if (isset('mode_$vcfg','sta') && ('$iftype'=='atheros'))
				{
					if (isset('encryption_$vcfg','psk-mixed/tkip+aes'))
					{
						document.getElementById('encryption_$vcfg').value = 'none';
					}
				}

				//
				// force encryption listbox to no selection if user tries
				// to set WPA (Radius) with anything but AP mode.
				//
				if (!isset('mode_$vcfg','ap'))
				{
					if (isset('encryption_$vcfg','wpa') || isset('encryption_$vcfg','wpa2'))
				        { 
						document.getElementById('encryption_$vcfg').value = 'none'; 
					}
				}
				//v = (isset('ap_mode_$device','11b') || isset('ap_mode_$device','11bg') || isset('ap_mode_$device','11g'));
				//set_visible('bgchannelform_$device', v); 

				v = (isset('ap_mode_$device','11a'));
				set_visible('achannelform_$device', v);

				// can not set to auto in ap/mesh mode
                                if (isset('ap_mode_$device','11ax'))
                                {
                                    v = (isset('bwmode_$device','20mHz'));
                                    set_visible('xachannelnormalform_$device', v);
                                    v = (isset('bwmode_$device','10mHz'));
                                    set_visible('xachannelhalfform_$device', v);
                                    v = (isset('bwmode_$device','5mHz'));
                                    set_visible('xachannelquatform_$device', v);
                                    v = (isset('bwmode_$device','40mHz'));
                                    set_visible('xachanneltuboform_$device', v);
                                }

                                else
                                {
				    v = (isset('ap_mode_$device','11a'));
				    set_visible('achannelform_$device', v);
                                    set_visible('xachannelnormalform_$device', 0);
                                    set_visible('xachannelhalfform_$device', 0);
                                    set_visible('xachanneltuboform_$device', 0);
                                }
                                
				if (!isset('mode_$vcfg','sta') && ('$iftype'=='mac80211')) 
				{ 
				    if (isset('bgchannel_$device','0')) 
				    { 
				    
				       document.getElementById('bgchannel_$device').selectedIndex = '1'; 
				    }
				    if (isset('achannel_$device','0'))
				    { 
					     
				       document.getElementById('achannel_$device').selectedIndex = '1'; 
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

				v = (!isset('mode_$vcfg','wds'));
				set_visible('broadcast_form_$vcfg', v);
				v = ((isset('mode_$vcfg','wds')) || (isset('mode_$vcfg','adhoc') && ('$iftype'=='mac80211')));
				set_visible('bssid_form_$vcfg', v);
				v = (isset('mode_$vcfg','sta'));
				set_visible('bgscan_form_$vcfg', v);
				v = (isset('mode_$vcfg','ap'));
				set_visible('isolate_form_$vcfg', v);
				v = (isset('encryption_$vcfg','psk') || isset('encryption_$vcfg','psk2') || isset('encryption_$vcfg','psk+psk2')  || isset('encryption_$vcfg','psk-mixed/tkip+aes'));
				set_visible('wpapsk_$vcfg', v);
                                set_visible('passwd_status_$vcfg', v);
                                if( document.getElementById('show_passwd_${vcfg}_1').checked)
                                {
                                     document.getElementById('wpa_psk_$vcfg').type = 'text';
                                }
                                else
                                {
                                     document.getElementById('wpa_psk_$vcfg').type = 'password';
                                }

				v = (('$iftype'=='broadcom') && (isset('encryption_$vcfg','psk') || isset('encryption_$vcfg','psk2') || isset('encryption_$vcfg','psk+psk2') || isset('encryption_$vcfg','wpa') || isset('encryption_$vcfg','wpa2') || isset('encryption_$vcfg','wpa+wpa2')));
				set_visible('install_nas_$vcfg', v);
				v = (('$iftype'!='broadcom') && (!isset('mode_$vcfg','sta')) && (isset('encryption_$vcfg','psk') || isset('encryption_$vcfg','psk2')));
				// do not show install-package field
				set_visible('install_hostapd_mini_$vcfg', 0);
				v = (('$iftype'!='broadcom') && (!isset('mode_$vcfg','sta')) && (isset('encryption_$vcfg','wpa') || isset('encryption_$vcfg','wpa2')));
				// do not show install-package field
				set_visible('install_hostapd_$vcfg', 0);
				v = (('$iftype'!='broadcom') && (isset('mode_$vcfg','sta')) && (isset('encryption_$vcfg','psk') || isset('encryption_$vcfg','psk2') || isset('encryption_$vcfg','wpa') || isset('encryption_$vcfg','wpa2')));
				// do not show install-package field
				set_visible('install_wpa_supplicant_$vcfg', 0);
				v = (isset('encryption_$vcfg','wpa') || isset('encryption_$vcfg','wpa2') || isset('encryption_$vcfg','wpa+wpa2'));
				set_visible('radiuskey_$vcfg', v);
				set_visible('radius_ip_$vcfg', v);
				set_visible('radius_port_form_$vcfg', v);
				v = (('$iftype'!='mac80211') && (isset('mode_$vcfg','ap') || isset('mode_$vcfg','sta')));
				set_visible('wds_form_$vcfg', v);
				v = (('$iftype'!='broadcom'));
				set_visible('macpolicy_form_$vcfg', v);
				v = (!isset('macpolicy_$vcfg','none'));
				set_visible('maclist_form_$vcfg', v);"
			append js "$javascript_forms" "$N"
			# mac80211 does not support multiple virtual interface
			#remove_vcfg="start_form
			#	field|
			#	string|<a href=\"$SCRIPT_NAME?remove_vcfg=$vcfg\">@TR<<Remove Virtual Interface>></a>"
			#append forms "$remove_vcfg" "$N"
			#append forms "end_form" "$N"
		fi
	done
	validate_wireless $iftype
done

header "Wireless" "Radio0" "@TR<<Wireless Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"
#####################################################################
# modechange script
#
cat <<EOF
<script type="text/javascript" src="/webif.js"></script>
<script type="text/javascript">
<!--
function modechange()
{
	var v;
	$js

	hide('save');
	show('save');
}
-->
</script>

EOF

display_form <<EOF
onchange|modechange
$validate_error
$forms
EOF

footer ?>
<!--
##WEBIF:name:Wireless:400:Radio0
-->
