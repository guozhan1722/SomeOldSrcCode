#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

## Network LAN Setting ############################################
uci_load network
config_get wan_bridged "lan" "bridge_wan"


interface="wan"
if [ "$FORM_submit" != "" ] ; then
    local ippassthrough=$(uci get lte.connect.ippassthrough)
    [ "$ippassthrough" = "Ethernet" -o "$ippassthrough" = "WANport" ] && {
         append validate_error "Error in Submit: Can not submit in IP Passthrough Mode" "$N"
    }
fi

if empty "$FORM_submit"; then
    config_get FORM_proto $interface proto
    config_get FORM_ipaddr $interface ipaddr
    config_get FORM_netmask $interface netmask
    config_get FORM_gateway $interface gateway
    config_get FORM_dns $interface dns

    eval FORM_dnsremove="\$FORM_${interface}_dnsremove"
    if [ "$FORM_dnsremove" != "" ]; then
        list_remove FORM_dns "$FORM_dnsremove"
        uci_set "network" "$interface" "dns" "$FORM_dns"
    fi
    [ "$wan_bridged" = "1" ] && FORM_mode="bridge_to_lan" || FORM_mode="independent"
else
    eval FORM_mode="\$FORM_${interface}_mode"
    eval FORM_proto="\$FORM_${interface}_proto"
    eval FORM_ipaddr="\$FORM_${interface}_ipaddr"
    eval FORM_netmask="\$FORM_${interface}_netmask"
    eval FORM_gateway="\$FORM_${interface}_gateway"
    eval FORM_dnsadd="\$FORM_${interface}_dnsadd"

    [ $FORM_mode = "independent" ] && {
        config_get FORM_dns $interface dns
        [ $FORM_dnsadd != "" ] && FORM_dns="$FORM_dns $FORM_dnsadd"
        [ "$FORM_proto" = "static" ] && req="required" || req=""
validate <<EOF
ip|FORM_${interface}_ipaddr|$interface @TR<<IP Address>>|$req|$FORM_ipaddr
netmask|FORM_${interface}_netmask|$interface @TR<<Netmask>>|$req|$FORM_netmask
ip|FORM_${interface}_gateway|$interface @TR<<Default Gateway>>||$FORM_gateway
ip|FORM_${interface}_dnsadd|@TR<<DNS Address>>||$FORM_dnsadd
EOF
        [ "$?" = "0" -a "$validate_error" = "" ] && {
            [ "$wan_bridged" = "1" ] && {
                uci_add "network" "interface" "wan"
                interface="$CONFIG_SECTION"
                uci_set "network" "$interface" "ifname" "eth1"
                uci_set "network" "$interface" "type" "bridge"
                uci_set "network" "lan" "ifname" "eth0"
                uci_set "network" "lan" "bridge_wan" "0"
            }
            uci_set "network" "$interface" "proto" "$FORM_proto"
            if [ "$FORM_proto" = "static" ]; then
                uci_set "network" "$interface" "ipaddr" "$FORM_ipaddr"
                uci_set "network" "$interface" "netmask" "$FORM_netmask"
                uci_set "network" "$interface" "gateway" "$FORM_gateway"
                uci_set "network" "$interface" "dns" "$FORM_dns"
            else
                uci_remove "network" "$interface" "ipaddr"
                uci_remove "network" "$interface" "netmask"
                uci_remove "network" "$interface" "gateway"
                uci_remove "network" "$interface" "dns"
            fi
        }
        append validate_error "$ERROR" "$N"
    } || { # Bridge to LAN
        [ "$validate_error" = "" ] && {
           uci_remove "network" "wan" 
           uci_set "network" "lan" "ifname" "eth0 eth1"
           uci_set "network" "lan" "bridge_wan" "1"
   
           uci_load multiwan
           uci_set "multiwan" "config" "enabled" "0"
        }
        #sleep 1
        #/etc/init.d/multiwan restart
    }

fi

cfgname="WAN"

network_options="start_form|$cfgname @TR<<Configuration>>
	field|@TR<<Working Mode>>
	select|${interface}_mode|$FORM_mode
	option|independent|@TR<<Independent>>
	option|bridge_to_lan|@TR<<Bridge to LAN>>

	field|@TR<<Connection Type>>|field_${interface}_proto|hidden
	select|${interface}_proto|$FORM_proto
	option|dhcp|@TR<<DHCP>>
	option|static|@TR<<Static IP>>

	field|@TR<<IP Address>>|field_${interface}_ipaddr|hidden
	text|${interface}_ipaddr|$FORM_ipaddr

	field|@TR<<Netmask>>|field_${interface}_netmask|hidden
	text|${interface}_netmask|$FORM_netmask

	field|@TR<<Default Gateway>>|field_${interface}_gateway|hidden
	text|${interface}_gateway|$FORM_gateway

	end_form

	start_form|$cfgname @TR<<DNS Servers>>|field_${interface}_dns|hidden
	listedit|${interface}_dns|$SCRIPT_NAME?${interface}_proto=static&amp;|$FORM_dns
	end_form
        "

append forms "$network_options" "$N"

###################################################################
# set JavaScript
javascript_forms="
    if (isset('${interface}_mode','independent')){
        set_visible('field_${interface}_proto', 1);
        v = (isset('${interface}_proto', 'static'));
        set_visible('field_${interface}_ipaddr', v);
        set_visible('field_${interface}_netmask', v);
        set_visible('field_${interface}_gateway', v);
        set_visible('field_${interface}_dns', v);
    } else {
        set_visible('field_${interface}_proto', 0);
        set_visible('field_${interface}_ipaddr', 0);
        set_visible('field_${interface}_netmask', 0);
        set_visible('field_${interface}_gateway', 0);
        set_visible('field_${interface}_dns', 0);        
    }
        "
        
append js "$javascript_forms" "$N"

ERROR=$validate_error

header "Network" "WAN" "@TR<<Network WAN Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"

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
$forms
EOF


footer ?>
<!--
##WEBIF:name:Network:103:WAN
-->
