#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

FORM_x2s_name="M2xl2tpserver"
#fetch local wan2 ip
FORM_left2="$(ifconfig br-wan2 | grep 'inet addr' | awk '{print $2}' | sed -e 's/.*://')"

form_valid="false"
#save a s2s tunnel
if ! empty "$FORM_submit"; then
    FORM_authby="$FORM_tunnelauthby"
    FORM_lefttype="$FORM_tunnellefttype"
    FORM_leftcert="$FORM_tunnelleftcert"
    FORM_leftpkey="$FORM_tunnelleftpkey"
    FORM_localip="$FORM_tunnellocalip"
    FORM_startip="$FORM_tunnelstartip"
    FORM_endip="$FORM_tunnelendip"
    if empty "$FORM_tunnelenabled" || ! equal $FORM_tunnelenabled "1"; then 
       FORM_tunnelenabled="0" 
    fi
    FORM_enabled="$FORM_tunnelenabled"
    if empty "$FORM_tunnelpfs" || ! equal $FORM_tunnelpfs "yes"; then 
       FORM_tunnelpfs="no" 
    fi
    FORM_pfs="$FORM_tunnelpfs"
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
    if empty "$FORM_tunneladvanced" || ! equal $FORM_tunneladvanced "yes"; then
       FORM_tunneladvanced="no"
    fi
    FORM_advanced="$FORM_tunneladvanced"
    FORM_leftid="$FORM_tunnelleftid"
    if empty "$FORM_tunnelipsec" || ! equal $FORM_tunnelipsec "enable"; then
       FORM_tunnelipsec="disable"
    fi
    FORM_ipsec="$FORM_tunnelipsec"

if [ "$FORM_enabled" = "1" ]; then
    if [ "$FORM_authby" = "secret" ]; then
         FORM_key="$FORM_tunnelkey"
validate <<EOF
ip|FORM_tunnellocalip|@TR<<Server IP Address>>|required|$FORM_localip
ip|FORM_tunnelstartip|@TR<<IP Address Range Start>>|required|$FORM_startip
ip|FORM_tunnelendip|@TR<<IP Address Range End>>|required|$FORM_endip
string|FORM_tunnelkey|@TR<<Preshared Key>>||$FORM_key
EOF
    else
         FORM_key="$FORM_tunnelprivate"
