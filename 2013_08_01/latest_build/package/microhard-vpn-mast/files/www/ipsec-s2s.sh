#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
#fetch local wan2 ip
FORM_left2="$(ifconfig br-wan2 | grep 'inet addr' | awk '{print $2}' | sed -e 's/.*://')"
#fetch local wan ip
FORM_left="$(ifconfig br-wan | grep 'inet addr' | awk '{print $2}' | sed -e 's/.*://')"

#save a s2s tunnel
if ! empty "$FORM_submit"; then
    form_valid="false"
    FORM_s2s_tunnel_name="$FORM_tunnelname"
    FORM_dpdaction="$FORM_tunneldpdaction"

    res=$(uci get ipsec.${FORM_s2s_tunnel_name})

    [ "$res" = "x2ctunnel" -o "$res" = "s2stunnel" -o "$res" = "x2stunnel" -o "$res" = "s2stb" ] && {
              name_error="@TR<<Conflict name>>"
    } || {
              name_error=""
    }
    res=$(uci get gre-tunnels.${FORM_s2s_tunnel_name})

    [ "$res" = "gretunnel" ] && {
              name_error="@TR<<Conflict name>>"
    }
    FORM_inf="$FORM_tunnelinf"
    FORM_binf="$FORM_tunnelbinf"

    if [ $FORM_dpdaction = "restart_by_peer" ]
    then
         res=$(uci get ipsec.${FORM_s2s_tunnel_name}_tb)

         [ "$res" = "x2ctunnel" -o "$res" = "s2stunnel" -o "$res" = "x2stunnel" ] && {
              name_error="@TR<<Conflict name>>"
         } || {
              name_error=""
         }
         res=$(uci get gre-tunnels.${FORM_s2s_tunnel_name}_tb)

         [ "$res" = "gretunnel" ] && {
              name_error="@TR<<Conflict name>>"
         }
    fi        

    [[ "${FORM_s2s_tunnel_name}" == "${FORM_s2s_tunnel_name% *}" ]] || name_error="@TR<<There are some space characters in tunnel name field>>"

    FORM_authby="$FORM_tunnelauthby"
    FORM_leftcert="$FORM_tunnelleftcert"
    FORM_leftpkey="$FORM_tunnelleftpkey"
    FORM_rightcert="$FORM_tunnelrightcert"
    FORM_lefttype="$FORM_tunnellefttype"
    FORM_righttype="$FORM_tunnelrighttype"
    FORM_leftnexthop="$FORM_tunnellefthop"
    FORM_rightnexthop="$FORM_tunnelrighthop"
    FORM_leftsubnet="$FORM_tunnelleftsubnet"
    FORM_leftsource="$FORM_tunnelleftsource"
    FORM_right="$FORM_tunnelright"
    FORM_bright="$FORM_tunnelbright"
    FORM_rightid="$FORM_tunnelrightid"
    FORM_leftid="$FORM_tunnelleftid"
    FORM_rightsubnet="$FORM_tunnelrightsubnet"
    FORM_leftmask="$FORM_tunnelleftmask"
    FORM_rightmask="$FORM_tunnelrightmask"
    if empty "$FORM_tunnelenabled" || ! equal $FORM_tunnelenabled "1"; then 
       FORM_tunnelenabled="0" 
    fi
    FORM_enabled="$FORM_tunnelenabled"
    if empty "$FORM_tunnelpfs" || ! equal $FORM_tunnelpfs "yes"; then 
       FORM_tunnelpfs="no" 
    fi
    FORM_pfs="$FORM_tunnelpfs"
    if empty "$FORM_tunnelaggr" || ! equal $FORM_tunnelaggr "yes"; then 
       FORM_tunnelaggr="no" 
    fi
    FORM_aggr="$FORM_tunnelaggr"
    FORM_ike_alg="$FORM_tunnelikealg"
    FORM_ike_auth="$FORM_tunnelikeauth"
    FORM_ike_dh="$FORM_tunnelikedh"
    FORM_esp_alg="$FORM_tunnelespalg"
    FORM_esp_auth="$FORM_tunnelespauth"
    FORM_esp_dh="$FORM_tunnelespdh"
    FORM_ikelifetime="$FORM_tunnelikelifetime"
    FORM_keylife="$FORM_tunnelkeylife"
    
    FORM_dpddelay="$FORM_tunneldpddelay"
    FORM_dpdtimeout="$FORM_tunneldpdtimeout"
    FORM_vtbit="$FORM_tunnelvtbit"
    FORM_phase2="$FORM_tunnelphase2"

    if [ "$FORM_authby" = "secret" ]; then
         FORM_key="$FORM_tunnelkey"
