#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

if ! empty "$FORM_editname" ; then
    FORM_grename="$FORM_editname"
    [ -d "/tmp/gre" ] || mkdir /tmp/gre
    echo $FORM_editname > /tmp/gre/editname
else
    FORM_grename=$(cat /tmp/gre/editname)
fi
#next step to implement mroute
#if ! empty "$FORM_mname" ; then
#    [ -d "/tmp/gre" ] || mkdir /tmp/gre
#    echo $FORM_mname > /tmp/gre/${FORM_grename}.mname
#fi

if ! empty "$FORM_eipsec" ; then
    [ -d "/tmp/gre" ] || mkdir /tmp/gre
    echo $FORM_eipsec > /tmp/gre/${FORM_grename}.eipsec
fi
if ! empty "$FORM_oinf" ; then
    [ -d "/tmp/gre" ] || mkdir /tmp/gre
    echo $FORM_oinf > /tmp/gre/${FORM_grename}.oinf
fi
#save a gre tunnel
if ! empty "$FORM_submit" ; then
    form_valid="false"

    if empty "$FORM_local_status_add" || ! equal $FORM_local_status_add "Enable"; then 
       FORM_local_status_add="Disable" 
    fi
    FORM_local_status="$FORM_local_status_add"  
     
    if empty "$FORM_multicast_add" || ! equal $FORM_multicast_add "Enable"; then 
       FORM_multicast_add="Disable" 
    fi
    FORM_multicast="$FORM_multicast_add"

    if empty "$FORM_arp_add" || ! equal $FORM_arp_add "Enable"; then 
       FORM_arp_add="Disable" 
    fi  
    FORM_arp="$FORM_arp_add"    
    FORM_ttl="$FORM_ttl_add"   
    FORM_key="$FORM_key_add"  
    FORM_inf="$FORM_tunnelinf"
    FORM_local_tunnel_ip="$FORM_local_tunnel_ip_add"
    FORM_local_tunnel_mask="$FORM_local_tunnel_mask_add"
    FORM_local_subnet_ip="$FORM_local_subnet_ip_add"
    FORM_local_subnet_mask="$FORM_local_subnet_mask_add"     
    FORM_local_wan="$FORM_local_wan_add"    
    FORM_remote_subnet="$FORM_remote_subnet_add"
    FORM_remote_subnet_mask="$FORM_remote_subnet_mask_add"   
    FORM_remote_wan="$FORM_remote_wan_add"

    if empty "$FORM_enableipsec_add"; then 
       FORM_enableipsec_add="Disable" 
    fi  
    FORM_enableipsec="$FORM_enableipsec_add"
    if empty "$FORM_tunnelpfs" || ! equal $FORM_tunnelpfs "yes"; then 
       FORM_tunnelpfs="no" 
    fi
    FORM_ipsec_mode="$FORM_ipsec_mode_add"
    FORM_pfs="$FORM_tunnelpfs"
    FORM_ike_alg="$FORM_tunnelikealg"
    FORM_ike_auth="$FORM_tunnelikeauth"
    FORM_ike_dh="$FORM_tunnelikedh"
    FORM_esp_alg="$FORM_tunnelespalg"
    FORM_esp_auth="$FORM_tunnelespauth"
    FORM_esp_dh="$FORM_tunnelespdh"
    FORM_ipsec_key="$FORM_tunnelkey"
    FORM_ikelifetime="$FORM_tunnelikelifetime"
    FORM_keylife="$FORM_tunnelkeylife"
    FORM_dpdaction="$FORM_tunneldpdaction"
    FORM_dpddelay="$FORM_tunneldpddelay"
    FORM_dpdtimeout="$FORM_tunneldpdtimeout"
    FORM_ipsec_localgwip="$FORM_ipsec_localgwip_add"
    FORM_ipsec_localsid="$FORM_ipsec_localsid_add"
    FORM_ipsec_remotegwip="$FORM_ipsec_remotegwip_add"
    FORM_ipsec_remotesid="$FORM_ipsec_remotesid_add"
    FORM_ipsec_localsbip="$FORM_ipsec_localsbip_add"
    FORM_ipsec_localsbmask="$FORM_ipsec_localsbmask_add"
    FORM_ipsec_remotesbip="$FORM_ipsec_remotesbip_add"
    FORM_ipsec_remotesbmask="$FORM_ipsec_remotesbmask_add"
    FORM_ipsec_localsip="$FORM_ipsec_localsip_add"
    FORM_ipsec_localnext="$FORM_ipsec_localnext_add"
    FORM_ipsec_remotenext="$FORM_ipsec_remotenext_add"
    FORM_lefttype="$FORM_tunnellefttype"
    FORM_righttype="$FORM_tunnelrighttype"
    if empty "$FORM_tunnelaggr" || ! equal $FORM_tunnelaggr "yes"; then 
       FORM_tunnelaggr="no" 
    fi
    FORM_aggr="$FORM_tunnelaggr"
