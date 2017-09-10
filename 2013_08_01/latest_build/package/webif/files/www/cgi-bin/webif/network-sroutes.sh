#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		route)
			append sroute_cfgs "$cfg_name" "$N"
		;;

		interface)
			if [ "$cfg_name" != "loopback" ]; then

                                if [ "$cfg_name" = "wan" ];then
                                    cfgname="WAN"
                                elif [ "$cfg_name" = "wan2" ];then
                                    cfgname="4G"
                                elif [ "$cfg_name" = "lan" ];then
                                    cfgname="LAN"
                                fi
                                
				append networks "option|$cfg_name|$cfgname" "$N"
                                append netmode "$cfg_name" "$N"
			fi
		;;
	esac
}

cur_color="odd"
get_tr() {
	if equal "$cur_color" "odd"; then
		cur_color="even"
		tr="string|<tr>"
	else
		cur_color="odd"
		tr="string|<tr class=\"odd\">"
	fi
}

added_error=""

if [ "$FORM_submit" != "" -o "$FORM_remove_sroute_vcfg" != "" -o "$FORM_add_sroute" != "" ] ; then
    local ippassthrough=$(uci get lte.connect.ippassthrough)
    [ "$ippassthrough" = "Ethernet" ] && {
         append added_error "Error in Submit: Can not submit in IP Passthrough Mode" "$N"
    }
fi

#remove static route
if ! empty "$FORM_remove_sroute_vcfg"; then
	uci_remove "network" "$FORM_remove_sroute_vcfg"
fi

if ! empty "$FORM_add_sroute"; then
    un_names=$(cat /var/run/route_sroute_name)
    for chname in $un_names; do
        if [ "$chname" = "$FORM_name_sroute" ]; then
            append added_error "Error in Name: Route Name not unique" "$N"
            break
        fi
    done
validate <<EOF
string|FORM_name_sroute|@TR<<Name>>|required|$FORM_name_sroute
ip|FORM_target_sroute|@TR<<Destination>>|required|$FORM_target_sroute
ip|FORM_gateway_sroute|@TR<<Gateway>>|required|$FORM_gateway_sroute
netmask|FORM_netmask_sroute|@TR<<Netmask>>||$FORM_netmask_sroute
int|FORM_metric_sroute|@TR<<Metric>>|max=254 min=0|$FORM_metric_sroute
EOF
	[ "$?" = "0" -a "$added_error" = "" ] && {
            [ "$FORM_netmask_sroute" = "" ] && FORM_netmask_sroute="255.255.255.255"
	    [ "$FORM_metric_sroute" = "" ] && FORM_metric_sroute="0"

	    uci_add network route "$FORM_name_sroute"; add_sroute_cfg="$CONFIG_SECTION"
            uci_set network "$add_sroute_cfg" target "$FORM_target_sroute"
	    uci_set network "$add_sroute_cfg" gateway "$FORM_gateway_sroute"
	    uci_set network "$add_sroute_cfg" netmask "$FORM_netmask_sroute"
	    uci_set network "$add_sroute_cfg" metric "$FORM_metric_sroute"
	    uci_set network "$add_sroute_cfg" interface "$FORM_interface_sroute"
	    unset FORM_name_sroute FORM_target_sroute FORM_gateway_sroute FORM_netmask_sroute FORM_metric_sroute FORM_interface_sroute
        }
        append  added_error "$ERROR" "$N"
fi

#init the adding field
uci_load network

FORM_name_sroute="route1"
FORM_target_sroute="192.168.168.0"
FORM_gateway_sroute="192.168.168.1"
FORM_netmask_sroute="255.255.255.0"
FORM_metric_sroute="0"
FORM_interface_sroute="lan"

form="start_form|Static Route Configuration
            field|@TR<<Name>>
	    text|name_sroute|$FORM_name_sroute

            field|@TR<<Destination>>
	    text|target_sroute|$FORM_target_sroute

            field|@TR<<Gateway>>
	    text|gateway_sroute|$FORM_gateway_sroute

            field|@TR<<Netmask>>
	    text|netmask_sroute|$FORM_netmask_sroute

            field|@TR<<Metric>>
	    text|metric_sroute|$FORM_metric_sroute

            field|@TR<<Interface>>
	    select|interface_sroute|$FORM_interface_sroute
	    $networks
	    option||@TR<<None>>

            field|
            submit|add_sroute|@TR<<Add Static Route>>
            end_form"
append forms "$form" "$N"

form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<Static Route Summary>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<Static Route Summary>>\">
	string|<th>@TR<<Name>></th>
	string|<th>@TR<<Destination>></th>
	string|<th>@TR<<Gateway>></th>
	string|<th>@TR<<Netmask>></th>
	string|<th>@TR<<Metric>></th>
	string|<th>@TR<<Interface>></th>
	string|</tr>"
append forms "$form" "$N"


uniq_names=""
rm -rf /var/run/route_sroute_name

for sroute in $sroute_cfgs; do
        config_get FORM_target $sroute target    
        config_get FORM_gateway $sroute gateway    
        config_get FORM_netmask $sroute netmask    
        config_get FORM_metric $sroute metric    
        config_get FORM_interface $sroute interface    
    
        append uniq_names "$sroute" "$N"
        echo "$uniq_names" > /var/run/route_sroute_name 
    
        get_tr
        form="$tr  
            string|<td>
            string|$sroute
            string|</td>
    
            string|<td>
            string|$FORM_target
            string|</td>
    
            string|<td>
            string|$FORM_gateway
            string|</td>
    
            string|<td>
            string|$FORM_netmask
            string|</td>
    
            string|<td>
            string|$FORM_metric
            string|</td>
    
            string|<td>
            string|$FORM_interface
            string|</td>
    
            string|<td>
            string|<a href=\"$SCRIPT_NAME?remove_sroute_vcfg=$sroute\">@TR<<Remove>></a>
            string|</td>
            string|</tr>"

        append forms "$form" "$N"
done
form="  string|</tr>
        string|</table></div>
        string|<br/>"

append forms "$form" "$N"

ERROR=$added_error

header "Network" "Routes" "@TR<<network_routes_Static_Routes#Static Routes Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"

display_form <<EOF
$forms
EOF

footer ?> 
<!--
##WEBIF:name:Network:500:Routes
-->
