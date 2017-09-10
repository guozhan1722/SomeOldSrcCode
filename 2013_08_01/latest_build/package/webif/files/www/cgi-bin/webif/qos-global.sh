#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		global)
			append global_cfgs "$cfg_name" "$N"
		;;
		interface)
			if [ "$cfg_name" != "loopback" ]; then
				append networks "option|$cfg_name" "$N"
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

if ! empty "$FORM_add_global"; then
    #[ -z "$FORM_name_rule" ] && append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Rule Name not unique</h3></td></tr>" "$N"

    un_names=$(cat /var/run/qos_global_name)
    for chname in $un_names; do
        if [ "$chname" = "item_$FORM_name_global" ]; then
            append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Global Name not unique</h3></td></tr>" "$N"
            break
        fi
    done
    validate <<EOF
string|FORM_name_global|@TR<<Name>>|required|$FORM_name_global
ipmask|FORM_src_ip_global|@TR<<Source IP Address>>||$FORM_src_ip_global
ipmask|FORM_dest_ip_global|@TR<<Destination IP Address>>||$FORM_dest_ip_global
ports|FORM_dest_port_global|@TR<<Destination Port>>||$FORM_dest_port_global
EOF
	equal "$?" 0 && {
		uci_add qos global "item_$FORM_name_global"; add_global_cfg="$CONFIG_SECTION"
                get_dscp_prio $FORM_target_global
		uci_set qos "$add_global_cfg" target "$FORM_target_global"
		uci_set qos "$add_global_cfg" src_ip "$FORM_src_ip_global"
		uci_set qos "$add_global_cfg" dest_ip "$FORM_dest_ip_global"
		uci_set qos "$add_global_cfg" proto "$FORM_protocol_global"
		uci_set qos "$add_global_cfg" dest_port "$FORM_dest_port_global"
                uci_set qos "$add_global_cfg" dscp "$dscp_cfg"
                uci_set qos "$add_global_cfg" prio "$prio_cfg"

		unset FORM_name_global FORM_dest_ip_global FORM_dest_port_global FORM_src_ip_global FORM_src_port_global FORM_protocol_global FORM_target_global
	#} || {
              #  append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"

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

FORM_name_global=""
FORM_target_global=""
FORM_src_ip_global=""
FORM_dest_ip_global=""
FORM_protocol_global=""
FORM_dest_port_global=""

form="start_form|Qos Global Configuration
            field|@TR<<Global Name>>
	    text|name_global|$FORM_name_global

            field|@TR<<Target>>
            select|target_global|$FORM_target_global
            option|High|@TR<<High>>
            option|Medium_high|@TR<<Medium_high>>
            option|Medium|@TR<<Medium>>
            option|Medium_low|@TR<<Medium_low>>
            option|Low|@TR<<Low>>

            field|@TR<<Source IP>>
	    text|src_ip_global|$FORM_src_ip_global

            field|@TR<<Destination IP>>
            text|dest_ip_global|$FORM_dest_ip_global

            field|@TR<<Protocol>>
            select|protocol_global|$FORM_protocol_global
	    option|tcp|TCP
	    option|udp|UDP
	    option|icmp|ICMP

            field|@TR<<Destination Port>>|dest_port_global_field|hidden
	    text|dest_port_global|$FORM_dest_port_global

            field|
            submit|add_global|@TR<<Add Rule>>
            end_form"
append forms "$form" "$N"

javascript_forms="
       v = isset('protocol_global', 'icmp');
       set_visible('dest_port_global_field', !v);
"
append js "$javascript_forms" "$N"


form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<Qos Global Summary>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<Firewall Rules Summary>>\">
	string|<th>@TR<<Name>></th>
	string|<th>@TR<<Target>></th>
	string|<th>@TR<<Source IP>></th>
	string|<th>@TR<<Destination IP>></th>
	string|<th>@TR<<Protocol>></th>
	string|<th>@TR<<Destination Port>></th>
	string|</tr>"
append forms "$form" "$N"

uci_load qos

vcfgcnt=0
uniq_names=""
rm -rf /var/run/qos_global_name

for rule in $global_cfgs; do

    echo "$rule" |grep -q "item_" && { 
        config_get FORM_target "$rule" target 
        config_get FORM_src_ip "$rule" src_ip
        config_get FORM_dest_ip "$rule" dest_ip
        config_get FORM_protocol "$rule" proto
        config_get FORM_dest_port "$rule" dest_port 

        name=$(echo "$rule" |cut -c 6-)
        append uniq_names "$rule" "$N"
        echo "$uniq_names" > /var/run/qos_global_name 

        get_tr
	form="$tr  
                string|<td>
                string|$name
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
		string|<a href=\"$SCRIPT_NAME?remove_vcfg=$rule\">@TR<<Remove Rule>></a>
		string|</td>
		string|</tr>"
	append forms "$form" "$N"
        let "vcfgcnt+=1"
    }
done

form="  string|</tr>
        string|</table></div>"

append forms "$form" "$N"

header "Qos" "Global" "@TR<<Qos Global Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"

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
string|<tr><td colspan=\"2\"><h3 class=\"warning\">No Qos Global Configuration in Bridge mode </h3></td></tr>
EOF
fi


footer ?>
<!--
###WEBIF:name:Qos:300:Global
-->