#    if empty "$FORM_enablemroute_add" || ! equal $FORM_enablemroute_add "Enable"; then 
#       FORM_enablemroute_add="Disable" 
#    fi  
#    FORM_enablemroute="$FORM_enablemroute_add"
#    FORM_mroutename="$FORM_mroutename_add"
#    FORM_mroute_localip="$FORM_mroute_localip_add"
#    FORM_mroute_localmask="$FORM_mroute_localmask_add"
#    FORM_mroute_remoteip="$FORM_mroute_remoteip_add"
#    FORM_mroute_remotemask="$FORM_mroute_remotemask_add"
#    FORM_mroute_sip="$FORM_mroute_sip_add"
#    FORM_mroute_smask="$FORM_mroute_smask_add"
#    FORM_mroute_gip="$FORM_mroute_gip_add"
    


validate <<EOF
ip|FORM_local_tunnel_ip_add|@TR<<Tunnel IP Address>>|required|$FORM_local_tunnel_ip_add
ip|FORM_local_subnet_ip_add|@TR<<Subnet IP Address>>|required|$FORM_local_subnet_ip_add
ip|FORM_local_wan_add|@TR<<Gateway IP Address>>|required|$FORM_local_wan_add
ip|FORM_remote_subnet_add|@TR<<Subnet IP Address>>|required|$FORM_remote_subnet_add
ip|FORM_remote_wan_add|@TR<<Gateway IP Address>>|required|$FORM_remote_wan_add
netmask|FORM_local_tunnel_mask_add|@TR<<Netmask>>|required|$FORM_local_tunnel_mask_add
netmask|FORM_local_subnet_mask_add|@TR<<Subnet Mask>>|required|$FORM_local_subnet_mask_add
netmask|FORM_remote_subnet_mask_add|@TR<<Subnet Mask>>|required|$FORM_remote_subnet_mask_add
EOF

	equal "$?" 0 && {
	    add_gre_cfg="$FORM_grename"
            uci_set gre-tunnels "$add_gre_cfg" local_status         "$FORM_local_status_add"
	    uci_set gre-tunnels "$add_gre_cfg" multicast            "$FORM_multicast_add"
	    uci_set gre-tunnels "$add_gre_cfg" arp                  "$FORM_arp_add"
	    uci_set gre-tunnels "$add_gre_cfg" ttl                  "$FORM_ttl_add"
            uci_set gre-tunnels "$add_gre_cfg" gre_key              "$FORM_key_add"
            uci_set gre-tunnels "$add_gre_cfg" interface            "$FORM_inf"
	    uci_set gre-tunnels "$add_gre_cfg" local_subnet_ip      "$FORM_local_subnet_ip_add"
	    uci_set gre-tunnels "$add_gre_cfg" local_subnet_mask    "$FORM_local_subnet_mask_add"
	    uci_set gre-tunnels "$add_gre_cfg" local_tunnel_ip      "$FORM_local_tunnel_ip_add"
	    uci_set gre-tunnels "$add_gre_cfg" local_tunnel_mask    "$FORM_local_tunnel_mask_add"
	    uci_set gre-tunnels "$add_gre_cfg" local_wan            "$FORM_local_wan_add"
	    uci_set gre-tunnels "$add_gre_cfg" remote_netmask       "$FORM_remote_subnet_mask_add"
	    uci_set gre-tunnels "$add_gre_cfg" remote_subnet        "$FORM_remote_subnet_add"
	    uci_set gre-tunnels "$add_gre_cfg" remote_wan           "$FORM_remote_wan_add"
#	     uci_set gre-tunnels "$add_gre_cfg" enablemroute         "$FORM_enablemroute_add"
#            uci_set gre-tunnels "$add_gre_cfg" mroutename           "$FORM_mroutename_add"
#            uci_set gre-tunnels "$add_gre_cfg" mroute_localip       "$FORM_mroute_localip_add"
#            uci_set gre-tunnels "$add_gre_cfg" mroute_localmask     "$FORM_mroute_localmask_add"
#            uci_set gre-tunnels "$add_gre_cfg" mroute_remoteip      "$FORM_mroute_remoteip_add"
#            uci_set gre-tunnels "$add_gre_cfg" mroute_remotemask    "$FORM_mroute_remotemask_add"
#            uci_set gre-tunnels "$add_gre_cfg" mroute_sip           "$FORM_mroute_sip_add"
#            uci_set gre-tunnels "$add_gre_cfg" mroute_smask         "$FORM_mroute_smask_add"
#            uci_set gre-tunnels "$add_gre_cfg" mroute_gip           "$FORM_mroute_gip_add"
            uci_set gre-tunnels "$add_gre_cfg" enableipsec          "$FORM_enableipsec_add"
            uci_set gre-tunnels "$add_gre_cfg" ipsec_localgwip      "$FORM_ipsec_localgwip_add"
            uci_set gre-tunnels "$add_gre_cfg" ipsec_localsid       "$FORM_ipsec_localsid_add"
            uci_set gre-tunnels "$add_gre_cfg" ipsec_remotegwip     "$FORM_ipsec_remotegwip_add"
            uci_set gre-tunnels "$add_gre_cfg" ipsec_remotesid      "$FORM_ipsec_remotesid_add"
            uci_set gre-tunnels "$add_gre_cfg" "pfs"                "$FORM_pfs"
            uci_set gre-tunnels "$add_gre_cfg" "ike_alg"            "$FORM_ike_alg"
            uci_set gre-tunnels "$add_gre_cfg" "ike_auth"           "$FORM_ike_auth"
            uci_set gre-tunnels "$add_gre_cfg" "ike_dh"             "$FORM_ike_dh"
            uci_set gre-tunnels "$add_gre_cfg" "ikelifetime"        "$FORM_ikelifetime"
            uci_set gre-tunnels "$add_gre_cfg" "esp_alg"            "$FORM_esp_alg"
            uci_set gre-tunnels "$add_gre_cfg" "esp_auth"           "$FORM_esp_auth"
            uci_set gre-tunnels "$add_gre_cfg" "esp_dh"             "$FORM_esp_dh"
            uci_set gre-tunnels "$add_gre_cfg" "keylife"            "$FORM_keylife"
            uci_set gre-tunnels "$add_gre_cfg" "ipsec_key"          "$FORM_ipsec_key"
            uci_set gre-tunnels "$add_gre_cfg" "ipsec_mode"         "$FORM_ipsec_mode"
            uci_set gre-tunnels "$add_gre_cfg" "dpddelay"           "$FORM_dpddelay"
            uci_set gre-tunnels "$add_gre_cfg" "dpdtimeout"         "$FORM_dpdtimeout"
            uci_set gre-tunnels "$add_gre_cfg" "dpdaction"          "$FORM_dpdaction"
            uci_set gre-tunnels "$add_gre_cfg" ipsec_localsbip      "$FORM_ipsec_localsbip_add"
            uci_set gre-tunnels "$add_gre_cfg" ipsec_localsbmask    "$FORM_ipsec_localsbmask_add"
            uci_set gre-tunnels "$add_gre_cfg" ipsec_remotesbip     "$FORM_ipsec_remotesbip_add"
            uci_set gre-tunnels "$add_gre_cfg" ipsec_remotesbmask   "$FORM_ipsec_remotesbmask_add"
            uci_set gre-tunnels "$add_gre_cfg" ipsec_localsip       "$FORM_ipsec_localsip_add"
            uci_set gre-tunnels "$add_gre_cfg" ipsec_localnext      "$FORM_ipsec_localnext_add"
            uci_set gre-tunnels "$add_gre_cfg" ipsec_remotenext     "$FORM_ipsec_remotenext_add"
            uci_set gre-tunnels "$add_gre_cfg" "lefttype"           "$FORM_lefttype"
            uci_set gre-tunnels "$add_gre_cfg" "righttype"          "$FORM_righttype"
            uci_set gre-tunnels "$add_gre_cfg" "aggrmode"           "$FORM_aggr"
            form_valid="true"
        }
