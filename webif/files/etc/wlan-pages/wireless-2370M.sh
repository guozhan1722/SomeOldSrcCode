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

cardtype=$(cat /lib/mdf/cardtype)

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

device="radio0"

if empty "$FORM_submit"; then
    config_get iftype "$device" type
    config_get FORM_channel $device channel
    config_get FORM_distance $device distance
    config_get_bool FORM_disabled $device disabled 0
else
    config_get iftype "$device" type
    eval FORM_distance="\$FORM_distance_$device"
    eval FORM_channel="\$FORM_channel_$device"
    eval FORM_disabled="\$FORM_disabled_$device"
    empty "$FORM_generate_wep_128" && empty "$FORM_generate_wep_40" &&
    {
	SAVED=1
validate <<EOF
int|FORM_distance_${device}|@TR<<Distance>>||$FORM_distance
EOF
        if [ "$?" = 0 ]; then
            uci_set "wireless" "$device" "distance" "$FORM_distance"
            uci_set "wireless" "$device" "disabled" "$FORM_disabled"
            uci_set "wireless" "$device" "channel" "$FORM_channel"
        fi
    }
fi

append forms "start_form" "$N"

mode_disabled="field|@TR<<Radio>>
                radio|disabled_$device|$FORM_disabled|0|@TR<<On>>
                radio|disabled_$device|$FORM_disabled|1|@TR<<Off>>"
append forms "$mode_disabled" "$N"


vcfgcnt=0
for vcfg in $vface; do
    config_get FORM_device $vcfg device
    if [ "$FORM_device" = "$device" ]; then
        if empty "$FORM_submit"; then
	    #config_get FORM_network $vcfg network
	    config_get FORM_mode $vcfg mode
	    config_get FORM_ssid $vcfg ssid
	    config_get FORM_mesh_id $vcfg mesh_id
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
	    #config_get FORM_rate $vcfg rate
	    config_get FORM_txpower $vcfg txpower
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
	    eval FORM_encryption="\$FORM_encryption_$vcfgcnt"
	    case "$FORM_encryption" in
                psk|psk2|psk-mixe*|psk+psk2) eval FORM_key="\$FORM_wpa_psk_$vcfgcnt";;
		wpa|wpa2|wpa+wpa2) eval FORM_key="\$FORM_radius_key_$vcfgcnt";;
	    esac
	    eval FORM_mode="\$FORM_mode_$vcfgcnt"
            eval FORM_wep_key="\$FORM_wep_key_$vcfgcnt"
	    eval FORM_key1="\$FORM_key1_$vcfgcnt"
	    eval FORM_key2="\$FORM_key2_$vcfgcnt"
	    eval FORM_key3="\$FORM_key3_$vcfgcnt"
	    eval FORM_key4="\$FORM_key4_$vcfgcnt"
	    eval FORM_ssid="\$FORM_ssid_$vcfgcnt"
	    eval FORM_mesh_id="\$FORM_mesh_id_$vcfgcnt"
            #eval FORM_network="\$FORM_network_$vcfgcnt"
            eval FORM_txpower="\$FORM_txpower_$vcfgcnt"
            #eval FORM_rate="\$FORM_rate_$vcfgcnt"
            eval FORM_macpolicy="\$FORM_macpolicy_$vcfgcnt"
            eval FORM_maclistadd="\$FORM_${vcfgcnt}_maclistadd"
            eval FORM_wep_passphrase="\$FORM_wep_passphrase_$vcfgcnt"
            config_get FORM_maclist "$vcfg" maclist
            [ $FORM_maclistadd != "" ] && FORM_maclist="$FORM_maclist $FORM_maclistadd"
            empty "$FORM_generate_wep_128" && empty "$FORM_generate_wep_40" &&
            {
                case "$FORM_encryption" in
		    psk|psk2|psk+psk2|*mixed*) append validate_forms "wpapsk|FORM_wpa_psk_$vcfgcnt|@TR<<WPA PSK#WPA Pre-Shared Key (min needs 8 characters) >>|min=8 max=63 required|$FORM_key" "$N";;
		    wpa|wpa2|wpa+wpa2) append validate_forms "string|FORM_radius_key_$vcfgcnt|@TR<<RADIUS Server Key>>|min=4 max=63 required|$FORM_key" "$N"
		                       append validate_forms "ip|FORM_server_$vcfgcnt|@TR<<RADIUS IP Address>>|required|$FORM_server" "$N"
		                       append validate_forms "port|FORM_radius_port_$vcfgcnt|@TR<<RADIUS Port>>|required|$FORM_radius_port" "$N";;
		    wep)append validate_forms "int|FORM_wep_key_$vcfgcnt|@TR<<Selected WEP Key>>|min=1 max=4|$FORM_wep_key" "$N"
		    	append validate_forms "wep|FORM_key1_$vcfgcnt|@TR<<WEP Key>> 1||$FORM_key1" "$N"
			append validate_forms "wep|FORM_key2_$vcfgcnt|@TR<<WEP Key>> 2||$FORM_key2" "$N"
			append validate_forms "wep|FORM_key3_$vcfgcnt|@TR<<WEP Key>> 3||$FORM_key3" "$N"
			append validate_forms "wep|FORM_key4_$vcfgcnt|@TR<<WEP Key>> 4||$FORM_key4" "$N";;
		esac
		case "$FORM_mode" in
        		mesh)
	        		append validate_forms "string|FORM_mesh_id_$vcfgcnt|@TR<<MESH ID>>|required|$FORM_mesh_id" "$N";; 
			*)
		        	append validate_forms "string|FORM_ssid_$vcfgcnt|@TR<<Network ID>>|required|$FORM_ssid" "$N";;
		esac

            SAVED=1