validate <<EOF
ip|FORM_tunnellocalip|@TR<<Server IP Address>>|required|$FORM_localip
ip|FORM_tunnelstartip|@TR<<IP Address Range Start>>|required|$FORM_startip
ip|FORM_tunnelendip|@TR<<IP Address Range End>>|required|$FORM_endip
string|FORM_tunnelprivate|@TR<<Private Key Password>>||$FORM_key
EOF
    fi

            
        equal "$?" 0 && {
 	    uci_set "ipsec" "$FORM_x2s_name" "interface"       "br-wan2"
            uci_set "ipsec" "$FORM_x2s_name" "advanced"        "$FORM_advanced"
            uci_set "ipsec" "$FORM_x2s_name" "lefttype"        "$FORM_lefttype"
            uci_set "ipsec" "$FORM_x2s_name" "localip"         "$FORM_localip"
            uci_set "ipsec" "$FORM_x2s_name" "startip"         "$FORM_startip"
            uci_set "ipsec" "$FORM_x2s_name" "endip"           "$FORM_endip"
            uci_set "ipsec" "$FORM_x2s_name" "tunnel_type"     "transport"
            uci_set "ipsec" "$FORM_x2s_name" "enabled"         "$FORM_enabled"
            uci_set "ipsec" "$FORM_x2s_name" "authby"          "$FORM_authby"
            uci_set "ipsec" "$FORM_x2s_name" "leftcert"        "$FORM_leftcert"
            uci_set "ipsec" "$FORM_x2s_name" "leftpkey"        "$FORM_leftpkey"
            uci_set "ipsec" "$FORM_x2s_name" "auto"            "route"
            uci_set "ipsec" "$FORM_x2s_name" "keyingtries"     "3"
            uci_set "ipsec" "$FORM_x2s_name" "rekey"           "yes"
            uci_set "ipsec" "$FORM_x2s_name" "pfs"             "$FORM_pfs"
            uci_set "ipsec" "$FORM_x2s_name" "ike_alg"         "$FORM_ike_alg"
            uci_set "ipsec" "$FORM_x2s_name" "ike_auth"        "$FORM_ike_auth"
            uci_set "ipsec" "$FORM_x2s_name" "ike_dh"          "$FORM_ike_dh"
            uci_set "ipsec" "$FORM_x2s_name" "ikelifetime"     "$FORM_ikelifetime"
            uci_set "ipsec" "$FORM_x2s_name" "esp_alg"         "$FORM_esp_alg"
            uci_set "ipsec" "$FORM_x2s_name" "esp_auth"        "$FORM_esp_auth"
            uci_set "ipsec" "$FORM_x2s_name" "esp_dh"          "$FORM_esp_dh"
            uci_set "ipsec" "$FORM_x2s_name" "keylife"         "$FORM_keylife"
            uci_set "ipsec" "$FORM_x2s_name" "key"             "$FORM_key"
            uci_set "ipsec" "$FORM_x2s_name" "dpddelay"        "$FORM_dpddelay"
            uci_set "ipsec" "$FORM_x2s_name" "dpdtimeout"      "$FORM_dpdtimeout"
            uci_set "ipsec" "$FORM_x2s_name" "dpdaction"       "$FORM_dpdaction"
            uci_set "ipsec" "$FORM_x2s_name" "leftid"          "$FORM_leftid"
            uci_set "ipsec" "$FORM_x2s_name" "ipsec"           "$FORM_ipsec"
        }
else
            uci_set "ipsec" "$FORM_x2s_name" "enabled"         "$FORM_enabled"
            rm -rf /var/run/mipsec/Mxl2tpserver.status >&- 2>&-
fi
fi

header_vpn "VPN" "L2TP Server" "@TR<<L2tp Server>>" '' "$SCRIPT_NAME"
cat <<EOF
<script type="text/javascript" src="/webif.js"></script>
<script type="text/javascript">
<!--
function enableserver()
{
	var v;
        var ipsecv;
	v = checked('tunnelenabled_1')
        set_visible('server_form', v);
        set_visible('ipsec_field', v);
        if (v) {
            ipsecv = checked('tunnelipsec_enable');
            set_visible('ipsec_form', ipsecv);
        }
        else {
            set_visible('ipsec_form', v);
        }

	hide('save');
	show('save');
}
function enableipsec()
{
        var ipsecv;
	ipsecv = checked('tunnelipsec_enable');
        set_visible('ipsec_form', ipsecv);

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
           hide('lpkey_field');
           hide('pkp_field');
           show('psk_field');
        }
        else {
           show('lcert_field');
           show('lpkey_field');
           show('pkp_field');
           hide('psk_field');
        }
     } 
         
     hide('save');
     show('save');
}
function adchange(sitem)
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
function typechange(sitem)
{
     var v;
     if (sitem.name == "tunnellefttype") {
        v = isset('tunnellefttype', 'ip-only');
        if (v)
           hide('lsid_field')
        else
           show('lsid_field')
     } 
         
     hide('save');
     show('save');
}
-->
</script>

EOF
inits=$(uci get ipsec.M2xl2tpserver)
if [ "$inits" != "x2stunnel" ]; then
    FORM_localip=""
    FORM_startip=""
    FORM_endip=""
    FORM_pfs="0"
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
    FORM_dpdaction="clear"
    FORM_authby="secret"
    FORM_advanced="no"
    FORM_leftid=""
    FORM_lefttype="ip-only"
    FORM_ipsec="enable"