fi

if [ "$form_valid" = "false" ]; then
    form_valid="true"
fi
    uci_load gre-tunnels
    config_get FORM_local_status            $FORM_grename local_status    
    config_get FORM_multicast               $FORM_grename multicast
    config_get FORM_arp                     $FORM_grename arp    
    config_get FORM_ttl                     $FORM_grename ttl   
    config_get FORM_key                     $FORM_grename gre_key
    config_get FORM_inf                     $FORM_grename interface  
    config_get FORM_local_subnet_ip         $FORM_grename local_subnet_ip
    config_get FORM_local_subnet_mask       $FORM_grename local_subnet_mask
    config_get FORM_local_tunnel_ip         $FORM_grename local_tunnel_ip
    config_get FORM_local_tunnel_mask       $FORM_grename local_tunnel_mask 
    config_get FORM_local_wan               $FORM_grename local_wan    
    config_get FORM_remote_subnet           $FORM_grename remote_subnet
    config_get FORM_remote_subnet_mask      $FORM_grename remote_netmask   
    config_get FORM_remote_wan              $FORM_grename remote_wan 
    config_get FORM_enableipsec             $FORM_grename enableipsec
#    config_get FORM_enablemroute            $FORM_grename enablemroute   
#    config_get FORM_mroutename              $FORM_grename mroutename
#    config_get FORM_mroute_localip          $FORM_grename mroute_localip
#    config_get FORM_mroute_localmask        $FORM_grename mroute_localmask
#    config_get FORM_mroute_remoteip         $FORM_grename mroute_remoteip
#    config_get FORM_mroute_remotemask       $FORM_grename mroute_remotemask
#    config_get FORM_mroute_sip              $FORM_grename mroute_sip
#    config_get FORM_mroute_smask            $FORM_grename mroute_smask
#    config_get FORM_mroute_gip              $FORM_grename mroute_gip
    config_get FORM_ipsec_localgwip         $FORM_grename ipsec_localgwip
    config_get FORM_ipsec_localsid          $FORM_grename ipsec_localsid
    config_get FORM_ipsec_remotegwip        $FORM_grename ipsec_remotegwip
    config_get FORM_ipsec_remotesid         $FORM_grename ipsec_remotesid
    config_get FORM_pfs                     $FORM_grename pfs
    config_get FORM_ike_alg                 $FORM_grename ike_alg
    config_get FORM_ike_auth                $FORM_grename ike_auth
    config_get FORM_ike_dh                  $FORM_grename ike_dh
    config_get FORM_ikelifetime             $FORM_grename ikelifetime
    config_get FORM_esp_alg                 $FORM_grename esp_alg
    config_get FORM_esp_auth                $FORM_grename esp_auth
    config_get FORM_esp_dh                  $FORM_grename esp_dh
    config_get FORM_keylife                 $FORM_grename keylife
    config_get FORM_ipsec_key               $FORM_grename ipsec_key
    config_get FORM_ipsec_mode              $FORM_grename ipsec_mode
    config_get FORM_dpddelay                $FORM_grename dpddelay
    config_get FORM_dpdtimeout              $FORM_grename dpdtimeout
    config_get FORM_dpdaction               $FORM_grename dpdaction
    config_get FORM_ipsec_localsbip         $FORM_grename ipsec_localsbip
    config_get FORM_ipsec_localsbmask       $FORM_grename ipsec_localsbmask
    config_get FORM_ipsec_remotesbip        $FORM_grename ipsec_remotesbip
    config_get FORM_ipsec_remotesbmask      $FORM_grename ipsec_remotesbmask
    config_get FORM_ipsec_localsip          $FORM_grename ipsec_localsip
    config_get FORM_ipsec_localnext         $FORM_grename ipsec_localnext
    config_get FORM_ipsec_remotenext        $FORM_grename ipsec_remotenext
    config_get FORM_aggr                    $FORM_grename aggrmode
    config_get FORM_lefttype                $FORM_grename lefttype
    config_get FORM_righttype               $FORM_grename righttype

    if [ $FORM_local_status = "Enable" ]; then
          local_status_flages="Enable||checked"
    else
          local_status_flages="Enable"
    fi
    if [ $FORM_multicast = "Enable" ]; then
          multicast_flages="Enable||checked"
    else
          multicast_flages="Enable"
    fi
    if [ $FORM_arp = "Enable" ]; then
          arp_flages="Enable||checked"
    else
          arp_flages="Enable"
    fi
    if [ $FORM_enableipsec = "Enable" ]; then
          FORM_enableipsec="bgs"
    fi
    if [ $FORM_enableipsec = "Disable" ]; then
          ipsec_fields="hidden"
    else
          ipsec_fields=""
    fi
    if [ $FORM_pfs = "yes" ]; then
          pfs_flages="yes||checked"
    else
          pfs_flages="yes"
    fi

    [ -z $FORM_aggr] && FORM_aggr="no"
    [ -z $FORM_lefttype] && FORM_lefttype="ip-id"
    [ -z $FORM_righttype] && FORM_righttype="ip-id"
    if [ $FORM_lefttype = "ip-only" ]; then
          lsid_flage="hidden"
          lgwip_flage=""
    else
          lsid_flage=""
          if [ $FORM_lefttype = "ip-id" ]; then
                lgwip_flage=""
          else
                lgwip_flage="hidden"
          fi 
    fi

    if [ $FORM_righttype = "ip-only" ]; then
          rsid_flage="hidden"
          rgwip_flage=""
    else
          rsid_flage=""
          if [ $FORM_righttype = "ip-id" ]; then
                rgwip_flage=""
          else
                rgwip_flage="hidden"
          fi
    fi

    [ "$FORM_enableipsec" = "Disable" ] && {
          lsid_flage="hidden"
          lgwip_flage="hidden"
          rsid_flage="hidden"
          rgwip_flage="hidden"
    }

