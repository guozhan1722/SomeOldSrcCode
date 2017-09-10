wpa_supplicant_setup_vif() {
	local vif="$1"
	local driver="$2"
	local key="$key"
	local options="$3"
        local freq="$4"

	# wpa_supplicant should use wext for mac80211 cards
	[ "$driver" = "mac80211" ] && driver='wext'

	# make sure we have the encryption type and the psk
	[ -n "$enc" ] || {
		config_get enc "$vif" encryption
	}
	[ -n "$key" ] || {
		config_get key "$vif" key
	}

	local net_cfg bridge
	config_get bridge "$vif" bridge
	[ -z "$bridge" ] && {
		net_cfg="$(find_net_config "$vif")"
		[ -z "$net_cfg" ] || bridge="$(bridge_interface "$net_cfg")"
		config_set "$vif" bridge "$bridge"
	}

	case "$enc" in
		*none*)
			key_mgmt='NONE'
		;;
		*wep*)
			key_mgmt='NONE'
			config_get key "$vif" key
			key="${key:-1}"
			case "$key" in
				[1234])
					for idx in 1 2 3 4; do
						local zidx
						zidx=$(($idx - 1))
						config_get ckey "$vif" "key${idx}"
						[ -n "$ckey" ] && \
							append "wep_key${zidx}" "wep_key${zidx}=$(prepare_key_wep "$ckey")"
					done
					wep_tx_keyidx="wep_tx_keyidx=$((key - 1))"
				;;
				*)
					wep_key0="wep_key0=$(prepare_key_wep "$key")"
					wep_tx_keyidx="wep_tx_keyidx=0"
				;;
			esac
		;;
		*psk*)
			key_mgmt='WPA-PSK'
			config_get_bool usepassphrase "$vif" passphrase 1
			if [ "$usepassphrase" = "1" ]; then
				passphrase="psk=\"${key}\""
			else
				passphrase="psk=${key}"
			fi
			case "$enc" in
				*psk2*)
					proto='proto=RSN'
					config_get ieee80211w "$vif" ieee80211w
				;;
				*psk*)
					proto='proto=WPA'
				;;
			esac
		;;
		*wpa*|*8021x*)
                        #proto='proto=WPA2'
                        proto='proto=RSN WPA'                      
			key_mgmt='WPA-EAP'
			config_get ieee80211w "$vif" ieee80211w
			config_get ca_cert "$vif" ca_cert
			config_get eap_type "$vif" eap_type peap
			ca_cert=${ca_cert:+"ca_cert=\"$ca_cert\""}
			case "$eap_type" in
				tls)
					pairwise='pairwise=CCMP'
					group='group=CCMP'
					config_get identity "$vif" identity
					config_get priv_key "$vif" priv_key
					config_get priv_key_pwd "$vif" priv_key_pwd
					identity="identity=\"$identity\""
					priv_key="private_key=\"$priv_key\""
					priv_key_pwd="private_key_passwd=\"$priv_key_pwd\""
				;;
				peap|ttls)
					config_get auth "$vif" auth
					config_get identity "$vif" identity
					config_get password "$vif" password
					phase2="phase2=\"auth=${auth:-MSCHAPV2}\""
					identity="identity=\"$identity\""
					password="password=\"$password\""
				;;
			esac
			eap_type="eap=$(echo $eap_type | tr 'a-z' 'A-Z')"
		;;
	esac

	case "$ieee80211w" in
		[012])
			ieee80211w="ieee80211w=$ieee80211w"
		;;
	esac

	config_get ifname "$vif" ifname
	config_get bridge "$vif" bridge
	config_get ssid "$vif" ssid
	config_get bssid "$vif" bssid
	config_get mode "$vif" mode
        . /etc/version
        if [ "$PRODUCT" = "IPnDDL" ];then
            [ "$mode" = "sta" ] && {
               eval ap_freq='ap_freq=$freq'
            }
        fi
        [ "$mode" = "repeater" ] && {
		eval ifname="$ifname.rep"
                eval ap_freq='ap_freq=$freq'
	}
	bssid=${bssid:+"bssid=$bssid"}
	rm -rf /var/run/wpa_supplicant-$ifname
	cat > /var/run/wpa_supplicant-$ifname.conf <<EOF
ctrl_interface=/var/run/wpa_supplicant-$ifname
#ap_scan config is important for roaming
#using default (1) at this time
$ap_scan
$ap_freq
network={
	scan_ssid=1
	ssid="$ssid"
	$bssid
	key_mgmt=$key_mgmt
	$proto
	$ieee80211w
	$passphrase
	$pairwise
	$group
	$eap_type
	$ca_cert
	$priv_key
	$priv_key_pwd
	$phase2
	$identity
	$password
	$wep_key0
	$wep_key1
	$wep_key2
	$wep_key3
	$wep_tx_keyidx
}
EOF
	[ -z "$proto" -a "$key_mgmt" != "NONE" ] || {
		config_load vlan
		config_get_bool vlan_disabled general disabled 1
		config_get vlan_management general management 'vlan1'
		if [ "$vlan_disabled" -le 0 -a "$vlan_management" != "vlan1" ]; then
			brctl addbr br-vlan1
			brctl setfd br-vlan1 0
			ifconfig br-vlan1 up
			wpa_supplicant ${bridge:+ -b br-vlan1} -B -P "/var/run/wifi-${ifname}.pid" -D ${driver:-wext} -i "$ifname" -c /var/run/wpa_supplicant-$ifname.conf $options
		else
			wpa_supplicant ${bridge:+ -b $bridge} -B -P "/var/run/wifi-${ifname}.pid" -D ${driver:-wext} -i "$ifname" -c /var/run/wpa_supplicant-$ifname.conf $options
		fi	
	}
		
}