validate <<EOF
string|FORM_tunnelname|@TR<<Tunnel Name>>|required|$FORM_s2s_tunnel_name
merror|FORM_tunnelname|@TR<<Tunnel Name>>||$name_error
ip|FORM_tunnelrightsubnet|@TR<<Group Subnet IP>>|required|$FORM_rightsubnet
ip|FORM_tunnelleftsubnet|@TR<<Group Subnet IP>>|required|$FORM_leftsubnet
ip|FORM_tunnelleftsource|@TR<<Group Subnet Gateway>>|required|$FORM_leftsource
netmask|FORM_tunnelrightmask|@TR<<Group Subnet Mask>>|required|$FORM_rightmask
netmask|FORM_tunnelleftmask|@TR<<Group Subnet Mask>>|required|$FORM_leftmask
string|FORM_tunnelkey|@TR<<Preshared Key>>|required|$FORM_key
EOF
    else
         FORM_key="$FORM_tunnelprivate"
validate <<EOF
string|FORM_tunnelname|@TR<<Tunnel Name>>|required|$FORM_s2s_tunnel_name
merror|FORM_tunnelname|@TR<<Tunnel Name>>||$name_error
ip|FORM_tunnelrightsubnet|@TR<<Group Subnet IP>>|required|$FORM_rightsubnet
ip|FORM_tunnelleftsubnet|@TR<<Group Subnet IP>>|required|$FORM_leftsubnet
ip|FORM_tunnelleftsource|@TR<<Group Subnet Gateway>>|required|$FORM_leftsource
netmask|FORM_tunnelrightmask|@TR<<Group Subnet Mask>>|required|$FORM_rightmask
netmask|FORM_tunnelleftmask|@TR<<Group Subnet Mask>>|required|$FORM_leftmask
string|FORM_tunnelprivate|@TR<<Private Key Password>>|required|$FORM_key
EOF
    fi

            
        equal "$?" 0 && {
	    #uci_add "ipsec" "s2stunnel" "$FORM_s2s_tunnel_name"; add_s2s_cfg="$CONFIG_SECTION"
            uci set ipsec.${FORM_s2s_tunnel_name}=s2stunnel
            if [ "$?" = "1" ]
            then
                 name_error="@TR<<Name can only contain the characters 'a-z', 'A-Z', '0-9' and '_'>>"
validate <<EOF
merror|FORM_tunnelname|@TR<<Tunnel Name>>||$name_error
EOF
            else
            form_valid="true"
            add_s2s_cfg="$FORM_s2s_tunnel_name"
            uci_set "ipsec" "$add_s2s_cfg" "interface"       "$FORM_inf"
            uci_set "ipsec" "$add_s2s_cfg" "binterface"      "$FORM_binf"
            uci_set "ipsec" "$add_s2s_cfg" "authby"          "$FORM_authby"
            uci_set "ipsec" "$add_s2s_cfg" "leftcert"        "$FORM_leftcert"
            uci_set "ipsec" "$add_s2s_cfg" "leftpkey"        "$FORM_leftpkey"
            uci_set "ipsec" "$add_s2s_cfg" "rightcert"       "$FORM_rightcert"
            uci_set "ipsec" "$add_s2s_cfg" "lefttype"        "$FORM_lefttype"
            uci_set "ipsec" "$add_s2s_cfg" "righttype"       "$FORM_righttype"
            uci_set "ipsec" "$add_s2s_cfg" "leftnexthop"     "$FORM_leftnexthop"
            uci_set "ipsec" "$add_s2s_cfg" "rightnexthop"    "$FORM_rightnexthop"
            uci_set "ipsec" "$add_s2s_cfg" "right"           "$FORM_right"
            uci_set "ipsec" "$add_s2s_cfg" "bright"          "$FORM_bright"
            uci_set "ipsec" "$add_s2s_cfg" "rightid"         "$FORM_rightid"
            uci_set "ipsec" "$add_s2s_cfg" "leftid"          "$FORM_leftid"
            uci_set "ipsec" "$add_s2s_cfg" "leftsubnet"      "$FORM_leftsubnet"
            uci_set "ipsec" "$add_s2s_cfg" "leftsourceip"    "$FORM_leftsource"
            uci_set "ipsec" "$add_s2s_cfg" "rightsubnet"     "$FORM_rightsubnet"
            uci_set "ipsec" "$add_s2s_cfg" "leftmask"        "$FORM_leftmask"
            uci_set "ipsec" "$add_s2s_cfg" "rightmask"       "$FORM_rightmask"
            uci_set "ipsec" "$add_s2s_cfg" "tunnel_type"     "tunnel"
            uci_set "ipsec" "$add_s2s_cfg" "enabled"         "$FORM_enabled"
            uci_set "ipsec" "$add_s2s_cfg" "auto"            "route"
            uci_set "ipsec" "$add_s2s_cfg" "keyingtries"     "3"
            uci_set "ipsec" "$add_s2s_cfg" "rekey"           "yes"
            uci_set "ipsec" "$add_s2s_cfg" "pfs"             "$FORM_pfs"
            uci_set "ipsec" "$add_s2s_cfg" "aggrmode"        "$FORM_aggr"
            uci_set "ipsec" "$add_s2s_cfg" "ike_alg"         "$FORM_ike_alg"
            uci_set "ipsec" "$add_s2s_cfg" "ike_auth"        "$FORM_ike_auth"
            uci_set "ipsec" "$add_s2s_cfg" "ike_dh"          "$FORM_ike_dh"
            uci_set "ipsec" "$add_s2s_cfg" "ikelifetime"     "$FORM_ikelifetime"
            uci_set "ipsec" "$add_s2s_cfg" "esp_alg"         "$FORM_esp_alg"
            uci_set "ipsec" "$add_s2s_cfg" "esp_auth"        "$FORM_esp_auth"
            uci_set "ipsec" "$add_s2s_cfg" "esp_dh"          "$FORM_esp_dh"
            uci_set "ipsec" "$add_s2s_cfg" "keylife"         "$FORM_keylife"
            uci_set "ipsec" "$add_s2s_cfg" "key"             "$FORM_key"
            uci_set "ipsec" "$add_s2s_cfg" "dpddelay"        "$FORM_dpddelay"
            uci_set "ipsec" "$add_s2s_cfg" "dpdtimeout"      "$FORM_dpdtimeout"
            uci_set "ipsec" "$add_s2s_cfg" "dpdaction"       "$FORM_dpdaction"
            uci_set "ipsec" "$add_s2s_cfg" "vtbit"           "$FORM_vtbit"
            uci_set "ipsec" "$add_s2s_cfg" "phase2"          "$FORM_phase2"
            if [ $FORM_dpdaction = "restart_by_peer" ]
            then
               uci_add "ipsec" "s2stb" "${FORM_s2s_tunnel_name}_tb"; add_s2s_tb="$CONFIG_SECTION"
               uci_set "ipsec" "$add_s2s_tb" "primary" "$add_s2s_cfg"
            fi
            fi
        }