else
    uci_load ipsec
    config_get FORM_localip         $FORM_x2s_name localip
    config_get FORM_startip         $FORM_x2s_name startip    
    config_get FORM_endip           $FORM_x2s_name endip
    config_get FORM_enabled         $FORM_x2s_name enabled
    config_get FORM_pfs             $FORM_x2s_name pfs
    config_get FORM_ike_alg         $FORM_x2s_name ike_alg
    config_get FORM_ike_auth        $FORM_x2s_name ike_auth
    config_get FORM_ike_dh          $FORM_x2s_name ike_dh
    config_get FORM_ikelifetime     $FORM_x2s_name ikelifetime
    config_get FORM_esp_alg         $FORM_x2s_name esp_alg
    config_get FORM_esp_auth        $FORM_x2s_name esp_auth
    config_get FORM_esp_dh          $FORM_x2s_name esp_dh
    config_get FORM_keylife         $FORM_x2s_name keylife  
    config_get FORM_key             $FORM_x2s_name key
    config_get FORM_dpddelay        $FORM_x2s_name dpddelay
    config_get FORM_dpdtimeout      $FORM_x2s_name dpdtimeout
    config_get FORM_dpdaction       $FORM_x2s_name dpdaction
    config_get FORM_authby          $FORM_x2s_name authby
    config_get FORM_leftcert        $FORM_x2s_name leftcert
    config_get FORM_leftpkey        $FORM_x2s_name leftpkey
    config_get FORM_advanced        $FORM_x2s_name advanced
    config_get FORM_leftid          $FORM_x2s_name leftid
    config_get FORM_lefttype        $FORM_x2s_name lefttype
    config_get FORM_ipsec           $FORM_x2s_name ipsec
fi
    [ -z $FORM_ipsec] && FORM_ipsec="enable"
    [ -z $FORM_authby] && FORM_authby="secret"
    if [ $FORM_authby = "secret" ]; then
          lcert_flage="hidden"
          lpkey_flage="hidden"
          pkp_flage="hidden"
          psk_flage=""
    else
          lcert_flage=""
          lpkey_flage=""
          pkp_flage=""
          psk_flage="hidden"
    fi

    [ -z $FORM_advanced] && FORM_advanced="no"
    if [ $FORM_advanced = "no" ]; then
          ikedh_flage="hidden"
          ikeauth_flage="hidden"
          ikealg_flage="hidden"
          espdh_flage="hidden"
          espauth_flage="hidden"
          espalg_flage="hidden"
    else
          ikedh_flage=""
          ikeauth_flage=""
          ikealg_flage=""
          espdh_flage=""
          espauth_flage=""
          espalg_flage=""
    fi

    [ -z $FORM_lefttype] && FORM_lefttype="ip-id"
    if [ $FORM_lefttype = "ip-only" ]; then
          lsid_flage="hidden"
    else
          lsid_flage=""
    fi
    if [ "$FORM_enabled" = "1" ]; then
          ipsec_flage=""
    else
          ipsec_flage="hidden"
    fi
display_form <<EOF
start_form|
field|@TR<<Enable>>
onclick|enableserver
checkbox|tunnelenabled|$FORM_enabled|1
onclick|

field|@TR<<IPsec>>|ipsec_field|${ipsec_flage}
onclick|enableipsec
checkbox|tunnelipsec|$FORM_ipsec|enable
onclick|
end_form
EOF

if [ "$FORM_enabled" = "1" ]; then
display_form <<EOF
start_form|@TR<<Server Setup>>|server_form
field|@TR<<Local Security Gateway Type>>
onchange|typechange
select|tunnellefttype|$FORM_lefttype
option|ip-only|@TR<<IP Only>>
option|ip-id|@TR<<IP + Server ID>>
onchange|

field|@TR<<Server ID>>|lsid_field|${lsid_flage}
text|tunnelleftid|$FORM_leftid
EOF
else
display_form <<EOF
start_form|@TR<<Server Setup>>|server_form|hidden
field|@TR<<Local Security Gateway Type>>
onchange|typechange
select|tunnellefttype|$FORM_lefttype
option|ip-only|@TR<<IP Only>>
option|ip-id|@TR<<IP + Server ID>>
onchange|

