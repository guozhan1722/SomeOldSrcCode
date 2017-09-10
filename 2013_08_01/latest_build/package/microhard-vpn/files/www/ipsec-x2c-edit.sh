#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

#fetch local wan2 ip
FORM_left2="$(ifconfig br-wan2 | grep 'inet addr' | awk '{print $2}' | sed -e 's/.*://')"
#fetch local wan ip
FORM_left="$(ifconfig br-wan | grep 'inet addr' | awk '{print $2}' | sed -e 's/.*://')"

#save an x2c tunnel
if ! empty "$FORM_submit"; then
    FORM_x2c_tunnel_name=$(cat /tmp/x2c/editname)
    FORM_inf="$FORM_tunnelinf"
    FORM_authby="$FORM_tunnelauthby"
    FORM_leftcert="$FORM_tunnelleftcert"
    FORM_leftpkey="$FORM_tunnelleftpkey"
    FORM_rightcert="$FORM_tunnelrightcert"
    FORM_lefttype="$FORM_tunnellefttype"
    FORM_righttype="$FORM_tunnelrighttype"
    FORM_leftid="$FORM_tunnelleftid"
    FORM_right="$FORM_tunnelright"
    FORM_rightid="$FORM_tunnelrightid"
    FORM_leftnexthop="$FORM_tunnellefthop"
    FORM_rightnexthop="$FORM_tunnelrighthop"
    FORM_rightsubnet="$FORM_tunnelrightsubnet"
    FORM_rightmask="$FORM_tunnelrightmask"
    FORM_authname="$FORM_tunnelauthname"
    FORM_pppidle="$FORM_tunnelpppidle"
    if empty "$FORM_tunnelpap" || ! equal $FORM_tunnelpap "yes"; then 
       FORM_tunnelpap="no" 
    fi
    FORM_pap="$FORM_tunnelpap"
    if empty "$FORM_tunnelchap" || ! equal $FORM_tunnelchap "yes"; then 
       FORM_tunnelchap="no" 
    fi
    FORM_chap="$FORM_tunnelchap"
    if empty "$FORM_tunnelenabled" || ! equal $FORM_tunnelenabled "1"; then 
       FORM_tunnelenabled="0" 
    fi
    FORM_enabled="$FORM_tunnelenabled"
    if empty "$FORM_tunnelpfs" || ! equal $FORM_tunnelpfs "yes"; then 
       FORM_tunnelpfs="no" 
    fi
    FORM_pfs="$FORM_tunnelpfs"
    if empty "$FORM_tunneladvanced" || ! equal $FORM_tunneladvanced "yes"; then
       FORM_tunneladvanced="no"
    fi
    FORM_advanced="$FORM_tunneladvanced"

    FORM_ike_alg="$FORM_tunnelikealg"
    FORM_ike_auth="$FORM_tunnelikeauth"
    FORM_ike_dh="$FORM_tunnelikedh"
    FORM_esp_alg="$FORM_tunnelespalg"
    FORM_esp_auth="$FORM_tunnelespauth"
    FORM_esp_dh="$FORM_tunnelespdh"
    FORM_ikelifetime="$FORM_tunnelikelifetime"
    FORM_keylife="$FORM_tunnelkeylife"
    FORM_dpdaction="$FORM_tunneldpdaction"
    FORM_dpddelay="$FORM_tunneldpddelay"
    FORM_dpdtimeout="$FORM_tunneldpdtimeout"
    if empty "$FORM_tunnelredial" || ! equal $FORM_tunnelredial "yes"; then 
       $FORM_tunnelredial="no" 
    fi
    FORM_redial="$FORM_tunnelredial"
    FORM_maxredial="$FORM_tunnelmaxredial"
    FORM_rtimeout="$FORM_tunnelrtimeout"

    if [ "$FORM_authby" = "secret" ]; then
         FORM_key="$FORM_tunnelkey"
validate <<EOF
ip|FORM_tunnelright|@TR<<Gateway IP Address>>|required|$FORM_right
ip|FORM_tunnelrightsubnet|@TR<<Group Subnet IP>>|required|$FORM_rightsubnet
netmask|FORM_tunnelrightmask|@TR<<Group Subnet Mask>>|required|$FORM_rightmask
string|FORM_tunnelkey|@TR<<Preshared Key>>|required|$FORM_key
string|FORM_tunnelauthname|@TR<<User Name>>|required|$$FORM_authname
EOF
    else
         FORM_key="$FORM_tunnelprivate"