validate <<EOF
$validate_forms
mac|FORM_maclistadd|@TR<<Mac List>>||$FORM_maclistadd
EOF
                if [ "$?" = 0 ]; then
                    #uci_set "wireless" "$vcfg" "network" "$FORM_network"
		    case "$FORM_mode" in
      			mesh)
			    # uci_remove "wireless" "$vcfg" "ssid"
			    uci_set "wireless" "$vcfg" "mesh_id" "$FORM_mesh_id" 
			;;
			*)
			    uci_set "wireless" "$vcfg" "ssid" "$FORM_ssid"
			    # uci_remove "wireless" "$vcfg" "mesh_id"
                        ;;
		    esac
                    uci_set "wireless" "$vcfg" "mode" "$FORM_mode"
                    uci_set "wireless" "$vcfg" "encryption" "$FORM_encryption"
                    uci_set "wireless" "$vcfg" "txpower" "$FORM_txpower"
                    #uci_set "wireless" "$vcfg" "rate" "$FORM_rate" 
        
                    case "$FORM_encryption" in
                        wep) uci_set "wireless" "$vcfg" "key" "$FORM_wep_key";;
                        psk|psk2|psk+psk2|*mixed*) uci_set "wireless" "$vcfg" "key" "$FORM_key";;
                        wpa|wpa2|wpa+wpa2) uci_set "wireless" "$vcfg" "key" "$FORM_key";;
                    esac
                    uci_set "wireless" "$vcfg" "key1" "$FORM_key1"
                    uci_set "wireless" "$vcfg" "key2" "$FORM_key2"
                    uci_set "wireless" "$vcfg" "key3" "$FORM_key3"
                    uci_set "wireless" "$vcfg" "key4" "$FORM_key4"
                    uci_set "wireless" "$vcfg" "macfilter" "$FORM_macpolicy"
                    uci_set "wireless" "$vcfg" "maclist" "$FORM_maclist"
                    uci_set "wireless" "$vcfg" "wep_passphrase" "$FORM_wep_passphrase"
                fi
            }
        fi
        let "count+=1"