fi

header_vpn "VPN" "Gateway To Gateway" "@TR<<Gateway To Gateway>>" "" "$SCRIPT_NAME"

cat <<EOF
<script type="text/javascript" src="/webif.js"></script>
<script type="text/javascript">
<!--
function modechange(sitem)
{
     var v;
     if (sitem.name == "tunnelinf") {
	v = isset('tunnelinf', 'br-wan');
        set_visible('left_field', v);
        v = isset('tunnelinf', 'br-wan2');
        set_visible('left2_field', v);
     } 
     else if (sitem.name == "tunnellefttype") {
        v = isset('tunnellefttype', 'ip-only');
        if (v)
           hide('lsid_field')
        else
           show('lsid_field')
     } 
     else if (sitem.name == "tunnelrighttype") {
        v = isset('tunnelrighttype', 'ip-only');
        if (v) {
           show('rgip_field');
           hide('rsid_field');
        }
        else {
           v = isset('tunnelrighttype', 'ip-id');
           show('rsid_field');
           set_visible('rgip_field', v);
        }
     }
         
     hide('save');
     show('save');
}
function cachange(sitem)
{
     var v;
     if (sitem.name == "tunnelauthby") {
        v = isset('tunnelauthby', 'secret');
        if (v) {
           hide('lcert_field');
           hide('rcert_field');
           hide('lpkey_field');
           hide('pkp_field');
           show('psk_field');
        }
        else {
           show('lcert_field');
           show('rcert_field');
           show('lpkey_field');
           show('pkp_field');
           hide('psk_field');
        }
     } 
         
     hide('save');
     show('save');
}
function dpdchange(sitem)
{
     var v;
     if (sitem.name == "tunneldpdaction") {
        v = isset('tunneldpdaction', 'restart_by_peer');
        if (v) {
           show('rbip_field');
           show('lbinf_field');
           show('vtbit_field');
        }
        else {
           hide('rbip_field');
           hide('lbinf_field');
           hide('vtbit_field');
        }
     } 
         
     hide('save');
     show('save');
}
-->
</script>