#    if [ $FORM_enablemroute = "Enable" ]; then
#          mroute_flages="Enable||checked"
#          mroute_fields=""
#    else
#          mroute_flages="Enable"
#          mroute_fields="hidden"
#    fi

add_form="start_form|
            string|<tr><td>@TR<<Name>></td>
	    string|<td><input id=\"grename_add\" type=\"text\" name=\"grename_add\" value=\"$FORM_grename\" disabled=\"disabled\"></td></tr>
            field|@TR<<Enable>>
            checkbox|local_status_add|$FORM_local_status|${local_status_flages}

            field|@TR<<Multicast>>
	    checkbox|multicast_add|$FORM_multicast|${multicast_flages}
            field|@TR<<TTL>>
	    text|ttl_add|$FORM_ttl
            field|@TR<<Key>>
	    text|key_add|$FORM_key

            field|@TR<<ARP>>
	    checkbox|arp_add|$FORM_arp|${arp_flages}
            field|@TR<<Interface>>
            select|tunnelinf|$FORM_inf
            option|br-wan|@TR<<WAN>>
            option|br-wan2|@TR<<4G>>
            end_form"
append forms "$add_form" "$N"

add_form="start_form|@TR<<Local Setup>>
            field|@TR<<Gateway IP Address>>
	    text|local_wan_add|$FORM_local_wan
            field|@TR<<Tunnel IP Address>>
	    text|local_tunnel_ip_add|$FORM_local_tunnel_ip
            field|@TR<<Netmask>>
	    text|local_tunnel_mask_add|$FORM_local_tunnel_mask
            field|@TR<<Subnet IP Address>>
	    text|local_subnet_ip_add|$FORM_local_subnet_ip
            field|@TR<<Subnet Mask>>
	    text|local_subnet_mask_add|$FORM_local_subnet_mask
            end_form"