validate <<EOF
ip|FORM_tunnelright|@TR<<Gateway IP Address>>|required|$FORM_right
ip|FORM_tunnelrightsubnet|@TR<<Group Subnet IP>>|required|$FORM_rightsubnet
netmask|FORM_tunnelrightmask|@TR<<Group Subnet Mask>>|required|$FORM_rightmask
string|FORM_tunnelprivate|@TR<<Private Key Password>>|required|$FORM_key
string|FORM_tunnelauthname|@TR<<User Name>>|required|$$FORM_authname
EOF
    fi
           
    equal "$?" 0 && {
	    add_x2c_cfg="$FORM_x2c_tunnel_name"
 	    uci_set "ipsec" "$add_x2c_cfg" "interface"       "$FORM_inf"
            uci_set "ipsec" "$add_x2c_cfg" "authby"          "$FORM_authby"
            uci_set "ipsec" "$add_x2c_cfg" "leftcert"        "$FORM_leftcert"
            uci_set "ipsec" "$add_x2c_cfg" "leftpkey"        "$FORM_leftpkey"
            uci_set "ipsec" "$add_x2c_cfg" "rightcert"       "$FORM_rightcert"
            uci_set "ipsec" "$add_x2c_cfg" "lefttype"        "$FORM_lefttype"
            uci_set "ipsec" "$add_x2c_cfg" "leftid"          "$FORM_leftid"
            uci_set "ipsec" "$add_x2c_cfg" "righttype"       "$FORM_righttype"
            uci_set "ipsec" "$add_x2c_cfg" "right"           "$FORM_right"
            uci_set "ipsec" "$add_x2c_cfg" "rightid"         "$FORM_rightid"
            uci_set "ipsec" "$add_x2c_cfg" "rightsubnet"     "$FORM_rightsubnet"
            uci_set "ipsec" "$add_x2c_cfg" "rightmask"       "$FORM_rightmask"
            uci_set "ipsec" "$add_x2c_cfg" "tunnel_type"     "transport"
            uci_set "ipsec" "$add_x2c_cfg" "enabled"         "$FORM_enabled"
            uci_set "ipsec" "$add_x2c_cfg" "auto"            "route"
            uci_set "ipsec" "$add_x2c_cfg" "keyingtries"     "3"
            uci_set "ipsec" "$add_x2c_cfg" "rekey"           "yes"
            uci_set "ipsec" "$add_x2c_cfg" "pfs"             "$FORM_pfs"
            uci_set "ipsec" "$add_x2c_cfg" "ike_alg"         "$FORM_ike_alg"
            uci_set "ipsec" "$add_x2c_cfg" "ike_auth"        "$FORM_ike_auth"
            uci_set "ipsec" "$add_x2c_cfg" "ike_dh"          "$FORM_ike_dh"
            uci_set "ipsec" "$add_x2c_cfg" "ikelifetime"     "$FORM_ikelifetime"
            uci_set "ipsec" "$add_x2c_cfg" "esp_alg"         "$FORM_esp_alg"
            uci_set "ipsec" "$add_x2c_cfg" "esp_auth"        "$FORM_esp_auth"
            uci_set "ipsec" "$add_x2c_cfg" "esp_dh"          "$FORM_esp_dh"
            uci_set "ipsec" "$add_x2c_cfg" "keylife"         "$FORM_keylife"
            uci_set "ipsec" "$add_x2c_cfg" "key"             "$FORM_key"
            uci_set "ipsec" "$add_x2c_cfg" "dpddelay"        "$FORM_dpddelay"
            uci_set "ipsec" "$add_x2c_cfg" "dpdtimeout"      "$FORM_dpdtimeout"
            uci_set "ipsec" "$add_x2c_cfg" "dpdaction"       "$FORM_dpdaction"
            uci_set "ipsec" "$add_x2c_cfg" "pap"             "$FORM_pap"
            uci_set "ipsec" "$add_x2c_cfg" "chap"            "$FORM_chap"
            uci_set "ipsec" "$add_x2c_cfg" "pppidle"         "$FORM_pppidle"
            uci_set "ipsec" "$add_x2c_cfg" "authname"        "$FORM_authname"
            uci_set "ipsec" "$add_x2c_cfg" "advanced"        "$FORM_advanced"
            uci_set "ipsec" "$add_x2c_cfg" "pppauth"         "yes"
            uci_set "ipsec" "$add_x2c_cfg" "redial"          "$FORM_redial"
            uci_set "ipsec" "$add_x2c_cfg" "maxredial"       "$FORM_maxredial"
            uci_set "ipsec" "$add_x2c_cfg" "rtimeout"        "$FORM_rtimeout"
            uci_set "ipsec" "$add_x2c_cfg" "leftnexthop"     "$FORM_leftnexthop"
            uci_set "ipsec" "$add_x2c_cfg" "rightnexthop"    "$FORM_rightnexthop"
            rm -rf /tmp/x2c/editname >&- 2>&-
            /usr/sbin/xl2tpd-control remove $add_x2c_cfg >/dev/null 2>&1
            /usr/sbin/ipsec auto --delete $add_x2c_cfg >/dev/null 2>&1
            echo "action=edit" > /var/run/mipsec/$add_x2c_cfg
            echo "status=waiting for connection" >> /var/run/mipsec/$add_x2c_cfg
            echo "constat=Connect" >> /var/run/mipsec/$add_x2c_cfg
   }