#	network="field|@TR<<Network>>
#            select|network_$vcfgcnt|$FORM_network
#	    $network_options"
#	append forms "$network" "$N"

	mode_fields="field|@TR<<WLAN Mode#Mode>>
            select|mode_$vcfgcnt|$FORM_mode
	    option|ap|@TR<<Master>>
            option|sta|@TR<<Slave>>
            option|repeater|@TR<<Repeater>>
	    option|mesh|@TR<<Mesh>>"
        append forms "$mode_fields" "$N"

        #rate_list="1M 2M 5.5M 11M 6M 9M 12M 18M 24M 36M 48M 54M"
        #rate_field="field|@TR<<TX Rate>>
        #select|rate_$vcfgcnt|$FORM_rate
        #option|auto|@TR<<Auto>>"
        #for rate in $rate_list; do
        #    rate_field="$rate_field
        #    option|$rate|@TR<<$rate>>"
        #done
        #append forms "$rate_field" "$N"

	ssid="field|@TR<<Network ID>>|ssid_form_$vcfgcnt|hidden
		text|ssid_$vcfgcnt|$FORM_ssid"
	append forms "$ssid" "$N"

	mesh_id="field|@TR<<MESH ID>>|mesh_id_form_$vcfgcnt|hidden
	        text|mesh_id_$vcfgcnt|$FORM_mesh_id"
	append forms "$mesh_id" "$N"

        CHANNELS="field|@TR<<Frequency>>
                select|channel_$device|$FORM_channel"
        CHANNELS="$CHANNELS
        option|1|2,304 GHz
        option|2|2,309 GHz
        option|3|2,314 GHz
        option|4|2,319 GHz
        option|5|2,324 GHz
        option|6|2,329 GHz
        option|7|2,334 GHz
        option|8|2,339 GHz
        option|9|2,344 GHz
        option|10|2,349 GHz
        option|11|2,254 GHz
        option|12|2,359 GHz"
        
        append forms "$CHANNELS" "$N"

        txpowers='20 21 22 23 24 25 26 27 28 29 30'
        txpower_field="field|@TR<<Tx Power>>
            select|txpower_$vcfgcnt|$FORM_txpower"
            for txpower in $txpowers; do
                txpower_field="$txpower_field
                option|$txpower|$txpower dbm"
            done
        append forms "$txpower_field" "$N"

        distance="field|@TR<<Wireless Distance (m)>>
        	text|distance_${device}|$FORM_distance"
        append forms "$distance" "$N"
        

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
			
            psk_option="option|psk-mixed/tkip+aes|Enabled"

            encryption_forms="field|@TR<<Encryption Type>>
        		select|encryption_$vcfgcnt|$FORM_encryption
			option|none|@TR<<Disabled>>
	        	$psk_option"
            append forms "$encryption_forms" "$N"

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

            wpa="field|Encryption Password|wpapsk_$vcfgcnt|hidden
			password|wpa_psk_$vcfgcnt|$FORM_key
                        field|@TR<<Show password>>|passwd_status_$vcfgcnt|hidden
		        checkbox|show_passwd_$vcfgcnt|$FORM_show_passwd|1
			field|@TR<<RADIUS IP Address>>|radius_ip_$vcfgcnt|hidden
			text|server_$vcfgcnt|$FORM_server
			field|@TR<<RADIUS Port>>|radius_port_form_$vcfgcnt|hidden
			text|radius_port_$vcfgcnt|$FORM_radius_port
			field|@TR<<RADIUS Server Key>>|radiuskey_$vcfgcnt|hidden
			text|radius_key_$vcfgcnt|$FORM_key"
            append forms "$wpa" "$N"

	    append forms "end_form" "$N"

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
				//
				// force encryption listbox to no selection if user tries
				// to set WPA (PSK) with mesh mode.
				//

		                v = isset('mode_$vcfgcnt','mesh');
				set_visible('mesh_id_form_$vcfgcnt', v);
				set_visible('ssid_form_$vcfgcnt', !v);
                                
				//
				//wpa_supplicant does not support psk+psk2
				//
				if (isset('mode_$vcfgcnt','sta') && ('$iftype'=='atheros'))
				{
					if (isset('encryption_$vcfgcnt','psk-mixed/tkip+aes'))
					{
						document.getElementById('encryption_$vcfgcnt').value = 'none';
					}
				}

				//
				// force encryption listbox to no selection if user tries
				// to set WPA (Radius) with anything but AP mode.
				//
				if (!isset('mode_$vcfgcnt','ap'))
				{
					if (isset('encryption_$vcfgcnt','wpa') || isset('encryption_$vcfgcnt','wpa2'))
				        { 
						document.getElementById('encryption_$vcfgcnt').value = 'none'; 
					}
				}
				//v = (isset('ap_mode_$device','11b') || isset('ap_mode_$device','11bg') || isset('ap_mode_$device','11g'));
				//set_visible('bgchannelform_$device', v); 

				if (!isset('mode_$vcfgcnt','sta') && ('$iftype'=='mac80211')) 
				{ 
				    if (isset('channel_$device','0')) 
				    { 
				    
				       document.getElementById('channel_$device').selectedIndex = '1'; 
				    }
				}
				v = (isset('encryption_$vcfgcnt','psk') || isset('encryption_$vcfgcnt','psk2') || isset('encryption_$vcfgcnt','psk+psk2')  || isset('encryption_$vcfgcnt','psk-mixed/tkip+aes'));
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

				v = (('$iftype'!='broadcom'));
				set_visible('macpolicy_form_$vcfgcnt', v);
				v = (!isset('macpolicy_$vcfgcnt','none'));
				set_visible('maclist_form_$vcfgcnt', v);"
			append js "$javascript_forms" "$N"
			# mac80211 does not support multiple virtual interface
			#remove_vcfg="start_form
			#	field|
			#	string|<a href=\"$SCRIPT_NAME?remove_vcfg=$vcfg\">@TR<<Remove Virtual Interface>></a>"
			#append forms "$remove_vcfg" "$N"
			#append forms "end_form" "$N"
                        let "vcfgcnt+=1"
		fi
	done

header "Wireless" "Radio" "@TR<<Radio Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"
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
##WEBIF:name:Wireless:400:Radio
-->