append forms "$add_form" "$N"

add_form="start_form|@TR<<Remote Setup>>
            field|@TR<<Gateway IP Address>>
	    text|remote_wan_add|$FORM_remote_wan
            field|@TR<<Subnet IP Address>>
	    text|remote_subnet_add|$FORM_remote_subnet
            field|@TR<<Subnet Mask>>
	    text|remote_subnet_mask_add|$FORM_remote_subnet_mask
            end_form"
append forms "$add_form" "$N"

add_form="start_form|@TR<<IPsec Setup>>
            field|@TR<<Enable>>
            onchange|ipsecbutton
            select|enableipsec_add|$FORM_enableipsec
            option|Disable|@TR<<None>>
            option|bgs|@TR<<Before GRE Setup>>
            option|ags|@TR<<After GRE Setup>>
            onchange|
            field|@TR<<Tunnel Mode>>|mode_field|${ipsec_fields}
            select|ipsec_mode_add|$FORM_ipsec_mode
            option|transport|@TR<<Transport>>
            option|tunnel|@TR<<Tunnel>>
            field|@TR<<Aggressive Mode>>|aggr_field|${ipsec_fields}
            checkbox|tunnelaggr|$FORM_aggr|yes||none

            field|@TR<<Local Security Gateway Type>>|lgwtype_field|${ipsec_fields}
            onchange|modechange
            select|tunnellefttype|$FORM_lefttype
            option|ip-only|@TR<<IP Only>>
            option|ip-id|@TR<<IP + Server ID>>
            option|dip-id|@TR<<Dynamic IP + Server ID>>
            onchange|
            field|@TR<<Local Gateway IP>>|lgwip_field|${lgwip_flage}
            text|ipsec_localgwip_add|$FORM_ipsec_localgwip
            field|@TR<<Local Server Id>>|lsid_field|${lsid_flage}
            text|ipsec_localsid_add|$FORM_ipsec_localsid
            field|@TR<<Local Next-hop Gateway IP>>|lnext_field|${ipsec_fields}
            text|ipsec_localnext_add|$FORM_ipsec_localnext
            field|@TR<<Local Subnet IP>>|lsbip_field|${ipsec_fields}
            text|ipsec_localsbip_add|$FORM_ipsec_localsbip
            field|@TR<<Local Subnet Mask>>|lsbmask_field|${ipsec_fields}
            text|ipsec_localsbmask_add|$FORM_ipsec_localsbmask
            field|@TR<<Local Subnet Gateway>>|lsip_field|${ipsec_fields}
            text|ipsec_localsip_add|$FORM_ipsec_localsip

            field|@TR<<Remote Security Gateway Type>>|rgwtype_field|${ipsec_fields}
            onchange|modechange
            select|tunnelrighttype|$FORM_righttype
            option|ip-only|@TR<<IP Only>>
            option|ip-id|@TR<<IP + Server ID>>
            option|dip-id|@TR<<Dynamic IP + Server ID>>
            onchange|
            field|@TR<<Remote Gateway IP>>|rgwip_field|${rgwip_flage}
            text|ipsec_remotegwip_add|$FORM_ipsec_remotegwip
            field|@TR<<Remote Server Id>>|rsid_field|${rsid_flage}
            text|ipsec_remotesid_add|$FORM_ipsec_remotesid
            field|@TR<<Remote Next-hop Gateway IP>>|rnext_field|${ipsec_fields}
            text|ipsec_remotenext_add|$FORM_ipsec_remotenext
            field|@TR<<Remote Subnet IP>>|rsbip_field|${ipsec_fields}
            text|ipsec_remotesbip_add|$FORM_ipsec_remotesbip
            field|@TR<<Remote Subnet Mask>>|rsbmask_field|${ipsec_fields}
            text|ipsec_remotesbmask_add|$FORM_ipsec_remotesbmask

            field|@TR<<Phase 1 DH Group>>|ikedh_field|${ipsec_fields}
            select|tunnelikedh|$FORM_ike_dh
            option|modp1024|@TR<<modp1024>>
            option|modp1536|@TR<<modp1536>>
            option|modp2048|@TR<<modp2048>>

            field|@TR<<Phase 1 Encryption>>|ikealg_field|${ipsec_fields}
            select|tunnelikealg|$FORM_ike_alg
            option|3des|@TR<<3des>>
            option|aes|@TR<<aes>>
            option|aes128|@TR<<aes128>>
            option|aes256|@TR<<aes256>>

            field|@TR<<Phase 1 Authentication>>|ikeauth_field|${ipsec_fields}
            select|tunnelikeauth|$FORM_ike_auth
            option|md5|@TR<<md5>>
            option|sha1|@TR<<sha1>>

            field|@TR<<Phase 1 SA Life Time(s)>>|ikelt_field|${ipsec_fields}
            text|tunnelikelifetime|$FORM_ikelifetime

            field|@TR<<Perfect Forward Secrecy>>|pfs_field|${ipsec_fields}
            checkbox|tunnelpfs|$FORM_pfs|${pfs_flages}

            field|@TR<<Phase 2 DH Group>>|espdh_field|${ipsec_fields}
            select|tunnelespdh|$FORM_esp_dh
            option|modp1024|@TR<<modp1024>>
            option|modp1536|@TR<<modp1536>>
            option|modp2048|@TR<<modp2048>>

            field|@TR<<Phase 2 Encryption>>|espalg_field|${ipsec_fields}
            select|tunnelespalg|$FORM_esp_alg
            option|3des|@TR<<3des>>
            option|aes|@TR<<aes>>
            option|aes128|@TR<<aes128>>
            option|aes256|@TR<<aes256>>

            field|@TR<<Phase 2 Authentication>>|espauth_field|${ipsec_fields}
            select|tunnelespauth|$FORM_esp_auth
            option|md5|@TR<<md5>>
            option|sha1|@TR<<sha1>>

            field|@TR<<Phase 2 SA Life Time(s)>>|espkl_field|${ipsec_fields}
            text|tunnelkeylife|$FORM_keylife
            field|@TR<<Preshared Key>>|ikey_field|${ipsec_fields}
            text|tunnelkey|$FORM_ipsec_key
            field|@TR<<DPD Delay(s)>>|dpdd_field|${ipsec_fields}
            text|tunneldpddelay|$FORM_dpddelay

            field|@TR<<DPD Timeout(s)>>|dpdt_field|${ipsec_fields}
            text|tunneldpdtimeout|$FORM_dpdtimeout

            field|@TR<<DPD Action>>|dpda_field|${ipsec_fields}
            select|tunneldpdaction|$FORM_dpdaction
            option|hold|@TR<<hold>>
            option|clear|@TR<<clear>>
            end_form"