EOF

if [ "$form_valid" = "false" ]; then
    form_valid="true"
else
    FORM_s2s_tunnel_name=""
    FORM_inf="br-wan2"
    FORM_binf="br-wan2"
    FORM_leftsubnet=""
    FORM_leftsource=""
    FORM_leftid=""
    FORM_rightsubnet=""
    FORM_rightid=""
    FORM_leftmask="255.255.255.0"
    FORM_rightmask="255.255.255.0"
    FORM_enabled="1"
    FORM_pfs="no"
    FORM_aggr="no"
    FORM_ike_alg="3des"
    FORM_ike_auth="md5"
    FORM_ike_dh="modp1024"
    FORM_ikelifetime="28800"
    FORM_esp_alg="3des"
    FORM_esp_auth="md5"
    FORM_esp_dh="modp1024"
    FORM_keylife="3600"
    FORM_key=""
    FORM_dpddelay="32"
    FORM_dpdtimeout="122"
    FORM_dpdaction="hold"
    FORM_lefttype="ip-id"
    FORM_righttype="ip-id"
    FORM_authby="secret"
    FORM_vtbit="30"
    FORM_phase2="esp"
fi
    if [ $FORM_lefttype = "ip-only" ]; then
          lsid_flage="hidden"
    else
          lsid_flage=""
    fi

    if [ $FORM_righttype = "ip-only" ]; then
          rsid_flage="hidden"
          rgip_flage=""
    else
          rsid_flage=""
          if [ $FORM_righttype = "ip-id" ]; then
                rgip_flage=""
          else
                rgip_flage="hidden"
          fi
    fi

    if [ $FORM_authby = "secret" ]; then
          lcert_flage="hidden"
          lpkey_flage="hidden"
          rcert_flage="hidden"
          pkp_flage="hidden"
          psk_flage=""
    else
          lcert_flage=""
          rcert_flage=""
          lpkey_flage=""
          pkp_flage=""
          psk_flage="hidden"
    fi

    if [ $FORM_dpdaction = "restart_by_peer" ]; then
          rbip_flage=""
          lbinf_falge=""
          vtbit_flage=""
    else
          rbip_flage="hidden"
          lbinf_flage="hidden"
          vtbit_flage="hidden"
    fi
    [ -z $FORM_vtbit] && FORM_vtbit="30"
    [ -z $FORM_phase2] && FORM_phase2="esp"