field|@TR<<Server ID>>|lsid_field|${lsid_flage}
text|tunnelleftid|$FORM_leftid
EOF
fi

cat <<EOF
<tr><td>@TR<<Interface>></td>
<td><input name="tunnelinf" value="4G" disabled="disabled"></td>
</tr>
<tr><td>@TR<<Interface IP Address>></td>
<td><input name="tunnelleft" value="${FORM_left2}" disabled="disabled"></td>
</tr>
EOF

display_form <<EOF

field|@TR<<Server IP Address>>
text|tunnellocalip|$FORM_localip
field|@TR<<IP Address Range Start>>
text|tunnelstartip|$FORM_startip
field|@TR<<IP Address Range End>>
text|tunnelendip|$FORM_endip

end_form
EOF

if [ "$FORM_enabled" = "1" ]; then
   if [ "$FORM_ipsec" = "enable" ]; then
display_form <<EOF
start_form|@TR<<IPSec Setup>>|ipsec_form
EOF
   else
display_form <<EOF
start_form|@TR<<IPSec Setup>>|ipsec_form|hidden
EOF
   fi
else
display_form <<EOF
start_form|@TR<<IPSec Setup>>|ipsec_form|hidden
EOF
fi

display_form <<EOF
field|@TR<<Authentication>>
onchange|cachange
select|tunnelauthby|$FORM_authby
option|secret|@TR<<Preshared Key>>
option|rsasig|@TR<<X.509 CA>>
onchange|

field|@TR<<Certificate>>|lcert_field|${lcert_flage}
text|tunnelleftcert|$FORM_leftcert
field|@TR<<Private Key>>|lpkey_field|${lpkey_flage}
text|tunnelleftpkey|$FORM_leftpkey

field|@TR<<Phase 1 SA Life Time(s)>>
text|tunnelikelifetime|$FORM_ikelifetime

field|@TR<<Perfect Forward Secrecy>>
checkbox|tunnelpfs|$FORM_pfs|yes

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

cat <<EOF
<tr>
<td colspan="2">
<input id="tunneladvanced" type="checkbox" name="tunneladvanced" value="yes" onclick="adchange(this)" />
Advanced+
</td></tr>
EOF

display_form <<EOF
field|@TR<<Phase 1 DH Group>>|ikedh_field|${ikedh_flage}
select|tunnelikedh|$FORM_ike_dh
option|modp1024|@TR<<modp1024>>
option|modp1536|@TR<<modp1536>>
option|modp2048|@TR<<modp2048>>

field|@TR<<Phase 1 Encryption>>|ikealg_field|${ikealg_flage}
select|tunnelikealg|$FORM_ike_alg
option|3des|@TR<<3des>>
option|aes|@TR<<aes>>
option|aes128|@TR<<aes128>>
option|aes256|@TR<<aes256>>

field|@TR<<Phase 1 Authentication>>|ikeauth_field|${ikeauth_flage}
select|tunnelikeauth|$FORM_ike_auth
option|md5|@TR<<md5>>
option|sha1|@TR<<sha1>>

field|@TR<<Phase 2 DH Group>>|espdh_field|${espdh_flage}
select|tunnelespdh|$FORM_esp_dh
option|modp1024|@TR<<modp1024>>
option|modp1536|@TR<<modp1536>>
option|modp2048|@TR<<modp2048>>

field|@TR<<Phase 2 Encryption>>|espalg_field|${espalg_flage}
select|tunnelespalg|$FORM_esp_alg
option|3des|@TR<<3des>>
option|aes|@TR<<aes>>
option|aes128|@TR<<aes128>>
option|aes256|@TR<<aes256>>

field|@TR<<Phase 2 Authentication>>|espauth_field|${espauth_flage}
select|tunnelespauth|$FORM_esp_auth
option|md5|@TR<<md5>>
option|sha1|@TR<<sha1>>

end_form

EOF

footer_vpn "ipsec-s2s-summary.sh" "update_x2s" "$FORM_enabled br-wan2" ?> 

