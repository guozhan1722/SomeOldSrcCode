#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		classify)
			append classify_cfgs "$cfg_name" "$N"
		;;
		interface)
			if [ "$cfg_name" != "loopback" ]; then
                                if [ "$cfg_name" = "lan" ]; then
                                    cfgname="LAN"
                                elif [ "$cfg_name" = "wan" ]; then
                                    cfgname="WAN"
                                fi
				append networks "option|$cfg_name|$cfgname" "$N"
                                append netmode "$cfg_name" "$N"
			fi
		;;
	esac
}

#target: "High Medium_high Medium Medium_low Low"
#dscp: "0x25 0x24 0x23 0x22 0x21"
#prio: "2 3 4 5 6"
get_dscp_prio() {

    local target_g="$1"
    
    case "$target_g" in
            High)
                    dscp_cfg="0x25"
                    prio_cfg="2"
            ;;
            Medium_high)
                    dscp_cfg="0x24"
                    prio_cfg="3"
            ;;
            Medium)
                    dscp_cfg="0x23"
                    prio_cfg="4"
            ;;
            Medium_low)
                    dscp_cfg="0x22"
                    prio_cfg="5"
            ;;
            Low)
                    dscp_cfg="0x21"
                    prio_cfg="6"
            ;;
    esac
}

#remove rule
if ! empty "$FORM_remove_vcfg"; then
	uci_remove "qos" "$FORM_remove_vcfg"
fi

if ! empty "$FORM_add_classify"; then
validate <<EOF
ipmask|FORM_src_ip_classify|@TR<<Source IP Address>>||$FORM_src_ip_classify
ipmask|FORM_dest_ip_classify|@TR<<Destination IP Address>>||$FORM_dest_ip_classify
ports|FORM_dest_port_classify|@TR<<Destination Port>>||$FORM_dest_port_classify
ports|FORM_ports_classify|@TR<<Ports>>||$FORM_ports_classify
EOF
	equal "$?" 0 && {
		uci_add qos classify ; add_classify_cfg="$CONFIG_SECTION"
                get_dscp_prio $FORM_target_classify
		uci_set qos "$add_classify_cfg" iface "br-$FORM_iface_classify"
		uci_set qos "$add_classify_cfg" target "$FORM_target_classify"
		uci_set qos "$add_classify_cfg" src_ip "$FORM_src_ip_classify"
		uci_set qos "$add_classify_cfg" dest_ip "$FORM_dest_ip_classify"
		uci_set qos "$add_classify_cfg" proto "$FORM_protocol_classify"
		uci_set qos "$add_classify_cfg" dest_port "$FORM_dest_port_classify"
		uci_set qos "$add_classify_cfg" ports "$FORM_ports_classify"
                uci_set qos "$add_classify_cfg" dscp "$dscp_cfg"
                uci_set qos "$add_classify_cfg" prio "$prio_cfg"
		unset FORM_iface_classify FORM_dest_ip_classify FORM_dest_port_classify FORM_src_ip_classify FORM_src_port_classify FORM_protocol_classify FORM_target_classify FORM_ports_classify
	#} || {
        #        append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"
        }
fi

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

#init the adding field
uci_load network

FORM_iface_classify=""
FORM_target_classify=""
FORM_src_ip_classify=""
FORM_dest_ip_classify=""
FORM_protocol_classify=""
FORM_dest_port_classify=""
FORM_ports_classify=""

form="start_form|Qos Local Configuration

            field|@TR<<Interface>>
            select|iface_classify|$FORM_iface_classify
            $networks

            field|@TR<<Target>>
            select|target_classify|$FORM_target_classify
            option|High|@TR<<High>>
            option|Medium_high|@TR<<Medium_high>>
            option|Medium|@TR<<Medium>>
            option|Medium_low|@TR<<Medium_low>>
            option|Low|@TR<<Low>>

            field|@TR<<Source IP>>
	    text|src_ip_classify|$FORM_src_ip_classify


            field|@TR<<Destination IP>>
            text|dest_ip_classify|$FORM_dest_ip_classify

            field|@TR<<Protocol>>
            select|protocol_classify|$FORM_protocol_classify
	    option|tcp|TCP
	    option|udp|UDP
	    option|icmp|ICMP

            field|@TR<<Destination Port>>|dest_port_field|hidden
	    text|dest_port_classify|$FORM_dest_port_classify

            field|@TR<<Ports>>|ports_field|hidden
	    text|ports_classify|$FORM_ports_classify

            field|
            submit|add_classify|@TR<<Add Local rule>>
            end_form"
append forms "$form" "$N"

javascript_forms="
       v = isset('protocol_classify', 'icmp');
       set_visible('dest_port_field', !v);
       set_visible('ports_field', !v);
"
append js "$javascript_forms" "$N"

form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<Qos Local Summary>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<Qos Local Summary>>\">
	string|<th>@TR<<Interface>></th>
	string|<th>@TR<<Target>></th>
	string|<th>@TR<<Source IP>></th>
	string|<th>@TR<<Destination IP>></th>
	string|<th>@TR<<Protocol>></th>
	string|<th>@TR<<Destination Port>></th>
	string|<th>@TR<<Ports>></th>
	string|</tr>"
append forms "$form" "$N"

uci_load qos

vcfgcnt=0

for rule in $classify_cfgs; do

        config_get FORM_iface "$rule" iface 
        config_get FORM_target "$rule" target 
        config_get FORM_src_ip "$rule" src_ip
        config_get FORM_dest_ip "$rule" dest_ip
        config_get FORM_protocol "$rule" proto
        config_get FORM_dest_port "$rule" dest_port 
        config_get FORM_ports "$rule" ports 
        
        FORM_iface=$(echo $FORM_iface|cut -c 4-)

        if [ "$FORM_iface" = "lan" ]; then
            FORM_iface="LAN"
        elif [ "$FORM_iface" = "wan" ]; then
            FORM_iface="WAN"
        fi

        get_tr
	form="$tr  
                string|<td>
                string|$FORM_iface
                string|</td>

        	string|<td>
        	string|$FORM_target
        	string|</td>


		string|<td>
		string|$FORM_src_ip
		string|</td>

		string|<td>
		string|$FORM_dest_ip
		string|</td>

		string|<td>
		string|$FORM_protocol
		string|</td>

		string|<td>
		string|$FORM_dest_port
		string|</td>

		string|<td>
		string|$FORM_ports
		string|</td>

		string|<td>
		string|<a href=\"$SCRIPT_NAME?remove_vcfg=$rule\">@TR<<Remove Local rule>></a>
		string|</td>
		string|</tr>"
	append forms "$form" "$N"
        let "vcfgcnt+=1"
done

form="  string|</tr>
        string|</table></div>"

append forms "$form" "$N"

header "Qos" "Local" "@TR<<Qos Local Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"

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

sysmode=$(uci get system.@system[0].systemmode)

if [ "$sysmode" = "router" -o "$sysmode" = "gateway" ] ; then
display_form <<EOF
onchange|modechange
$added_error
$forms
EOF
else
display_form <<EOF
string|<tr><td colspan=\"2\"><h3 class=\"warning\">No Qos Local Configuration in Bridge mode </h3></td></tr>
EOF
fi


footer ?>
<!--
##WEBIF:name:Qos:400:Local
-->