display_form <<EOF
start_form|@TR<<Add a New Tunnel>>

field|@TR<<Tunnel Name>>
text|tunnelname|$FORM_s2s_tunnel_name
field|@TR<<Enable>>
checkbox|tunnelenabled|$FORM_enabled|1||checked
field|@TR<<Authentication>>
onchange|cachange
select|tunnelauthby|$FORM_authby
option|secret|@TR<<Preshared Key>>
option|rsasig|@TR<<X.509 CA>>
onchange|

field|@TR<<Interface>>
onchange|modechange
select|tunnelinf|$FORM_inf
option|br-wan|@TR<<WAN>>
option|br-wan2|@TR<<4G>>
onchange|
end_form

start_form|@TR<<Local Group Setup>>

field|@TR<<Local Security Gateway Type>>
onchange|modechange
select|tunnellefttype|$FORM_lefttype
option|ip-only|@TR<<IP Only>>
option|ip-id|@TR<<IP + Server ID>>
option|dip-id|@TR<<Dynamic IP + Server ID>>
onchange|
EOF

case $FORM_inf in
"br-wan")
cat <<EOF
</select></td></tr>
<tr id="left2_field" style="display: none"><td>@TR<<Interface IP Address>></td>
<td><input type="text" name="tunnelleft" value="${FORM_left2}" disabled="disabled"></td>
</tr>
<tr id="left_field" style="display: "><td>@TR<<Interface IP Address>></td>
<td><input type="text" name="tunnelleft" value="${FORM_left}" disabled="disabled"></td>
</tr>
EOF
;;
"br-wan2")
cat <<EOF
</select></td></tr>
<tr id="left2_field" style="display: "><td>@TR<<Interface IP Address>></td>
<td><input type="text" name="tunnelleft" value="${FORM_left2}" disabled="disabled"></td>
</tr>
<tr id="left_field" style="display: none"><td>@TR<<Interface IP Address>></td>
<td><input type="text" name="tunnelleft" value="${FORM_left}" disabled="disabled"></td>
</tr>
EOF
;;
esac

display_form <<EOF
field|@TR<<Server ID>>|lsid_field|${lsid_flage}
text|tunnelleftid|$FORM_leftid
field|@TR<<Next-hop Gateway IP>>
text|tunnellefthop|$FORM_leftnexthop
field|@TR<<Group Subnet IP>>
text|tunnelleftsubnet|$FORM_leftsubnet
field|@TR<<Group Subnet Mask>>
text|tunnelleftmask|$FORM_leftmask
field|@TR<<Group Subnet Gateway>>
text|tunnelleftsource|$FORM_leftsource
field|@TR<<Certificate>>|lcert_field|${lcert_flage}
text|tunnelleftcert|$FORM_leftcert
field|@TR<<Private Key>>|lpkey_field|${lpkey_flage}
text|tunnelleftpkey|$FORM_leftpkey
end_form

start_form|@TR<<Remote Group Setup>>
field|@TR<<Remote Security Gateway Type>>
onchange|modechange
select|tunnelrighttype|$FORM_righttype
option|ip-only|@TR<<IP Only>>
option|ip-id|@TR<<IP + Server ID>>
option|dip-id|@TR<<Dynamic IP + Server ID>>
onchange|

