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
#    config_get FORM_enablemroute            $FORM_grename enablemroute   
#    config_get FORM_mroutename              $FORM_grename mroutename
#    config_get FORM_mroute_localip          $FORM_grename mroute_localip
#    config_get FORM_mroute_localmask        $FORM_grename mroute_localmask
#    config_get FORM_mroute_remoteip         $FORM_grename mroute_remoteip
#    config_get FORM_mroute_remotemask       $FORM_grename mroute_remotemask
#    config_get FORM_mroute_sip              $FORM_grename mroute_sip
#    config_get FORM_mroute_smask            $FORM_grename mroute_smask
#    config_get FORM_mroute_gip              $FORM_grename mroute_gip

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
-->
</script>

EOF

display_form <<EOF
$forms
EOF

footer_vpn "gre-summary.sh" "edit" "$add_gre_cfg" ?>