append forms "$add_form" "$N"

#add_form="start_form|@TR<<Multicast Route Setup>>
#            field|@TR<<Enable>>
#            onclick|mroutebutton
#            checkbox|enablemroute_add|$FORM_enablemroute|${mroute_flages}
#            onclick|
#            field|@TR<<Name>>|mrn_field|${mroute_fields}
#	    text|mroutename_add|$FORM_mroutename
#            field|@TR<<Local IP Address>>|mrlip_field|${mroute_fields}
#	    text|mroute_localip_add|$FORM_mroute_localip
#            field|@TR<<Local Subnet Mask>>|mrlm_field|${mroute_fields}
#	    text|mroute_localmask_add|$FORM_mroute_localmask
#            field|@TR<<Remote IP Address>>|mrrip_field|${mroute_fields}
#	    text|mroute_remoteip_add|$FORM_mroute_remoteip
#            field|@TR<<Remote Subnet Mask>>|mrrm_field|${mroute_fields}
#	    text|mroute_remotemask_add|$FORM_mroute_remotemask
#            field|@TR<<Source IP Address>>|mrsip_field|${mroute_fields}
#	    text|mroute_sip_add|$FORM_mroute_sip
#            field|@TR<<Source Subnet Mask>>|mrsm_field|${mroute_fields}
#	    text|mroute_smask_add|$FORM_mroute_smask
#            field|@TR<<Group IP>>|mrgip_field|${mroute_fields}
#	    text|mroute_gip_add|$FORM_mroute_gip
#            end_form"
#append forms "$add_form" "$N"

