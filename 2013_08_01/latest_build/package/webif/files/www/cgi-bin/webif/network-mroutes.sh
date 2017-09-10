#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		droute)
			append droute_cfgs "$cfg_name" "$N"
		;;

		route)
			append sroute_cfgs "$cfg_name" "$N"
		;;

		interface)
			if [ "$cfg_name" != "loopback" ]; then

                                if [ "$cfg_name" = "wan" ];then
                                    cfgname="WAN"
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

#remove static route
if ! empty "$FORM_remove_sroute_vcfg"; then
	uci_remove "network" "$FORM_remove_sroute_vcfg"
fi

if ! empty "$FORM_add_sroute"; then
    un_names=$(cat /var/run/route_sroute_name)
    for chname in $un_names; do
        if [ "$chname" = "$FORM_name_sroute" ]; then
            append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Route Name not unique</h3></td></tr>" "$N"
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
	equal "$?" 0 && {
            [ "$FORM_netmask_sroute" = "" ] && FORM_netmask_sroute="255.255.255.255"
	    [ "$FORM_metric_sroute" = "" ] && FORM_metric_sroute="0"

	    uci_add network route "$FORM_name_sroute"; add_sroute_cfg="$CONFIG_SECTION"
            uci_set network "$add_sroute_cfg" target "$FORM_target_sroute"
	    uci_set network "$add_sroute_cfg" gateway "$FORM_gateway_sroute"
	    uci_set network "$add_sroute_cfg" netmask "$FORM_netmask_sroute"
	    uci_set network "$add_sroute_cfg" metric "$FORM_metric_sroute"
	    uci_set network "$add_sroute_cfg" interface "$FORM_interface_sroute"
	    unset FORM_name_sroute FORM_target_sroute FORM_gateway_sroute FORM_netmask_sroute FORM_metric_sroute FORM_interface_sroute
	#} || {
        #    append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"
        }
fi

if ! empty "$FORM_remove_droute_vcfg"; then
	uci_remove "route" "$FORM_remove_droute_vcfg"
fi

if ! empty "$FORM_add_route"; then
    un_names=$(cat /var/run/route_route_name)
    for chname in $un_names; do
        if [ "$chname" = "network_$FORM_name_route" ]; then
            append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Route Name not unique</h3></td></tr>" "$N"
            break
        fi
    done
validate <<EOF
ipmask|FORM_network_route|@TR<<Network>>|required|$FORM_network_route
EOF
	equal "$?" 0 && {
	    uci_add route droute "network_$FORM_name_route"; add_route_cfg="$CONFIG_SECTION"
            uci_set route "$add_route_cfg" network "$FORM_network_route"
	    unset FORM_name_route FORM_network_route
        }
fi

#init the adding field
uci_load network
uci_load route

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

FORM_name_route="05"
FORM_network_route="192.168.6.0/255.255.255.0"

if [ "$FORM_submit" = "" ]; then
    config_get FORM_select protocol_0 select "enable"
else
    FORM_select="$FORM_select_g"
    uci_set route protocol_0 select "$FORM_select"
fi

form="start_form|Dynamic Route Configuration
        
            field|@TR<<Route Mode>>
            select|select_g|$FORM_select
            option|enable|@TR<<Enable>>
            option|disable|@TR<<Disable>>

            field|@TR<<Name>>|route_name_field|hidden
	    text|name_route|$FORM_name_route

            field|@TR<<Network>>|route_network_field|hidden
	    text|network_route|$FORM_network_route

            string|<br/>
            field||add_route_button_field|hidden
            submit|add_route|@TR<<Add Dynamic Route>>
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


form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<Dynamic Route Summary>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<Dynamic Route Summary>>\">
	string|<th>@TR<<Name>></th>
	string|<th>@TR<<Network>></th>
	string|</tr>"
append forms "$form" "$N"

uniq_names=""
rm -rf /var/run/route_route_name

for route in $droute_cfgs; do
        config_get FORM_network $route network    

        name=$(echo "$route" |cut -c 9-)
        append uniq_names "$route" "$N"
        echo "$uniq_names" > /var/run/route_route_name 
    
        get_tr
        form="$tr  
            string|<td>
            string|$name
            string|</td>
    
            string|<td>
            string|$FORM_network
            string|</td>
    
    
            string|<td>
            string|<a href=\"$SCRIPT_NAME?remove_droute_vcfg=$route\">@TR<<Remove>></a>
            string|</td>
            string|</tr>"
        append forms "$form" "$N"
done
form="  string|</tr>
        string|</table></div>"
append forms "$form" "$N"

javascript_forms="
 
        v = isset('select_g', 'enable');
        set_visible('route_name_field', v);
        set_visible('route_network_field', v);
        set_visible('add_route_button_field', v);
"
append js "$javascript_forms" "$N"

header "Network" "Routes" "@TR<<network_routes_Static_Routes#Routes Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"

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
$added_error
$forms
EOF

footer ?> 
#<!--
###WEBIF:name:Network:500:Routes
#-->
