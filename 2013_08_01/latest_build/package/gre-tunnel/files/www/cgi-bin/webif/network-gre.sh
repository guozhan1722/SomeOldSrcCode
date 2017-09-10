#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

#save a report
if ! empty "$FORM_submit" ; then
    form_valid="false"
    FORM_grename="$FORM_grename_add"
    res=$(uci get ipsec.${FORM_grename})

    [ "$res" = "x2ctunnel" -o "$res" = "s2stunnel" -o "$res" = "x2stunnel" ] && {
              name_error="@TR<<Conflict name>>"
    } || {
              name_error=""
    }
    res=$(uci get gre-tunnels.${FORM_grename})

    [ "$res" = "gretunnel" ] && {
              name_error="@TR<<Conflict name>>"
    }
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
            form_valid="true"
	    uci_add gre-tunnels  gretunnel "$FORM_grename_add"; add_gre_cfg="$CONFIG_SECTION"
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
        }
fi
if [ "$form_valid" = "false" ]; then
    form_valid="true"
else
    FORM_inf="br-wan2"
    FORM_pfs="0"
    FORM_ike_alg="3des"
    FORM_ike_auth="md5"
    FORM_ike_dh="modp1024"
    FORM_ikelifetime="28800"
    FORM_esp_alg="3des"
    FORM_esp_auth="md5"
    FORM_esp_dh="modp1024"
    FORM_keylife="3600"
    FORM_ipsec_key=""
    FORM_dpddelay="32"
    FORM_dpdtimeout="122"
    FORM_dpdaction="hold"
    FORM_lefttype="ip-only"
    FORM_righttype="ip-only"
    FORM_aggr="no"
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

add_form="start_form|@TR<<IPsec Setup>>
            field|@TR<<Enable>>
            onchange|ipsecbutton
            select|enableipsec_add|$FORM_enableipsec
            option|Disable|@TR<<None>>
            option|bgs|@TR<<Before GRE Setup>>
            option|ags|@TR<<After GRE Setup>>
            onchange|
            field|@TR<<Tunnel Mode>>|mode_field|hidden
            select|ipsec_mode_add|$FORM_ipsec_mode
            option|transport|@TR<<Transport>>
            option|tunnel|@TR<<Tunnel>>
            field|@TR<<Aggressive Mode>>|aggr_field|hidden
            checkbox|tunnelaggr|$FORM_aggr|yes||none

            field|@TR<<Local Security Gateway Type>>|lgwtype_field|hidden
            onchange|modechange
            select|tunnellefttype|$FORM_lefttype
            option|ip-only|@TR<<IP Only>>
            option|ip-id|@TR<<IP + Server ID>>
            option|dip-id|@TR<<Dynamic IP + Server ID>>
            onchange|
            field|@TR<<Local Gateway IP>>|lgwip_field|hidden
            text|ipsec_localgwip_add|$FORM_ipsec_localgwip
            field|@TR<<Local Server Id>>|lsid_field|hidden
            text|ipsec_localsid_add|$FORM_ipsec_localsid
            field|@TR<<Local Next-hop Gateway IP>>|lnext_field|hidden
            text|ipsec_localnext_add|$FORM_ipsec_localnext
            field|@TR<<Local Subnet IP>>|lsbip_field|hidden
            text|ipsec_localsbip_add|$FORM_ipsec_localsbip
            field|@TR<<Local Subnet Mask>>|lsbmask_field|hidden
            text|ipsec_localsbmask_add|$FORM_ipsec_localsbmask
            field|@TR<<Local Subnet Gateway>>|lsip_field|hidden
            text|ipsec_localsip_add|$FORM_ipsec_localsip

            field|@TR<<Remote Security Gateway Type>>|rgwtype_field|hidden
            onchange|modechange
            select|tunnelrighttype|$FORM_righttype
            option|ip-only|@TR<<IP Only>>
            option|ip-id|@TR<<IP + Server ID>>
            option|dip-id|@TR<<Dynamic IP + Server ID>>
            onchange|
            field|@TR<<Remote Gateway IP>>|rgwip_field|hidden
            text|ipsec_remotegwip_add|$FORM_ipsec_remotegwip
            field|@TR<<Remote Server Id>>|rsid_field|hidden
            text|ipsec_remotesid_add|$FORM_ipsec_remotesid
            field|@TR<<Remote Next-hop Gateway IP>>|rnext_field|hidden
            text|ipsec_remotenext_add|$FORM_ipsec_remotenext
            field|@TR<<Remote Subnet IP>>|rsbip_field|hidden
            text|ipsec_remotesbip_add|$FORM_ipsec_remotesbip
            field|@TR<<Remote Subnet Mask>>|rsbmask_field|hidden
            text|ipsec_remotesbmask_add|$FORM_ipsec_remotesbmask

            field|@TR<<Phase 1 DH Group>>|ikedh_field|hidden
            select|tunnelikedh|$FORM_ike_dh
            option|modp1024|@TR<<modp1024>>
            option|modp1536|@TR<<modp1536>>
            option|modp2048|@TR<<modp2048>>

            field|@TR<<Phase 1 Encryption>>|ikealg_field|hidden
            select|tunnelikealg|$FORM_ike_alg
            option|3des|@TR<<3des>>
            option|aes|@TR<<aes>>
            option|aes128|@TR<<aes128>>
            option|aes256|@TR<<aes256>>

            field|@TR<<Phase 1 Authentication>>|ikeauth_field|hidden
            select|tunnelikeauth|$FORM_ike_auth
            option|md5|@TR<<md5>>
            option|sha1|@TR<<sha1>>

            field|@TR<<Phase 1 SA Life Time(s)>>|ikelt_field|hidden
            text|tunnelikelifetime|$FORM_ikelifetime

            field|@TR<<Perfect Forward Secrecy>>|pfs_field|hidden
            checkbox|tunnelpfs|$FORM_pfs|yes||none

            field|@TR<<Phase 2 DH Group>>|espdh_field|hidden
            select|tunnelespdh|$FORM_esp_dh
            option|modp1024|@TR<<modp1024>>
            option|modp1536|@TR<<modp1536>>
            option|modp2048|@TR<<modp2048>>

            field|@TR<<Phase 2 Encryption>>|espalg_field|hidden
            select|tunnelespalg|$FORM_esp_alg
            option|3des|@TR<<3des>>
            option|aes|@TR<<aes>>
            option|aes128|@TR<<aes128>>
            option|aes256|@TR<<aes256>>

            field|@TR<<Phase 2 Authentication>>|espauth_field|hidden
            select|tunnelespauth|$FORM_esp_auth
            option|md5|@TR<<md5>>
            option|sha1|@TR<<sha1>>

            field|@TR<<Phase 2 SA Life Time(s)>>|espkl_field|hidden
            text|tunnelkeylife|$FORM_keylife
            field|@TR<<Preshared Key>>|ikey_field|hidden
            text|tunnelkey|$FORM_ipsec_key
            field|@TR<<DPD Delay(s)>>|dpdd_field|hidden
            text|tunneldpddelay|$FORM_dpddelay

            field|@TR<<DPD Timeout(s)>>|dpdt_field|hidden
            text|tunneldpdtimeout|$FORM_dpdtimeout

            field|@TR<<DPD Action>>|dpda_field|hidden
            select|tunneldpdaction|$FORM_dpdaction
            option|hold|@TR<<hold>>
            option|clear|@TR<<clear>>
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

footer_vpn "gre-summary.sh" "add" "$add_gre_cfg" ?>