header_nfr "Network" "GRE" "@TR<<Edit a Tunnel>>" "" "$SCRIPT_NAME" "" "" "gre-summary.sh"

cat <<EOF
<script type="text/javascript" src="/webif.js"></script>
<script type="text/javascript">
<!--
function mroutebutton()
{
	var v;
	v = checked('enablemroute_add_Enable')
        set_visible('mrn_field', v);
        set_visible('mrlip_field', v);
        set_visible('mrlm_field', v);
        set_visible('mrrip_field', v);
        set_visible('mrrm_field', v);
        set_visible('mrsip_field', v);
        set_visible('mrsm_field', v);
        set_visible('mrgip_field', v);

	hide('save');
	show('save');
}
function ipsecbutton()
{
	var v;
        var vl;
        var vr;
	v = isset('enableipsec_add','Disable')
        if (v) {
        hide('mode_field');
        hide('ikealg_field');
        hide('ikeauth_field');
        hide('ikedh_field');
        hide('espalg_field');
        hide('espauth_field');
        hide('espdh_field');
        hide('ikelt_field');
        hide('espkl_field');
        hide('dpdd_field');
        hide('dpdt_field');
        hide('dpda_field');
        hide('pfs_field');
        hide('ikey_field');
        hide('lgwip_field');
        hide('lsid_field');
        hide('rgwip_field');
        hide('rsid_field');
        hide('lsbip_field');
        hide('lsbmask_field');
        hide('lsip_field');
        hide('rsbip_field');
        hide('rsbmask_field');
        hide('lnext_field');
        hide('rnext_field');
        hide('lgwtype_field');
        hide('rgwtype_field');
        hide('aggr_field');
        } 
        else{
        show('mode_field');
        show('ikealg_field');
        show('ikeauth_field');
        show('ikedh_field');
        show('espalg_field');
        show('espauth_field');
        show('espdh_field');
        show('ikelt_field');
        show('espkl_field');
        show('dpdd_field');
        show('dpdt_field');
        show('dpda_field');
        show('pfs_field');
        show('ikey_field');
        show('lsbip_field');
        show('lsbmask_field');
        show('lsip_field');
        show('rsbip_field');
        show('rsbmask_field');
        show('lnext_field');
        show('rnext_field');
        show('lgwtype_field');
        show('rgwtype_field');
        vl = isset('tunnellefttype', 'ip-only');
        if (vl) 
            hide('lsid_field')
        else
            show('lsid_field')
        vr = isset('tunnelrighttype', 'ip-only');
        if (vr) 
            hide('rsid_field')
        else
            show('rsid_field')
        vl = isset('tunnellefttype', 'dip-id');
        if (vl) 
            hide('lgwip_field')
        else
            show('lgwip_field')
        vr = isset('tunnelrighttype', 'dip-id');
        if (vr) 
            hide('rgwip_field')
        else
            show('rgwip_field')
        show('aggr_field');
        }

	hide('save');
	show('save');
}
function modechange(sitem)
{
     var v;
     if (sitem.name == "tunnellefttype") {
        v = isset('tunnellefttype', 'ip-only');
        if (v) {
           hide('lsid_field');
           show('lgwip_field');
        }
        else {
           v = isset('tunnellefttype', 'ip-id');
           show('lsid_field');
           set_visible('lgwip_field', v);
        }
     } 
     else if (sitem.name == "tunnelrighttype") {
        v = isset('tunnelrighttype', 'ip-only');
        if (v) {
           show('rgwip_field');
           hide('rsid_field');
        }
        else {
           v = isset('tunnelrighttype', 'ip-id');
           show('rsid_field');
           set_visible('rgwip_field', v);
        }
     }
         
     hide('save');
     show('save');
}
-->
</script>

EOF

display_form <<EOF
$forms
EOF

footer_vpn "gre-summary.sh" "edit" "$add_gre_cfg" ?>