field|@TR<<Gateway IP Address>>|rgip_field|${rgip_flage}
text|tunnelright|$FORM_right
field|@TR<<Server ID>>|rsid_field|${rsid_flage}
text|tunnelrightid|$FORM_rightid
field|@TR<<Next-hop Gateway IP>>
text|tunnelrighthop|$FORM_rightnexthop
field|@TR<<Group Subnet IP>>
text|tunnelrightsubnet|$FORM_rightsubnet
field|@TR<<Group Subnet Mask>>
text|tunnelrightmask|$FORM_rightmask
field|@TR<<Certificate>>|rcert_field|${rcert_flage}
text|tunnelrightcert|$FORM_rightcert
end_form

start_form|@TR<<IPSec Setup>>
field|@TR<<Aggressive Mode>>
checkbox|tunnelaggr|$FORM_aggr|yes||none

field|@TR<<Phase 1 DH Group>>
select|tunnelikedh|$FORM_ike_dh
option|modp1024|@TR<<modp1024>>
option|modp1536|@TR<<modp1536>>
option|modp2048|@TR<<modp2048>>

field|@TR<<Phase 1 Encryption>>
select|tunnelikealg|$FORM_ike_alg
option|3des|@TR<<3des>>
option|aes|@TR<<aes>>
option|aes128|@TR<<aes128>>
option|aes256|@TR<<aes256>>

field|@TR<<Phase 1 Authentication>>
select|tunnelikeauth|$FORM_ike_auth
option|md5|@TR<<md5>>
option|sha1|@TR<<sha1>>

field|@TR<<Phase 1 SA Life Time(s)>>
text|tunnelikelifetime|$FORM_ikelifetime

field|@TR<<Perfect Forward Secrecy>>
checkbox|tunnelpfs|$FORM_pfs|yes||none

field|@TR<<Phase 2 SA Type>>
select|tunnelphase2|$FORM_phase2
option|esp|@TR<<ESP>>
option|ah|@TR<<AH>>

field|@TR<<Phase 2 DH Group>>
select|tunnelespdh|$FORM_esp_dh
option|modp1024|@TR<<modp1024>>
option|modp1536|@TR<<modp1536>>
option|modp2048|@TR<<modp2048>>

field|@TR<<Phase 2 Encryption>>
select|tunnelespalg|$FORM_esp_alg
option|3des|@TR<<3des>>
option|aes|@TR<<aes>>
option|aes128|@TR<<aes128>>
option|aes256|@TR<<aes256>>

field|@TR<<Phase 2 Authentication>>
select|tunnelespauth|$FORM_esp_auth
option|md5|@TR<<md5>>
option|sha1|@TR<<sha1>>

field|@TR<<Phase 2 SA Life Time(s)>>
text|tunnelkeylife|$FORM_keylife

field|@TR<<Preshared Key>>|psk_field|${psk_flage}
text|tunnelkey|$FORM_key
field|@TR<<Private Key Password>>|pkp_field|${pkp_flage}
text|tunnelprivate|$FORM_key

field|@TR<<DPD Delay(s)>>
text|tunneldpddelay|$FORM_dpddelay

field|@TR<<DPD Timeout(s)>>
text|tunneldpdtimeout|$FORM_dpdtimeout

field|@TR<<DPD Action>>
onchange|dpdchange
select|tunneldpdaction|$FORM_dpdaction
option|hold|@TR<<hold>>
option|clear|@TR<<clear>>

option|restart|@TR<<restart>>
option|restart_by_peer|@TR<<backup>>
onchange|

field|@TR<<Remote Backup IP Address>>|rbip_field|${rbip_flage}
text|tunnelbright|$FORM_bright

field|@TR<<Local Interface>>|lbinf_field|${lbinf_flage}
select|tunnelbinf|$FORM_binf
option|br-wan|@TR<<WAN>>
option|br-wan2|@TR<<4G>>

field|@TR<<VPN Tunnel Backup Idle Time>>|vtbit_field|${vtbit_flage}
text|tunnelvtbit|$FORM_vtbit|seconds [30...999]
end_form

EOF

footer_vpn "ipsec-s2s-summary.sh" "add_s2s" "$add_s2s_cfg" ?> 

<!--
##WEBIF:name:VPN:200:Gateway To Gateway
-->
