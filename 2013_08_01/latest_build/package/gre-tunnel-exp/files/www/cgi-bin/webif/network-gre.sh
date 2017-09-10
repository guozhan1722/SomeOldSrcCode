#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

#save a report
if ! empty "$FORM_submit" ; then
    form_valid="false"
    FORM_grename="$FORM_grename_add"
    [[ "${FORM_grename}" == "${FORM_grename% *}" ]] || name_error="@TR<<There are some space characters in gre name field>>"

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

#next step to implement mroute
    #if empty "$FORM_enablemroute_add" || ! equal $FORM_enablemroute_add "Enable"; then 
    #   FORM_enablemroute_add="Disable" 
    #fi  
    #FORM_enablemroute="$FORM_enablemroute_add"
    #FORM_mroutename="$FORM_mroutename_add"
    #FORM_mroute_localip="$FORM_mroute_localip_add"
    #FORM_mroute_localmask="$FORM_mroute_localmask_add"
    #FORM_mroute_remoteip="$FORM_mroute_remoteip_add"
    #FORM_mroute_remotemask="$FORM_mroute_remotemask_add"
    #FORM_mroute_sip="$FORM_mroute_sip_add"
    #FORM_mroute_smask="$FORM_mroute_smask_add"
    #FORM_mroute_gip="$FORM_mroute_gip_add"
    


validate <<EOF
string|FORM_grename_add|@TR<<Name>>|required|$FORM_grename_add
merror|FORM_grename_add|@TR<<Name>>||$name_error
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
	    #uci_add gre-tunnels  gretunnel "$FORM_grename_add"; add_gre_cfg="$CONFIG_SECTION"
            uci set gre-tunnels.${FORM_grename_add}=gretunnel 
            if [ "$?" = "1" ]
            then
                 name_error="@TR<<Name can only contain the characters 'a-z', 'A-Z', '0-9' and '_'>>"
validate <<EOF
merror|FORM_grename_add|@TR<<Name>>||$name_error
EOF
            else
            form_valid="true"
            add_gre_cfg="$FORM_grename_add"
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
	    #uci_set gre-tunnels "$add_gre_cfg" enablemroute         "$FORM_enablemroute_add"
            #uci_set gre-tunnels "$add_gre_cfg" mroutename           "$FORM_mroutename_add"
            #uci_set gre-tunnels "$add_gre_cfg" mroute_localip       "$FORM_mroute_localip_add"
            #uci_set gre-tunnels "$add_gre_cfg" mroute_localmask     "$FORM_mroute_localmask_add"
            #uci_set gre-tunnels "$add_gre_cfg" mroute_remoteip      "$FORM_mroute_remoteip_add"
            #uci_set gre-tunnels "$add_gre_cfg" mroute_remotemask    "$FORM_mroute_remotemask_add"
            #uci_set gre-tunnels "$add_gre_cfg" mroute_sip           "$FORM_mroute_sip_add"
            #uci_set gre-tunnels "$add_gre_cfg" mroute_smask         "$FORM_mroute_smask_add"
            #uci_set gre-tunnels "$add_gre_cfg" mroute_gip           "$FORM_mroute_gip_add"
            fi
        }
fi
if [ "$form_valid" = "false" ]; then
    form_valid="true"
fi
      
add_form="start_form|
            field|@TR<<Name>>
	    text|grename_add|$FORM_grename
            field|@TR<<Enable>>
            checkbox|local_status_add|$FORM_local_status|Enable||checked

            field|@TR<<Multicast>>
	    checkbox|multicast_add|$FORM_multicast|Enable
            field|@TR<<TTL>>
	    text|ttl_add|$FORM_ttl
            field|@TR<<Key>>
	    text|key_add|$FORM_key

            field|@TR<<ARP>>
	    checkbox|arp_add|$FORM_arp|Enable
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
#            checkbox|enablemroute_add|$FORM_enablemroute|Enable
#            onclick|
#            field|@TR<<Name>>|mrn_field|hidden
#	    text|mroutename_add|$FORM_mroutename
#            field|@TR<<Local IP Address>>|mrlip_field|hidden
#	    text|mroute_localip_add|$FORM_mroute_localip
#            field|@TR<<Local Subnet Mask>>|mrlm_field|hidden
#	    text|mroute_localmask_add|$FORM_mroute_localmask
#            field|@TR<<Remote IP Address>>|mrrip_field|hidden
#	    text|mroute_remoteip_add|$FORM_mroute_remoteip
#            field|@TR<<Remote Subnet Mask>>|mrrm_field|hidden
#	    text|mroute_remotemask_add|$FORM_mroute_remotemask
#            field|@TR<<Source IP Address>>|mrsip_field|hidden
#	    text|mroute_sip_add|$FORM_mroute_sip
#            field|@TR<<Source Subnet Mask>>|mrsm_field|hidden
#	    text|mroute_smask_add|$FORM_mroute_smask
#            field|@TR<<Group IP>>|mrgip_field|hidden
#	    text|mroute_gip_add|$FORM_mroute_gip
#            end_form"
#append forms "$add_form" "$N"

header_nfr "Network" "GRE" "@TR<<Add a New Tunnel>>" "" "$SCRIPT_NAME" "" "" "gre-summary.sh"
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

footer_vpn "gre-summary.sh" "add" "$add_gre_cfg" ?>