fi

header_vpn "VPN" "Client To Gateway" "@TR<<Client To Gateway>>" '' "$SCRIPT_NAME"
cat <<EOF
<script type="text/javascript" src="/webif.js"></script>
<script type="text/javascript">
<!--
function modechange()
{
	var v;
	v = checked('tunneladvanced')
        set_visible('ikealg_field', v);
        set_visible('ikeauth_field', v);
        set_visible('ikedh_field', v);
        set_visible('espalg_field', v);
        set_visible('espauth_field', v);
        set_visible('espdh_field', v);

	hide('save');
	show('save');
}
function infchange()
{
	var v;
	v = isset('tunnelinf', 'br-wan');
        set_visible('left_field', v);
        v = isset('tunnelinf', 'br-wan2');
        set_visible('left2_field', v);

	hide('save');
	show('save');
}
function typechange(sitem)
{
     var v;
     if (sitem.name == "tunnelrighttype") {
        v = isset('tunnelrighttype', 'ip-only');
        if (v)
           hide('rsid_field')
        else
           show('rsid_field')
     } 
     else if (sitem.name == "tunnellefttype") {
        v = isset('tunnellefttype', 'ip-only');
        if (v) {
           hide('lsid_field');
        }
        else {
           show('lsid_field');
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
-->
</script>

EOF

if ! empty "$FORM_editname"; then
    FORM_x2c_tunnel_name="$FORM_editname"
    [ -d "/tmp/x2c" ] || mkdir /tmp/x2c
    echo $FORM_editname > /tmp/x2c/editname

else
    FORM_x2c_tunnel_name=$(cat /tmp/x2c/editname)
fi

    uci_load ipsec
    config_get FORM_inf             $FORM_x2c_tunnel_name interface
    config_get FORM_rightid         $FORM_x2c_tunnel_name rightid
    config_get FORM_rightsubnet     $FORM_x2c_tunnel_name rightsubnet    
    config_get FORM_right           $FORM_x2c_tunnel_name right
    config_get FORM_rightmask       $FORM_x2c_tunnel_name rightmask
    config_get FORM_enabled         $FORM_x2c_tunnel_name enabled
    config_get FORM_advanced        $FORM_x2c_tunnel_name advanced
    config_get FORM_pfs             $FORM_x2c_tunnel_name pfs
    config_get FORM_ike_alg         $FORM_x2c_tunnel_name ike_alg
    config_get FORM_ike_auth        $FORM_x2c_tunnel_name ike_auth
    config_get FORM_ike_dh          $FORM_x2c_tunnel_name ike_dh
    config_get FORM_ikelifetime     $FORM_x2c_tunnel_name ikelifetime
    config_get FORM_esp_alg         $FORM_x2c_tunnel_name esp_alg
    config_get FORM_esp_auth        $FORM_x2c_tunnel_name esp_auth
    config_get FORM_esp_dh          $FORM_x2c_tunnel_name esp_dh
    config_get FORM_keylife         $FORM_x2c_tunnel_name keylife  
    config_get FORM_key             $FORM_x2c_tunnel_name key
    config_get FORM_dpddelay        $FORM_x2c_tunnel_name dpddelay
    config_get FORM_dpdtimeout      $FORM_x2c_tunnel_name dpdtimeout
    config_get FORM_dpdaction       $FORM_x2c_tunnel_name dpdaction
    config_get FORM_pap             $FORM_x2c_tunnel_name pap
    config_get FORM_chap            $FORM_x2c_tunnel_name chap
    config_get FORM_authname        $FORM_x2c_tunnel_name authname
    config_get FORM_pppidle         $FORM_x2c_tunnel_name pppidle
    config_get FORM_redial          $FORM_x2c_tunnel_name redial
    config_get FORM_maxredial       $FORM_x2c_tunnel_name maxredial
    config_get FORM_rtimeout        $FORM_x2c_tunnel_name rtimeout
    config_get FORM_authby          $FORM_x2c_tunnel_name authby
    config_get FORM_leftcert        $FORM_x2c_tunnel_name leftcert
    config_get FORM_leftpkey        $FORM_x2c_tunnel_name leftpkey
    config_get FORM_rightcert       $FORM_x2c_tunnel_name rightcert
    config_get FORM_lefttype        $FORM_x2c_tunnel_name lefttype
    config_get FORM_righttype       $FORM_x2c_tunnel_name righttype
    config_get FORM_leftid          $FORM_x2c_tunnel_name leftid
    config_get FORM_leftnexthop     $FORM_x2c_tunnel_name leftnexthop
    config_get FORM_rightnexthop    $FORM_x2c_tunnel_name rightnexthop

    [ -z $FORM_lefttype] && FORM_lefttype="ip-only"
    [ -z $FORM_righttype] && FORM_righttype="ip-id"
    if [ $FORM_lefttype = "ip-only" ]; then
          lsid_flage="hidden"
    else
          lsid_flage=""
    fi

    if [ $FORM_righttype = "ip-only" ]; then
          rsid_flage="hidden"
    else
          rsid_flage=""
    fi

    [ -z $FORM_authby] && FORM_authby="secret"
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

    if [ -z "$FORM_leftnexthop" ]; then
          [ "$FORM_inf" = "br-wan" ] && {
             FORM_leftnexthop="$FORM_left"
          } || {
             FORM_leftnexthop="$FORM_left2"
          }
    fi

    if [ -z "$FORM_rightnexthop" ]; then
          [ "$FORM_inf" = "br-wan" ] && {
             FORM_rightnexthop="$FORM_left"
          } || {
             FORM_rightnexthop="$FORM_left2"
          }
    fi

display_form <<EOF
start_form|@TR<<Edit a Tunnel>>
EOF

cat <<EOF
<tr><td>@TR<<Tunnel Name>></td>
<td><input name="tunnelname" value="${FORM_x2c_tunnel_name}" disabled="disabled"></td>
</tr>
EOF

display_form <<EOF
field|@TR<<Enable>>
checkbox|tunnelenabled|$FORM_enabled|1||checked
field|@TR<<Authentication>>
onchange|cachange
select|tunnelauthby|$FORM_authby
option|secret|@TR<<Preshared Key>>
option|rsasig|@TR<<X.509 CA>>
onchange|

field|@TR<<Interface>>
onchange|infchange
select|tunnelinf|$FORM_inf
option|br-wan|@TR<<WAN>>
option|br-wan2|@TR<<4G>>
infchange|
end_form

start_form|@TR<<Local Group Setup>>
field|@TR<<Local Security Gateway Type>>
onchange|typechange
select|tunnellefttype|$FORM_lefttype
option|ip-only|@TR<<IP Only>>
option|ip-id|@TR<<IP + Server ID>>
option|dip-id|@TR<<Dynamic IP + Server ID>>
onchange|
EOF

case $FORM_inf in
"br-wan")
cat <<EOF
<tr id="left2_field" style="display: none"><td>@TR<<Interface IP Address>></td>
<td><input name="tunnelleft" value="${FORM_left2}" disabled="disabled"></td>
</tr>
<tr id="left_field" style="display: "><td>@TR<<Interface IP Address>></td>
<td><input name="tunnelleft" value="${FORM_left}" disabled="disabled"></td>
</tr>
EOF
;;
"br-wan2")
cat <<EOF
<tr id="left2_field" style="display: "><td>@TR<<Interface IP Address>></td>
<td><input name="tunnelleft" value="${FORM_left2}" disabled="disabled"></td>
</tr>
<tr id="left_field" style="display: none"><td>@TR<<Interface IP Address>></td>
<td><input name="tunnelleft" value="${FORM_left}" disabled="disabled"></td>
</tr>
EOF
;;
esac

display_form <<EOF
field|@TR<<Server ID>>|lsid_field|${lsid_flage}
text|tunnelleftid|$FORM_leftid
field|@TR<<Next-hop Gateway IP>>
text|tunnellefthop|$FORM_leftnexthop
field|@TR<<Certificate>>|lcert_field|${lcert_flage}
text|tunnelleftcert|$FORM_leftcert
field|@TR<<Private Key>>|lpkey_field|${lpkey_flage}
text|tunnelleftpkey|$FORM_leftpkey
end_form

start_form|@TR<<Remote Group Setup>>
field|@TR<<Remote Security Gateway Type>>
onchange|typechange
select|tunnelrighttype|$FORM_righttype
option|ip-only|@TR<<IP Only>>
option|ip-id|@TR<<IP + Server ID>>
onchange|
field|@TR<<Gateway IP Address>>
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

start_form|@TR<<PPP Setup>>

field|@TR<<Idle time before hanging up>>
text|tunnelpppidle|$FORM_pppidle|seconds [0...65535]
field|@TR<<PAP>>
checkbox|tunnelpap|$FORM_pap|yes|Unencrypted Password|none
field|@TR<<CHAP>>
checkbox|tunnelchap|$FORM_chap|yes|Challenge Handshake Authentication Protocol|checked
field|@TR<<User Name>>
text|tunnelauthname|$FORM_authname
field|@TR<<Redial>>
checkbox|tunnelredial|$FORM_redial|yes||checked
field|@TR<<Redial attempts>>
text|tunnelmaxredial|$FORM_maxredial
field|@TR<<Time between redial attempts>>
text|tunnelrtimeout|$FORM_rtimeout
end_form

start_form|@TR<<IPSec Setup>>

field|@TR<<Phase 1 SA Life Time(s)>>
text|tunnelikelifetime|$FORM_ikelifetime

field|@TR<<Perfect Forward Secrecy>>
checkbox|tunnelpfs|$FORM_pfs|yes||none

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
select|tunneldpdaction|$FORM_dpdaction
option|hold|@TR<<hold>>
option|clear|@TR<<clear>>
EOF

[ "$FORM_advanced" = "no" ] && {
cat <<EOF
<tr>
<td colspan="2">
<input id="tunneladvanced" type="checkbox" name="tunneladvanced" value="yes" onclick="modechange(this)" />
Advanced+
</td></tr>
EOF
display_form <<EOF
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

end_form
EOF
} || {
cat <<EOF
<tr>
<td colspan="2">
<input id="tunneladvanced" type="checkbox" name="tunneladvanced" value="yes" onclick="modechange(this)" checked="checked"/>
Advanced+
</td></tr>
EOF
display_form <<EOF
field|@TR<<Phase 1 DH Group>>|ikedh_field
select|tunnelikedh|$FORM_ike_dh
option|modp1024|@TR<<modp1024>>
option|modp1536|@TR<<modp1536>>
option|modp2048|@TR<<modp2048>>

field|@TR<<Phase 1 Encryption>>|ikealg_field
select|tunnelikealg|$FORM_ike_alg
option|3des|@TR<<3des>>
option|aes|@TR<<aes>>
option|aes128|@TR<<aes128>>
option|aes256|@TR<<aes256>>

field|@TR<<Phase 1 Authentication>>|ikeauth_field
select|tunnelikeauth|$FORM_ike_auth
option|md5|@TR<<md5>>
option|sha1|@TR<<sha1>>

field|@TR<<Phase 2 DH Group>>|espdh_field
select|tunnelespdh|$FORM_esp_dh
option|modp1024|@TR<<modp1024>>
option|modp1536|@TR<<modp1536>>
option|modp2048|@TR<<modp2048>>

field|@TR<<Phase 2 Encryption>>|espalg_field
select|tunnelespalg|$FORM_esp_alg
option|3des|@TR<<3des>>
option|aes|@TR<<aes>>
option|aes128|@TR<<aes128>>
option|aes256|@TR<<aes256>>

field|@TR<<Phase 2 Authentication>>|espauth_field
select|tunnelespauth|$FORM_esp_auth
option|md5|@TR<<md5>>
option|sha1|@TR<<sha1>>

end_form
EOF
}

footer_vpn "ipsec-s2s-summary.sh" "edit_x2c" "$add_x2c_cfg" ?> 


